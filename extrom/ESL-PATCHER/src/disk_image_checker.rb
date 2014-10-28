require 'ap'
$stdout.sync = true

$trase_check=true
$trase_check=false

class Checker
	def initialize(stage2_file)#,offset)
		# @offset=detect_offset()
		body=File.open(stage2_file,"r:ascii").read#.unpack("C*")
		# ap body.map{|v|v.chr}.join.size
		# ap body.size
		@patchers=get_patcher_table(body)
		# @detector_data=File.open(stage2_file,"r:ascii").read.unpack("C*")[offset..-1]
		# ap @body.size
	end
	
	def check_file(file_name)

		p [:check_file,file_name] if $trase_check
		result="nil"
		maybe_count,maybe_match=0,0

		crc_ok,load_addr,size,ram=get_file(file_name)
		# ap [crc_ok,"%04x" % load_addr,size]
		if crc_ok
			result,maybe_count,maybe_match=detector(ram)
			if result==false && load_addr == 0
				result="BAD_#{"%04x" % load_addr}_#{size}"
			elsif result==false && load_addr == 0xee && (size == 0x0e*1024) 
				result = "CPM_NET_KORNET_drive_b"
			# else
			end
			# case size
			# when 10240,13312,14336
				# result=detector(ram)
			# when 13312			
			# 	result=":MICRODOS_1_13312"
			# when 14336			
				# result=":MICRODOS_2_14336"
			# else 
				# result=size.to_s
			# end
		else
			result="!!! CRC failed"
		end
		[result,maybe_count,maybe_match]
	end

	private

	def get_patcher_table(body)
		offset_tab=body.index("PATCHER OFFSET TABLE>>")+"PATCHER OFFSET TABLE>>".size
		data_start=body.index("PATCHER DATA>>")+"PATCHER DATA>>".size
		data_end=body.index("<<PATCHER DATA")-1
		# ap [offset_tab,data_start,data_end].map{|v| "%04x" % v}

		offsets=body[offset_tab..-1].unpack("S*")
		terminator=offsets.index(0)-1
		offsets=offsets[0..terminator]
		offsets=offsets.map{|v| v-0x2000}

		offsets << data_end #terminate last dataset

		# ap body.size
		patchers=[]
		0.upto(offsets.size-2) do |idx|
			# ap idx
			size=offsets[idx+1]-offsets[idx]
			# ap [idx,"%04x" % offsets[idx],size]
			patchers<<body.unpack("@#{offsets[idx]}C#{size}")
			# p patchers[-1].map{|v| "%02x" % v}#.join
			raise "no tailing zero in #{idx}" unless patchers[-1][-1]==0 
		end
		patchers
	end

	def detector(ram)
		ptr=0
		# ap [ram.map{|v| "%02x" % v}]
		name=" -- undefied -- "
		

		r=[]
		@patchers.each do |patch|
			result,name,maybe_count,maybe_match=check_patch(patch,ram)
			r << [name,maybe_count,maybe_match] if result
		end
		if r.size>1
			if r[0][0] == 'CPM_12_88_3_alternativa' and r[1][0] == 'CPM_12_88_3_niijaf'
				r.delete_at(1)
			else 
				ap r
				raise "More than one bios matched !!!!!!!!!"
			end
		end

		r=["!!!undefined",0,0] if r.empty?
		r.flatten
	end

	def check_patch(patch,ram)
		stream=patch.each
		
		fast_exit=true
		run=true
		found=true
		name=""
		maybe_count=0
		maybe_match=0

		while run
			byte=stream.next
			case byte
			when 0x50 #stop  
				run=false
				while (byte=stream.next) != 0
					name+=byte.chr
				end
				p ["stop",found,name] if $trase_check
			when 0x51 #pW_CHK_PATCH
				addr=stream.next+(stream.next<<8)
				vold=stream.next+(stream.next<<8)
				vnew=stream.next+(stream.next<<8)
				p [0x51,addr,vold,vnew].map{|v| "%04x" % v} if $trase_check

				in_ram=ram[addr]+(ram[addr+1]<<8)
				found=false unless in_ram == vold
				p [:r51,[in_ram,vold].map{|v| "%04x" % v},found] if $trase_check
			when 0x52 #pB_CHK_PATCH
				addr=stream.next+(stream.next<<8)
				vold=stream.next#+(stream.next<<8)
				vnew=stream.next#+(stream.next<<8)
				p [0x52,addr,vold,vnew].map{|v| "%04x" % v} if $trase_check

				in_ram=ram[addr]#+(ram[addr+1]<<8)
				found=false unless in_ram == vold
				p [:r52,[in_ram,vold].map{|v| "%04x" % v},found] if $trase_check
			when 0x54 #pW_STORE
				addr=stream.next+(stream.next<<8)
				value=stream.next+(stream.next<<8)
				p [0x54,addr,value].map{|v| "%04x" % v} if $trase_check
			when 0x55 #pNotSupported
				p [0x55].map{|v| "%04x" % v} if $trase_check
			when 0x56 #pSETFLAG_MICRODOS
				p [0x56].map{|v| "%04x" % v} if $trase_check
			when 0x57 #pREQUIRED_OPTS1
				p [0x57].map{|v| "%04x" % v} if $trase_check
			when 0x58 #pREQUIRED_OPTS2
				p [0x58].map{|v| "%04x" % v} if $trase_check
			when 0x59 #pB_MAYBE_PATCH
				addr=stream.next+(stream.next<<8)
				vold=stream.next#+(stream.next<<8)
				vnew=stream.next#+(stream.next<<8)
				p [0x59,addr,vold,vnew].map{|v| "%04x" % v} if $trase_check

				in_ram=ram[addr]#+(ram[addr+1]<<8)
				#found=false unless in_ram == vold
				maybe_count+=1
				maybe_match+=1 if in_ram == vold

			when 0x5A #pSubBios
				type=stream.next
				p [0x5A,type].map{|v| "%04x" % v} if $trase_check
			else 
				raise "Unsupported patchcode #{"%02x % byte"}"
			end

			run=false if fast_exit && found==false
		end
		[found,name,maybe_count,maybe_match]
	end

	def detector2(ram)
		ptr=0
		# ap [ram.map{|v| "%02x" % v}]
		name=" -- undefied -- "
		run=true
		work=@detector_data.dup.clone
		found_flag=true
		while run do
			b=work[ptr];ptr+=1
			# p ["%04x" % (ptr-1),"%02x" % b]
			case b
			when 0xff # stop
				run=false
			when 0 # stop
				tmp=""
				while work[ptr] != 0 do
					tmp+=work[ptr].chr
					ptr+=1
				end
				ptr+=1

				if found_flag
					run=false
					name=tmp 
				else
					found_flag=true
					name=" -- undefied -- "
				end

			when 1 # word chk patch
				#addr,old,new
				addr=work[ptr]+(work[ptr+1]<<8);ptr+=2
				vold=work[ptr]+(work[ptr+1]<<8);ptr+=2
				vnew=work[ptr]+(work[ptr+1]<<8);ptr+=2

				in_ram=ram[addr]+(ram[addr+1]<<8)
				found_flag=false unless in_ram == vold
				# p [:word,addr,"%04x" % addr,"%04x" % vold,"%04x" % in_ram]

			when 2 # byte chk patch
				#addr,old,new
				addr=work[ptr]+(work[ptr+1]<<8);ptr+=2
				vold=work[ptr];ptr+=1
				vnew=work[ptr];ptr+=1

				in_ram=ram[addr]
				found_flag=false unless in_ram == vold
				# p [:word,addr,"%04x" % addr,"%02x" % vold,"%02x" % in_ram]
			when 4 # word store
				#addr,value
				addr=work[ptr]+(work[ptr+1]<<8);ptr+=2
				vnew=work[ptr]+(work[ptr+1]<<8);ptr+=2
				# p [:store,"%04x" % addr,"%04x" % vnew]
			when 5 # Unsupported
			else 
				raise "Unsupported CODE #{"%04x" % ptr} : #{"%02x" % b}"
			end
		end
		name
	end


	def get_file(file_name)
		body=File.open(file_name,"r:ascii").read

		loadadress ,
		runadress  ,
		count      ,
		sizedisk   ,
		density    ,
		tpi        ,
		skewfactor ,
		secsize    ,
		inside     ,
		secpertrack,
		trkperdisk ,

		spt        ,		# No. of Logical ( 128-byte ) Sectors per Logical Track
		bsh        ,		# Block Shift - Block Size is given by 128 * 2^(BSH)
		blm        ,		# Block Mask - Block Size is given by 128 * (BLM +1)
		exm        ,		# Extent Mask ( see opposite )
		dsm        ,		# объем памяти на диске в блоках минус 1
		drm        ,		# число входов в директорию диска минус 1
		al0        ,		# определяет, какие блоки зарезервированы
		al1        ,		# под директорию
		cks        ,		# размер вектора контроля директории
		ofs        ,		# число системных дорожек на диске
		checksum 			= body[0..31].unpack("SSSCCCCCCSS"+"SCCCSSCCSSC")

		file_body=body.unpack("C*")

		calculated_crc=file_body[0..31-1].reduce(0x66){|sum,n| (sum+n.ord)&0xff}

		ram=(0..loadadress-1).to_a.map{|v| 0x55} 		#ram before bios
		ram+=file_body[0..(count*1024)]
		ram+=(0..(65535-ram.size-1)).to_a.map{|v| 0xAA} #ram after bios

		[calculated_crc == checksum,loadadress,count*1024,ram]
	end		

