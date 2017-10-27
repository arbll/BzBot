#include "stdafx.h"
#include "rsa.h"
#include "debug.h"

bool rsaCryptBE(HCRYPTKEY key, BYTE * data, DWORD dataSize, BYTE ** dataOut, DWORD * dataOutSize)
{
	*dataOutSize = dataSize;
	if (!CryptEncrypt(key, 0, TRUE, 0, NULL, dataOutSize, 0))
	{
		WDEBUG("Could not rsa botId : " << GetLastError());
		return false;
	}

	*dataOut = new BYTE[*dataOutSize];
	memcpy_s(*dataOut, *dataOutSize, data, dataSize);
	if (!CryptEncrypt(key, 0, TRUE, 0, *dataOut, &dataSize, *dataOutSize))
	{
		WDEBUG("Could not rsa botId : " << GetLastError());
		return false;
	}

	for (unsigned int i = 0; i < (*dataOutSize / 2); i++)
	{
		BYTE c = (*dataOut)[i];
		(*dataOut)[i] = (*dataOut)[(*dataOutSize) - 1 - i];
		(*dataOut)[(*dataOutSize) - 1 - i] = c;
	}
	return true;
}
