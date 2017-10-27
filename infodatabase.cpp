#include "stdafx.h"
#include "debug.h"
#include "infodatabase.h"
#include "defines.h"
#include "tools.h"
#include "filesystem.h"
#include "kernel.h"
#include "sync.h"

void InfoDatabase::init()
{
	
}

void InfoDatabase::uninit()
{

}

bool InfoDatabase::createDefaultDatabase(const LPWSTR pathHome, const LPWSTR databaseFile, info_database_t * infoDatabase)
{
	GetSystemTime(&infoDatabase->header.installDate);
	generateBotId(infoDatabase->header.botId, sizeof(infoDatabase->header.botId));
	infoDatabase->header.xorHkey = rand();
	infoDatabase->header.itemCount = 0;
	infoDatabase->header.itemsSize = 0;
	memcpy_s(infoDatabase->header.magic, 3, INFODATABASE_MAGIC, 3);
	if (!FileSystem::combinePath(infoDatabase->path, pathHome, databaseFile)) return false;
	return true;
}

static bool saveDatabaseFile(int maxRetry, InfoDatabase::info_database_t const& infoDatabase)
{
	InfoDatabase::info_database_header_t packedDatabase;
	memcpy_s(&packedDatabase, sizeof(InfoDatabase::info_database_header_t), &infoDatabase.header, sizeof(InfoDatabase::info_database_header_t));

	char xorHkey = packedDatabase.xorHkey;

	for (int i = 0; i < sizeof(packedDatabase.garbage); i++)
	{
		((char*)(&packedDatabase))[i] = rand() % 0xFF;
	}

	for (int i = sizeof(packedDatabase.garbage); i < sizeof(InfoDatabase::info_database_header_t); i++)
	{
		((char*)(&packedDatabase))[i] ^= xorHkey;
	}

	packedDatabase.xorHkey = xorHkey;

	for (int loop = 0;; loop++)
	{
		HANDLE mutex = Kernel::waitForMutexOfObject(MUTEX_INFODATABASE);
		if (mutex == NULL) return false;
		if (FileSystem::saveDataToFile((WCHAR *)infoDatabase.path, &packedDatabase, sizeof(InfoDatabase::info_database_header_t)))
		{
			Sync::_freeMutex(mutex);
			return true;
		}
		else
		{
			WDEBUG0(WDDT_ERROR, "Failed to save DATABASE file.");
			if (maxRetry >= 0 && maxRetry <= loop)break;
		}
		Sync::_freeMutex(mutex);
		Sleep(5000 + loop);
	}
	return false;
}

bool InfoDatabase::_saveDatabaseHeader(info_database_t * infoDatabase, bool useMutex)
{
	WCHAR directory[MAX_PATH];

	wcscpy_s(directory, infoDatabase->path);

	if (!PathRemoveFileSpec(directory)) return false;

	if (GetFileAttributes(directory) == INVALID_FILE_ATTRIBUTES)
	{
		if (!FileSystem::createDirectoryTree(directory, NULL)) return false;
	}

	HANDLE mutex = NULL;
	
	if(useMutex) 
		mutex = Kernel::waitForMutexOfObject(MUTEX_INFODATABASE);

	if (!useMutex || mutex != NULL)
	{
		HANDLE file = CreateFile(infoDatabase->path, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (file != INVALID_HANDLE_VALUE)
		{
			CloseHandle(file);
			saveDatabaseFile(10, *infoDatabase);
			if(useMutex) Sync::_freeMutex(mutex);
			return true;
		}
		if(useMutex) Sync::_freeMutex(mutex);
	}
	WDEBUG0(WDDT_ERROR, "Can't create database " << infoDatabase->path);
	return false;
}

bool InfoDatabase::loadDatabase(const LPWSTR pathHome, const LPWSTR databaseFile, info_database_t * infoDatabase)
{
	WCHAR path[MAX_PATH];

	if (!FileSystem::combinePath(path, pathHome, databaseFile)) return false;

	if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES) return false;

	wcscpy_s(infoDatabase->path, path);

	HANDLE mutex = Kernel::waitForMutexOfObject(MUTEX_INFODATABASE);
	if (mutex == NULL) return false;

	HANDLE file = CreateFile(infoDatabase->path, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		DWORD byteRead;
		if (ReadFile(file, &infoDatabase->header, sizeof(InfoDatabase::info_database_header_t), &byteRead, NULL) != FALSE &&
			byteRead == sizeof(InfoDatabase::info_database_header_t))
		{
			char xorHkey = infoDatabase->header.xorHkey;
			for (int i = sizeof(infoDatabase->header.garbage); i < sizeof(InfoDatabase::info_database_header_t); i++)
			{
				((char*)(&infoDatabase->header))[i] ^= xorHkey;
			}
			infoDatabase->header.xorHkey = xorHkey;
			CloseHandle(file);
			Sync::_freeMutex(mutex);
			return true;
		}
	}
	Sync::_freeMutex(mutex);
	return false;
}

