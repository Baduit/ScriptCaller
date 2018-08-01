  def hello()
	return "Hello world"
end

	def add(a, b)
	return a + b
end

def concat(s1, s2)
	return s1 + s2
end

def makeArray(a, b, c)
	return [a, b, c]
end

class Fred
    def initialize(p1, p2)
      @a, @b = p1, p2
	end
	
	def hi()
		return "Hi"
	end

	def a()
		return @a
	end

	def self.staticHi()
		return "static hi"
	end
end

fred = Fred.new('cat', 99)

my_str = "Hello World!"