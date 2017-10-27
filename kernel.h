#pragma once

#include "debug.h"
#include "infodatabase.h"

typedef struct
{
	struct
	{
		GUID guid;
	}os;

	struct
	{
		SECURITY_ATTRIBUTES saAllowAll;
		SECURITY_DESCRIPTOR sdAllowAll;
		void *buf;
	}securityAttributes;

	//Chemin d'accès statiques
	struct
	{
		WCHAR home[MAX_PATH];
		WCHAR process[MAX_PATH];
	}paths;

	//Contient les évènements globaux
	struct
	{
		HANDLE stopEvent;
	}globalEvents;

	struct
	{
		HCRYPTKEY bzpValidatePublicKey;
		HCRYPTKEY bzpDataPublicKey;
	}keys;

	InfoDatabase::info_database_t database;
}kernel_data_t;
extern kernel_data_t kernelData;

namespace Kernel
{
	bool init(void);
	void uninit(void);
	HANDLE waitForMutexOfObject(DWORD id);
	bool isAlive();
};