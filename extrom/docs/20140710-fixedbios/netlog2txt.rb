require "ap"

def netlog2txt(file)
	body=File.read(file,"rb").unpack("c*")
end

ap netlog2txt("rmp_net.log")