#include "stdafx.h"
#include "filesystem.h"

bool FileSystem::combinePath(LPWSTR dest, const LPWSTR dir, const LPWSTR file)
{
	LPWSTR p = (LPWSTR)file;
	if (p != NULL)while (*p == '\\' || *p == '/')p++;
	return PathCombineW(dest, dir, p) == NULL ? false : true;
}

bool FileSystem::createDirectoryTree(LPWSTR path, LPSECURITY_ATTRIBUTES securityAttributes)
{
	bool r = false;
	LPWSTR p = PathSkipRootW(path);

	if (p == NULL)p = path;

	for (;; p++)if (*p == '\\' || *p == '/' || *p == 0)
	{
		WCHAR cold = *p;
		*p = 0;

		DWORD attr = GetFileAttributesW(path);
		if (attr == INVALID_FILE_ATTRIBUTES)
		{
			if (CreateDirectoryW(path, securityAttributes) == FALSE)break;
		}
		else if ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0)break;

		if (cold == 0) { r = true; break; }

		*p = cold;
	}
	return r;
}

bool FileSystem::fileToMemory(LPWSTR filePath, memory_file_t * memoryFile)
{
	memoryFile->file = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (memoryFile->file != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;

		//On vérrifie si le fichier n'est pas trop grand ( > sizeof(LONG) )
		if (GetFileSizeEx(memoryFile->file, &fileSize) && fileSize.HighPart == 0)
		{
			memoryFile->size = fileSize.LowPart;

			if (memoryFile->size == 0)
			{
				memoryFile->data = NULL;
				return true;
			}

			if ((memoryFile->data = (LPBYTE)VirtualAlloc(NULL, memoryFile->size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)) != NULL)
			{
				DWORD readed;
				if (ReadFile(memoryFile->file, memoryFile->data, memoryFile->size, &readed, NULL) != FALSE && readed == memoryFile->size)
				{
					return true;
				}
				VirtualFree(memoryFile->data, 0, MEM_RELEASE);
			}
		}
		CloseHandle(memoryFile->file);
	}
	return false;
}

void FileSystem::closeMemoryFile(memory_file_t *memoryFile)
{
	if (memoryFile->data) VirtualFree(memoryFile->data, 0, MEM_RELEASE);
	if (memoryFile->file) CloseHandle(memoryFile->file);
}

bool FileSystem::deleteFile(LPWSTR filePath)
{
	SetFileAttributesW(filePath, FILE_ATTRIBUTE_NORMAL);
	return DeleteFileW(filePath) ? true : false;
}

bool FileSystem::saveDataToFile(const LPWSTR filePath, const void *data, DWORD dataSize)
{
	bool r = false;
	HANDLE file = CreateFileW(filePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE)
	{
		DWORD size;
		if (data == NULL || dataSize == 0 || WriteFile(file, data, dataSize, &size, NULL) != FALSE) r = true;
		CloseHandle(file);
		if (r != true) deleteFile(filePath);
	}
	return r;
}