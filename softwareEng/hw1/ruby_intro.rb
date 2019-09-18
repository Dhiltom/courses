# When done, submit this entire file to the autograder.

# Part 1

def sum arr
  if arr.length == 0
  	return 0
  end
  s = 0
  for i in 0...arr.length do
        s += arr[i]
  end
  return s
end

def max_2_sum arr
  if arr.length == 0
        return 0
  end
  if arr.length == 1
        return arr[0]
  end
  
  for i in 0...2 do
	for j in (i+1)...arr.length do
        	if (arr[i]<arr[j])
                        t = arr[i] 
                	arr[i] = arr[j]
                        arr[j] = t
                end
        end
   end

   return (arr[0]+arr[1])
			 
end

def sum_to_n? arr, n
  if (arr.length == 0) or (arr.length == 1)
  	return false
  end
  
  d = Hash.new
  arr.each_with_index do |a,ind|
	x = d[n - a]
	if x != nil
		return true
 	end
 	d[a] = ind
  end
  return false
end

# Part 2

def hello(name)
 return  ("Hello, ").concat(name)
end

def starts_with_consonant? s
  if /^[^aeiou\W]/i.match(s) == nil
    return false
  else
    return true
  end 
end

def binary_multiple_of_4? s
  if s == "0"
	return true
  end 
  /^[10]*00$/.match(s) != nil 
end

# Part 3

class BookInStock

	attr_accessor :isbn
	attr_accessor :price

	def initialize(isbn, price)
		raise ArgumentError, 
			"Wrong argument entered" if isbn.empty?  or price <= 0
		@isbn = isbn
		@price = price
	end

	def price_as_string
		sprintf("$%.2f", @price)
	end
end