bool InfoDatabase::addItem(info_database_t * infoDatabase, unsigned short type, unsigned int size, unsigned int flags, const char * buffer)
{
	if (GetFileAttributes(infoDatabase->path) == INVALID_FILE_ATTRIBUTES) return false;

	if (flags & ITEM_FLAG_UNIQUE && hasItemOfType(infoDatabase, type)) return false;

	HANDLE mutex = Kernel::waitForMutexOfObject(MUTEX_INFODATABASE);
	if (mutex == NULL) return false;

	HANDLE file = CreateFile(infoDatabase->path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		char xorKey = rand() % 0xFF;
		info_database_item_header_t header = { type, size, flags, xorKey };

		for (int i = 0; i < sizeof(header); i++)
		{
			((char*)&header)[i] ^= xorKey;
		}

		header.xorKey = xorKey;

		char * xorBuffer = new char[size];

		for (unsigned int i = 0; i < size; i++)
		{
			xorBuffer[i] = buffer[i] ^ xorKey;
		}
		
		SetFilePointer(file, (LONG)infoDatabase->header.itemsSize + sizeof(info_database_header_t), NULL, FILE_BEGIN);

		DWORD writenHeader;
		DWORD writenBuffer;
		if (!WriteFile(file, &header, sizeof(info_database_item_header_t), &writenHeader, NULL)
			|| !WriteFile(file, xorBuffer, size, &writenBuffer, NULL))
		{
			delete xorBuffer;
			CloseHandle(file);
			Sync::_freeMutex(mutex);
			return false;
		}
		delete xorBuffer;
		CloseHandle(file);
		infoDatabase->header.itemsSize += sizeof(info_database_item_header_t) + size;
		infoDatabase->header.itemCount += 1;
		bool result = _saveDatabaseHeader(infoDatabase, false);
		Sync::_freeMutex(mutex);
		return result;
	}
	return false;
}

bool InfoDatabase::_saveItemHeader(const info_database_t * infoDatabase, const info_database_item_header_t * header, unsigned long headerOffset)
{
	if (GetFileAttributes(infoDatabase->path) == INVALID_FILE_ATTRIBUTES) return false;

	HANDLE mutex = Kernel::waitForMutexOfObject(MUTEX_INFODATABASE);
	if (mutex == NULL) return false;

	HANDLE file = CreateFile(infoDatabase->path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		char xorKey = header->xorKey;
		info_database_item_header_t newHeader;
		memcpy_s(&newHeader, sizeof(info_database_item_header_t), header, sizeof(info_database_item_header_t));

		for (int i = 0; i < sizeof(newHeader); i++)
		{
			((char*)&newHeader)[i] ^= xorKey;
		}

		newHeader.xorKey = xorKey;

		SetFilePointer(file, headerOffset + sizeof(info_database_header_t), NULL, FILE_BEGIN);

		DWORD writenHeader;
		if (!WriteFile(file, &newHeader, sizeof(info_database_item_header_t), &writenHeader, NULL))
		{
			CloseHandle(file);
			Sync::_freeMutex(mutex);
			return false;
		}
		CloseHandle(file);
		return true;
	}
	return false;
}

bool InfoDatabase::_readItemHeader(const info_database_t * infoDatabase, unsigned long headerOffset, info_database_item_header_t * itemHeader)
{
	if (GetFileAttributes(infoDatabase->path) == INVALID_FILE_ATTRIBUTES) return false;

	HANDLE mutex = Kernel::waitForMutexOfObject(MUTEX_INFODATABASE);
	if (mutex == NULL) return false;

	HANDLE file = CreateFile(infoDatabase->path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		if (infoDatabase->header.itemsSize >= headerOffset + sizeof(info_database_item_header_t))
		{
			SetFilePointer(file, headerOffset + sizeof(info_database_header_t), NULL, FILE_BEGIN);
			DWORD byteRead;
			if (ReadFile(file, itemHeader, sizeof(info_database_item_header_t), &byteRead, NULL) && byteRead == sizeof(info_database_item_header_t))
			{
				char xorKey = itemHeader->xorKey;

				for (int i = 0; i < sizeof(info_database_item_header_t); i++)
				{
					((char*)itemHeader)[i] ^= xorKey;
				}

				itemHeader->xorKey = xorKey;
				CloseHandle(file);
				Sync::_freeMutex(mutex);
				return true;
			}
		}
		
		CloseHandle(file);
	}
	Sync::_freeMutex(mutex);
	return false;
}

