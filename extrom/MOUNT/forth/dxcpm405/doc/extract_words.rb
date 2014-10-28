require 'ap'
 # -EXT  ( c-addr u1 -- c-addr u2 )                      A

 #     Delete any extension from the filename string c-addr u1.  The
 #     resulting string is c-addr u2.  Formerly named -FILENAME .

 # -PATH  ( c-addr u1 -- c-addr u2 )                     A


def parse_dxforth_glo(fname,h_params)
     h_asm,h_comment=h_params

     h=["ASM","I","A","S","94","83","79","FIG","C"]

     used_h=[]

     section=""
     File.open(fname).each_line do |l|
          l.chomp!
          section=l if l =~ /^\d+\. /
          next unless l=~/^ (\S+)\s+(\(.*?\))\s+(\(.*?\))?\s+(.*)$/
          # p l
          # p [$1,$2,$3,$4]
          name=$1
          f=$4.split

          flags=["","","","","","","","","",]
          f.map{|v| 
               idx=h.index(v);
               raise "ERR: #{v}" if idx.nil?
               flags[idx]=v
          }

          if h_asm[name]
               flags[0]=h_asm[name]
               used_h << name
          end

          puts [section,$1,$2,$3,*flags].join(";")

     end

     not_in_list=h_asm.keys-used_h

     section=""
     not_in_list.each do |name|
          outstr=[section,name,h_comment[name],"",h_asm[name]].map{|v| v ? v.sub(';','";"') : v}.join(";")
          puts outstr
     end

end

def extract_asm(path)
     flist=Dir["#{path}/*.Z80"]
     flist+=Dir["#{path}/*.MAC"]
     h_asm={}
     h_comment={}
     flist.each do |fname|
          File.open(fname).each_line do |l|
               l.chomp!
               if l=~/^\s+hdr\s+(.*?),\'(.*?)\'/
                    # p [l,$1,$2]
                    h_asm[$2]=File.basename(fname).sub(".Z80",'').sub(".MAC",'')
               end
               if l=~/^;\s+(\S+)\s+(\(.*?\))/
                    h_comment[$1.upcase]=$2
               end
          end
     end
     [h_asm,h_comment]
end

h_asm=extract_asm("..")
parse_dxforth_glo("DXFORTH.GLO",h_asm)