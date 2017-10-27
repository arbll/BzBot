#pragma once

namespace Sync
{
	void getObjectName(DWORD id, LPWSTR buffer);

	HANDLE _createUniqueMutex(SECURITY_ATTRIBUTES *mutexAttributes, LPWSTR name);
	HANDLE _waitForMutex(SECURITY_ATTRIBUTES *mutexAttributes, LPWSTR name);
	void _freeMutex(HANDLE mutex);
};