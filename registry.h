#pragma once

namespace Registry
{
	bool _setValueAsString(HKEY key, const LPWSTR subKey, const LPWSTR value, const LPWSTR buffer, DWORD bufferSize);
	bool _setValueAsBinary(HKEY key, const LPWSTR subKey, const LPWSTR value, DWORD type, const void *buffer, DWORD bufferSize);

};