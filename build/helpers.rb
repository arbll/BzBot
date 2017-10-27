def render path, bindings
    content = File.read(File.expand_path(path + '.eruby'))
    t = Erubis::Eruby.new(content)
	t.result(bindings)
end

class String
  def ^ (byte)
    self.bytes.map { |(c)| c ^ byte }.pack('c*')
  end
end