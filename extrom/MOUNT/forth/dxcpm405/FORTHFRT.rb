require "ap"


def split(from)
	a=File.open(from,"r:koi8-r:utf-8").read
	r=a.split(//)

	rr=r.each_slice(64).to_a.map{|v| v.join}.each_slice(16).to_a#.slice(16)

	# ap rr

	File.open("txt/#{from}.txt","w") {|f|
		rr.each_with_index {|v,idx|
			f.puts "Block: #{idx}"
			f.puts v.join("\n")
			f.puts
		}
	}
end

Dir["*.SCR"].each do |f| split(f) end