bool InfoDatabase::_readItemData(const info_database_t * infoDatabase, info_database_item_header_t * itemHeader, unsigned long offset, char * buffer)
{
	if (GetFileAttributes(infoDatabase->path) == INVALID_FILE_ATTRIBUTES) return false;

	HANDLE mutex = Kernel::waitForMutexOfObject(MUTEX_INFODATABASE);
	if (mutex == NULL) return false;

	HANDLE file = CreateFile(infoDatabase->path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		if (infoDatabase->header.itemsSize >= offset + sizeof(info_database_item_header_t) + itemHeader->size)
		{
			SetFilePointer(file, offset + sizeof(info_database_header_t) + sizeof(info_database_item_header_t), NULL, FILE_BEGIN);
			DWORD byteRead;
			if (ReadFile(file, buffer, itemHeader->size, &byteRead, NULL) && byteRead == itemHeader->size)
			{
				char xorKey = itemHeader->xorKey;

				for (unsigned int i = 0; i < itemHeader->size; i++)
				{
					buffer[i] ^= xorKey;
				}
				CloseHandle(file);
				Sync::_freeMutex(mutex);
				return true;
			}
		}
		CloseHandle(file);
	}
	Sync::_freeMutex(mutex);
	return false;
}

bool InfoDatabase::firstItem(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor)
{
	if (infoDatabase->header.itemCount < 1) return false;
	
	cursor->itemNumber = 1;
	cursor->currentItemHeaderOffset = 0;

	if (!_readItemHeader(infoDatabase, cursor->currentItemHeaderOffset, &cursor->currentItemHeader)) return false;
	return true;
}

bool InfoDatabase::nextItem(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor)
{
	if (infoDatabase->header.itemCount < cursor->itemNumber + 1) return false;

	info_database_item_header_t itemHeader;
	unsigned int newOffset = cursor->currentItemHeaderOffset + cursor->currentItemHeader.size + sizeof(info_database_item_header_t);

	if (!_readItemHeader(infoDatabase, newOffset, &itemHeader)) return false;

	cursor->itemNumber++;
	cursor->currentItemHeaderOffset = newOffset;
	memcpy_s(&cursor->currentItemHeader, sizeof(info_database_item_header_t), &itemHeader, sizeof(info_database_item_header_t));
	return true;
}

bool InfoDatabase::openItem(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor)
{
	cursor->itemData = new char[cursor->currentItemHeader.size];

	if (!_readItemData(infoDatabase, &cursor->currentItemHeader, cursor->currentItemHeaderOffset, cursor->itemData))
	{
		delete cursor->itemData;
		return false;
	}
	return true;
}

bool InfoDatabase::saveItemHeader(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor)
{
	return _saveItemHeader(infoDatabase, &cursor->currentItemHeader, cursor->currentItemHeaderOffset);
}

void InfoDatabase::closeItem(info_database_item_cursor_t * cursor)
{
	delete cursor->itemData;
}

bool InfoDatabase::hasItemOfType(const info_database_t * infoDatabase, unsigned short type)
{
	info_database_item_cursor_t cursor;
	firstItem(infoDatabase, &cursor);
	while (cursor.currentItemHeader.type != type && nextItem(infoDatabase, &cursor));
	return cursor.currentItemHeader.type == type;
}

bool InfoDatabase::nextItemOfType(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor, unsigned short type)
{
	while (nextItem(infoDatabase, cursor) && cursor->currentItemHeader.type != type);
	return cursor->currentItemHeader.type == type;
}

bool InfoDatabase::nextItemWithFlag(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor, unsigned int flag)
{
	while (nextItem(infoDatabase, cursor) && !(cursor->currentItemHeader.flags & flag));
	return cursor->currentItemHeader.flags & flag;
}