end
z=Checker.new("stage2.rom")

flist=Dir["/home/esl/Dropbox/Emulator/Korvet/KorvetSoft/**/*"]
# flist=Dir["/home/esl//Dropbox/Emulator/Korvet/korvet-pk8020-emulator/extrom/DISK/*"]
kdi=flist.select{|v| v=~/\.(kdi|img)$/i}.select{|v| !File.directory?(v)}.sort

# kdi=Dir["../../*.kdi"]
# p z.check_file("../../DISK/12_87_11_niijaf.kdi")
# __END__
h_oscount={}
h_os={}
kdi.each do |f|
	# print "#{f} - "
	type,maybe_count,maybe_match=z.check_file(f)
	# ap [type,maybe_count,maybe_match,f]
	name="%-30s " % type
	tail="[%d:%d:%d] - %s" % [maybe_count,maybe_match,maybe_count-maybe_match,f]
	puts "#{name}#{tail}"

	h_oscount[type]||=0
	h_oscount[type]+=1

	h_os[type]||=[]
	h_os[type]<<=tail

end

puts "-"*64
keys=h_oscount.keys.sort{|a,b| h_oscount[b] <=> h_oscount[a]}
keys.each do |k|
	puts "%-30s - %3d" % [k,h_oscount[k]]
end

puts "-"*64
spc=" "*(30+3+4)
keys.each do |k|
	puts "%-30s - %d\n#{spc}%s" % [k,h_os[k].size,h_os[k].join("\n#{spc}")]
end