#include "stdafx.h"
#include "threadsgroup.h"

void ThreadsGroup::init(void)
{

}

void ThreadsGroup::uninit(void)
{

} 

void ThreadsGroup::_createGroup(GROUP *group)
{
	ZeroMemory(group, sizeof(GROUP));
}

void ThreadsGroup::_closeTerminatedHandles(GROUP *group)
{
	BYTE i = 0, j = 0;
	for (; i < group->count; i++)if (group->handles[i] != NULL)
	{
		if (WaitForSingleObject(group->handles[i], 0) == WAIT_OBJECT_0)
		{
			CloseHandle(group->handles[i]);
			group->handles[i] = NULL;
		}
		else
		{
			group->handles[j] = group->handles[i];
			j++;
		}
	}

	group->count = j;
}

DWORD ThreadsGroup::_numberOfActiveThreads(GROUP *group)
{
	DWORD count = 0;
	for (BYTE i = 0; i < group->count; i++)if (group->handles[i] != NULL &&  WaitForSingleObject(group->handles[i], 0) == WAIT_TIMEOUT)count++;
	return count;
}

bool ThreadsGroup::_createThread(GROUP *group, SIZE_T stackSize, LPTHREAD_START_ROUTINE startAddress, LPVOID parameter, LPDWORD threadId, HANDLE *threadHandle)
{
	if (group->count >= MAXIMUM_WAIT_OBJECTS)
	{
		SetLastError(ERROR_TOO_MANY_TCBS);
		return false;
	}

	if (startAddress)
	{
		HANDLE handle = CreateThread(NULL, stackSize, startAddress, parameter, 0, threadId);
		if (handle == NULL)return false;

		group->handles[group->count++] = handle;
		if (threadHandle != NULL)*threadHandle = handle;
	}
	return true;
}

bool ThreadsGroup::_waitForAllExit(GROUP *group, DWORD timeout)
{
	return (group->count == 0 || WaitForMultipleObjects(group->count, group->handles, TRUE, timeout) == WAIT_OBJECT_0);
}

void ThreadsGroup::_closeGroup(GROUP *group)
{
	for (BYTE i = 0; i < group->count; i++)CloseHandle(group->handles[i]);
}
