#include "stdafx.h"
#include "sync.h"
#include <string>

void Sync::getObjectName(DWORD id, LPWSTR buffer)
{
	_itow_s(id, buffer, 50, 16);
}

HANDLE Sync::_createUniqueMutex(SECURITY_ATTRIBUTES * mutexAttributes, LPWSTR name)
{
	HANDLE mutexHandle = CreateMutexW(mutexAttributes, TRUE, name);
	if (mutexHandle != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(mutexHandle);
		mutexHandle = NULL;
	}

	return mutexHandle;
} 

HANDLE Sync::_waitForMutex(SECURITY_ATTRIBUTES *mutexAttributes, LPWSTR name)
{
	HANDLE mutexHandle = CreateMutexW(mutexAttributes, FALSE, name);
	if (mutexHandle != NULL)
	{
		DWORD r = WaitForSingleObject(mutexHandle, INFINITE);
		if (r == WAIT_OBJECT_0 || r == WAIT_ABANDONED)return mutexHandle;
		CloseHandle(mutexHandle);
	}
	return NULL;
}

void Sync::_freeMutex(HANDLE mutex)
{
	ReleaseMutex(mutex);
	CloseHandle(mutex);
}
