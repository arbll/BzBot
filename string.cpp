#include "stdafx.h"
#include "string.h"

int String::_LengthW(LPCWSTR pstrStr)
{
	int c = 0;
	if (pstrStr != NULL)while (pstrStr[c] != 0)c++;
	return c;
}

int String::_LengthA(LPCSTR pstrStr)
{
	int c = 0;
	if (pstrStr != NULL)while (pstrStr[c] != 0)c++;
	return c;
}

static int xToUnicode(DWORD codePage, const LPSTR source, int sourceSize, LPWSTR dest, int destSize)
{
	if (sourceSize == -1)sourceSize = String::_LengthA(source);
	int size = MultiByteToWideChar(codePage, 0, source, sourceSize, dest, destSize);
	if (destSize > 0)
	{
		if (size >= destSize)size = 0;
		dest[size] = 0;
	}
	return size;
}

static LPWSTR xToUnicodeEx(DWORD codePage, LPSTR source, int size)
{
	if (size == -1)size = String::_LengthA(source);
	int destSize = xToUnicode(codePage, source, size, NULL, 0);
	if (destSize > 0)
	{
		destSize += sizeof(BYTE);
		LPWSTR dest = (LPWSTR)malloc(destSize * sizeof(WCHAR));
		if (dest != NULL)
		{
			xToUnicode(codePage, source, size, dest, destSize);
			return dest;
		}
	}
	return NULL;
}

static int unicodeToX(DWORD codePage, const LPWSTR source, int sourceSize, LPSTR dest, int destSize)
{
	if (sourceSize == -1)sourceSize = String::_LengthW(source);
	int size = WideCharToMultiByte(codePage, 0, source, sourceSize, dest, destSize, NULL, NULL);
	if (destSize > 0)
	{
		if (size >= destSize)size = 0;
		dest[size] = 0;
	}
	return size;
}

static LPSTR unicodeToXEx(DWORD codePage, const LPWSTR source, int size)
{
	if (size == -1)size = String::_LengthW(source);
	int destSize = unicodeToX(codePage, source, size, NULL, 0);
	if (destSize > 0)
	{
		destSize += sizeof(BYTE);
		LPSTR dest = (LPSTR)malloc(destSize * sizeof(BYTE));
		if (dest != NULL)
		{
			unicodeToX(codePage, source, size, dest, destSize);
			return dest;
		}
	}
	return NULL;
}

int String::_ansiToUnicode(LPSTR source, int sourceSize, LPWSTR dest, int destSize)
{
	return ::xToUnicode(CP_ACP, source, sourceSize, dest, destSize);
}      

LPWSTR String::_ansiToUnicodeEx(LPSTR source, int size)
{
	return xToUnicodeEx(CP_ACP, source, size);
}

LPSTR String::_unicodeToAnsiEx(const LPWSTR source, int size)
{
	return unicodeToXEx(CP_ACP, source, size);
}

int String::_unicodeToAnsi(const LPWSTR source, int sourceSize, LPSTR dest, int destSize)
{
	return unicodeToX(CP_ACP, source, sourceSize, dest, destSize);
}