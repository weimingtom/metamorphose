-- Compare benchmark results from multiple VMs.
--
local results = {}
local scripts = {}
local totals = {}
local vm_names = {}
local max_script_len = 0

local function read_logfile(vm_name, file)
	local total = 0.0
	for line in io.lines(file) do
		local idx=line:find(":")
		if idx ~= nil then
			local script = line:sub(1,idx-1)
			local val = tonumber(line:sub(idx+1))
			if results[script] == nil then
				table.insert(scripts, script)
				results[script] = {}
				if max_script_len < script:len() then
					max_script_len = script:len()
				end
			end
			results[script][vm_name] = val
			total = total + val
		end
	end
	totals[vm_name] = total
end

for i,file in ipairs(arg) do
	local idx = file:find(".", -4)
	local vm_name = file:sub(1,idx-1)
	table.insert(vm_names, vm_name)
	read_logfile(vm_name,file)
end

table.sort(scripts)
local pad = (max_script_len - 6) + 2
io.write("script")
io.write((" "):rep(pad))
for i,vm_name in ipairs(vm_names) do
	pad = (7 - math.min(vm_name:len(), 7)) + 2
	io.write(vm_name)
	if pad > 0 then
		io.write((" "):rep(pad))
	end
end
io.write("\n")

for i,script in ipairs(scripts) do
	local values = results[script]
	pad = (max_script_len - script:len()) + 2
	io.write(script)
	io.write((" "):rep(pad))
	for i,vm_name in ipairs(vm_names) do
		pad = math.max(vm_name:len(), 7) - 4
		io.write(string.format("%0.2f",values[vm_name]))
		if pad < 0 then
			pad = pad * -1
		end
		pad = pad + 2
		io.write((" "):rep(pad))
	end
	io.write("\n")
end

pad = (max_script_len - 5) + 2
local total
io.write("Total")
io.write((" "):rep(pad))
for i,vm_name in ipairs(vm_names) do
	total = string.format("%3.2f",totals[vm_name])
	pad = math.max(vm_name:len(), 7) - total:len()
	io.write(total)
	if pad < 0 then
		pad = pad * -1
	end
	pad = pad + 2
	io.write((" "):rep(pad))
end
io.write("\n")


