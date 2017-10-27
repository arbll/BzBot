#pragma once

namespace ThreadsGroup
{
	typedef struct
	{
		BYTE count;
		HANDLE handles[MAXIMUM_WAIT_OBJECTS];
	}GROUP;

	void init(void);
	void uninit(void);

	void _createGroup(GROUP *group);
	void _closeTerminatedHandles(GROUP *group);
	DWORD _numberOfActiveThreads(GROUP *group);
	bool _createThread(GROUP *group, SIZE_T stackSize, LPTHREAD_START_ROUTINE startAddress, LPVOID parameter, LPDWORD threadId, HANDLE *threadHandle);
	bool _waitForAllExit(GROUP *group, DWORD timeout);
	void _closeGroup(GROUP *group);
};
