#include "stdafx.h"
#include "registry.h"

bool Registry::_setValueAsString(HKEY key, const LPWSTR subKey, const LPWSTR value, const LPWSTR buffer, DWORD bufferSize)
{
	return _setValueAsBinary(key, subKey, value, REG_SZ, (LPBYTE)buffer, bufferSize * sizeof(WCHAR) + sizeof(WCHAR));
}

bool Registry::_setValueAsBinary(HKEY key, const LPWSTR subKey, const LPWSTR value, DWORD type, const void *buffer, DWORD bufferSize)
{
	bool retVal = false;
	if (RegCreateKeyExW(key, subKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &key, NULL) == ERROR_SUCCESS)
	{
		if (RegSetValueExW(key, value, 0, type, (LPBYTE)buffer, bufferSize) == ERROR_SUCCESS)retVal = true;
		RegCloseKey(key);
	}
	return retVal;
}