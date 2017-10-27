#pragma once

#define executeVoidScript(id)				 CSCRA(script_##id, ##id); ScriptModule::_executeScript(script_##id, ##result, ##hasResult);
#define executeScript(id, result, hasResult, error) CSCRA(script_##id, ##id); *error = ScriptModule::_executeScript(script_##id, ##result, ##hasResult);	
#define executeStringScript(script, result, hasResult)				 ScriptModule::_executeScript(script, ##result, ##hasResult);

namespace ScriptModule
{
	typedef struct {
		DWORD id;
		DWORD len;
		DWORD cycle;
		BOOL log;
		void * state;
	}_script_data_t;

	void init(void);
	void uninit(void);
	BOOL _executeScript(const char * script, LPVOID * result, BOOL * hasResult);
	void registerCronScript(DWORD id, DWORD cycle, DWORD len, BOOL log);
};