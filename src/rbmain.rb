puts "Hello, mruby on stm32f4 discovery"
def fib(n); n <= 1 ? n : fib(n-2) + fib(n-1); end  
25.times do |i|
  puts "fib(#{i})=#{fib(i)}"
end
