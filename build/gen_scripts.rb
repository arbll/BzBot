def gen_scripts
	salt = '},z*F?-Y!*^Z<39n-iMP&S53!iDY*vHBiD_ah,Qda@Kw-3|GyD?<]/(t1Jd!u }>'

	aes_key = SecureRandom.random_bytes(16)

	aes_key_cpp = aes_key.unpack('C*').map {|e| ('0x' + e.to_s(16))}.join(', ')

	cryptedscripts = {}

	cipher = OpenSSL::Cipher::AES128.new(:CBC)
	cipher.encrypt                                                                                                                                                                                              
	cipher.key = aes_key
	iv = cipher.random_iv
	cipher.iv = iv
	aes_iv_cpp = iv.unpack('C*').map {|e| ('0x' + e.to_s(16))}.join(', ')

	Digest::SHA1.hexdigest 'foo'
	Dir["./payloads/*.lua"].each do |file|
		cipher = OpenSSL::Cipher::AES128.new(:CBC)
		cipher.encrypt                                                                                                                                                                                              
		cipher.key = aes_key
		cipher.iv = iv
		lua_script = (File.read(file).unpack('C*') << 0).pack('C*')
		file =  File.basename(file, ".*")                                                                                                                                                                                                                                                                                                                                                              
		cipher_text = cipher.update(lua_script) + cipher.final
		cryptedscripts[file] = [Digest::SHA1.hexdigest(Digest::SHA1.hexdigest(file + salt) + salt), cipher_text.unpack('C*').map {|e| ('\x' + e.to_s(16))}.join, cipher_text.length]
	end

	File.write('cryptedscripts.h', render("cryptedscripts.h", binding))
	File.write('cryptedscripts.cpp', render("cryptedscripts.cpp", binding))
end