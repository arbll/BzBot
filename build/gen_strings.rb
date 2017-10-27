def gen_strings
	cryptedstrings_txt = File.read('cryptedstrings.txt')

	cryptedstrings = {}

	cryptedstrings_txt.split("\n").each do |line|
		if (line != "" && !line.empty? && !line.start_with?( '//', '#'))
			line_contents = line.gsub(/\t/, ' ').gsub(/\s+/m, ' ').split(" ", -1)
			if (line_contents.length >= 2)
				key = rand(0..255)
				cryptedstrings[line_contents[0]] = [((line_contents.drop(1).join(" ").gsub("\\n", "\n")) ^ key).unpack('C*').map {|e| ('\x' + e.to_s(16))}.join, key, (line_contents.drop(1).join(" ").gsub("\\n", "\n")).length]
			end
		end
	end

	File.write('cryptedstrings.h', render("cryptedstrings.h", binding))
	File.write('cryptedstrings.cpp', render("cryptedstrings.cpp", binding))
end