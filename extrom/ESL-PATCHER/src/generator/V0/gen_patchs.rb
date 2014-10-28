# encoding: UTF-8
# require 'ap'
require 'fileutils'


class PATHCH_GEN
	def initialize(h_values)
		@h_values=h_values.dup
		

		type=@h_values[:INFO_TEMPLATE]

		@checkers=@h_values.delete(:CUSTOM_CHECKERS)
		@h_values=remove_empty_keys(@h_values)


		@checkers||=[]
		@h_values[:CUSTOM_CHECKERS]=create_checkers()
		# ap @checkers

		fname="./data/extrom-patcher-template-#{type}.asm"

		raise "empty type" if type==nil or type.empty? 

		template_body=File.read(fname)

		remove_comments=true
		remove_comments=false if @h_values[:NAME] == 'CPM_21_89___wiza'
		remove_comments=false if @h_values[:NAME] == 'MICRODOS_OPTS2_900105'

		@array=filter_comments(template_body,remove_comments) 
	end

	def remove_empty_keys(h)
		h.delete_if{ |k,v| (h[k].nil? or h[k].empty?)} 
		h.delete_if{ |k,v| k =~ /^INFO_/ }
		h
	end

	def apply
		check
		out=@array.map{ |srcline|
			line=srcline.dup
			# @h_values.each_pair{|k,v| line.gsub!(":#{k.to_s}:",">>>#{k} -> #{v}<<<")}
			@h_values.each_pair{|k,v| line.gsub!(":#{k.to_s}:",v)}
			line
		}
		out
	end

	def create_checkers
		out=[]
		@checkers.each do |chkrec|
			addr,value=chkrec
			type=nil
			case value
			when /^\s*0x[0-9A-Fa-f]{2}\s*$/i
				type="db"
			when /^\s*0x[0-9A-Fa-f]{4}\s*$/i
				type="dw"
			else
				raise "can't detect db/dw for value #{value}"
			end
			out << "\t#{type}patch\t#{addr}\t,#{value} ,#{value}"
		end
		out=["\t;custom checkers",*out] if out.size > 0
		out.join("\n")
	end

	def check
		template_tokens=template_tokens().sort
		values_tokens=@h_values.keys.map{|v| ":#{v.to_s}:"}.sort
		# ap [template_tokens,values_tokens]

		common=values_tokens & template_tokens
		undefined_template=template_tokens-common
		undefined_values=values_tokens-common
		
		raise "params use tokens not declared in template [#{undefined_values.join(",")}] " if undefined_values.size>0
		raise "template use following tokens not declared in params [#{undefined_template.join(",")}] " if undefined_template.size>0

		# ap [[:common,common],[:undefined_template,undefined_template],[:undefined_values,undefined_values]]
		nil
	end

	def filter_comments(txt,remove_comments)
		array=txt.split("\n")
		# array=array.each_with_index.map{|x,i| x+"|#{i}"}
		if remove_comments
			array=array.reject{|v| v=~/^\s*?\;.*/}
			array=array.reject{|v| v=~/^\s*?$/}
		end
		array
	end

	def template_tokens
		tokens_in_source=@array.map{|v| v.scan(/(\:[^\: \t]+\:)/).flatten}.flatten.sort.uniq
		tokens_in_source
	end
end

class CSVREADER
	def initialize(fname)
		@fname=fname
	end

	def read
		out=[]
		rows=File.readlines(@fname)
				 .map{|line| v=line;v.gsub!("'",'');v} 	#remove ''
				 .map{|line| line.split(",")} 			#split by comas
				 .reject{|row| row[0]==""} 				#remove rows with empty first collumn
				 .reject{|row| row[0]=~/\s*#/}			#remove rows with collumn started from #
				 .map{|row| row[-1].sub!(/\n/,'');row}	#remove trailing CR

		tab=rows.transpose 								

		#extract header and data
		hdr 	=tab[0].map{|v| v.to_sym}
		biosdata=tab[1..-1]

		biosdata.each do |data|
			h={}
			hdr.each_with_index{ |x,i| h[x]=data[i] } 	#create hash, key-column name, value-value from corresponding collumn
			out << fix_checker(h) 						#convert CUSTOM_CHECKERS to special structure
		end
		out
	end

	#convert CUSTOM_CHECKERS\d\d(A|V) pairs into array [ [ADDR,VALUE], ... ]
	def fix_checker(h_in)
		checkers=[]
		keys=h_in.keys.sort.select{|v| v=~/CUSTOM_CHECKERS/}
		keys.each do |name|
			if name =~ /CUSTOM_CHECKERS(\d\d)(A|V)/
				id=$1.to_i-1
				type=$2 
				offset= type == "A" ? 0 : 1 
				value=h_in[name]
				h_in.delete(name)
				unless value.empty?
					checkers[id]||=[]
					checkers[id][offset]=value
				end
			else
				raise "NAME: #{name}"
			end
		end
		
		h_in[:CUSTOM_CHECKERS]=checkers if checkers.size>0
		
		h_in
	end
end


patcher_blocks=[]

patch_groups=["cpm_chk","cpm1","cpm2","microdos","unsupported"]

#read all patcher data into one list
patch_groups.each do |path_group|
	patchdata_fname="./data/patcher-data-#{path_group}.csv"
	variants=CSVREADER.new(patchdata_fname).read

	patcher_blocks += variants
end

#order by KDI image count,
patcher_blocks=patcher_blocks.sort{ |a,b| 
	cmp_value=b[:INFO_IMAGES_COUNT].to_i <=> a[:INFO_IMAGES_COUNT].to_i
	cmp_value=a[:INFO_TEMPLATE] <=> b[:INFO_TEMPLATE] if cmp_value==0
	cmp_value=b[:NAME] <=> a[:NAME] if cmp_value==0
	cmp_value
}

# ap [:after,patcher_blocks]
patcher_blocks=[patcher_blocks]

names=[]

#generate asm files
FileUtils.mkdir_p("out")

patcher_blocks.each do |variants|
	variants.each do |v|
		name=v[:NAME]
		type=v[:INFO_TEMPLATE]
		p [type,name]
		fname="out/patcher-#{type}-#{name}.asm"
		names << v
		File.open(fname,"w"){|f| f.puts PATHCH_GEN.new(v).apply }
	end
end

#create include-pathcer.asm (ordered by kdi count)
File.open("out/include-patcher.asm","w"){|f|
	f.puts "bios_variants:"
	# f.puts names.map{|name| "\tdw\t_CPM_BIOS_#{name}"}
	f.puts names.map{|v| "\tdw\t_BIOS_#{v[:NAME]}"}
	f.puts "\tdw\t0"
	f.puts
	f.puts "\tdb 	\"PATCHER DATA>>\""
	f.puts
	names.each do |v|
		name=v[:NAME]
		type=v[:INFO_TEMPLATE]
		count=v[:INFO_IMAGES_COUNT].to_i
		if count/1000 > 0
			count-=(count/1000)*1000
		end
		count = count>0 ? "\t;#{"%3d" % count} images in collection" : ""

		f.puts "\tinclude \"generator/V0/out/patcher-#{type}-#{name}.asm\"#{count}"
	end
	f.puts "\tdb 0xff"
}
