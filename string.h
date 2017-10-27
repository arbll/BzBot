#pragma once

namespace String
{
	int _LengthW(LPCWSTR pstrStr);
	int _LengthA(LPCSTR pstrStr);
	int _ansiToUnicode(LPSTR source, int sourceSize, LPWSTR dest, int destSize);
	LPWSTR _ansiToUnicodeEx(LPSTR source, int size);
	LPSTR _unicodeToAnsiEx(const LPWSTR source, int size);
	int _unicodeToAnsi(const LPWSTR source, int sourceSize, LPSTR dest, int destSize);
};