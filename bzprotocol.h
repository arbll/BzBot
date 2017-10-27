#pragma once

#define BZ_MAX_LENGTH 1024 * 1000 * 5
#ifdef _DEBUG
#define BZ_SEND_CLOCK 10 * 1000
#define BZ_HEARTBEAT_CLOCK 10 * 1000
#else
#define BZ_SEND_CLOCK 5 * 60 * 1000
#define BZ_HEARTBEAT_CLOCK 10 * 60 * 1000
#endif

#define TYPE_HEARTBEAT -8347

static unsigned char send_key[] = { 0xc2, 0x70, 0xba, 0x6d, 0x54, 0x36, 0xea, 0xee, 0x45, 0x41, 0x5c, 0x2e, 0x17, 0x8d, 0x7d, 0x96 };
static unsigned char send_iv[] = { 0x2e, 0x28, 0xf5, 0xd4, 0x6a, 0xa5, 0xc9, 0x69, 0xc1, 0xa1, 0xc5, 0xaf, 0xa, 0xec, 0x10, 0x25 };

namespace BzProtocol
{
	enum VALIDATION_ERROR{
		VALIDATION_SUCCESS,
		VALIDATION_ERROR_HOST,
		VALIDATION_ERROR_HASH,
	};

	typedef struct
	{

	}bz_protocol_data_t;

	VALIDATION_ERROR validateHost(LPCWSTR url);
	BOOL sendData(long dataType, BYTE * data, DWORD dataSize);
	DWORD WINAPI _procSender(void *p);
	DWORD WINAPI _procHeartbeat(void * p);
};