function calculate_pi() 
	--local no_of_parts = 200000000
	local no_of_parts =   200000000
	local interval = 1.0/no_of_parts
	local result = 0.0
	local x = 0.0
	for i = 0 , no_of_parts do
		local y = 4.0/(1.0+math.pow(x,2))
		result = result+interval*y
		x = x+ interval
		--print("Arijit"..i)
	end
	print("Value of the pi      : "..result)
 	print("Original Value of pi : "..math.pi)
end

calculate_pi();
