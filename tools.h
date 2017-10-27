#pragma once

void generateBotId(char * buffer, int size);
std::string hexencode(const std::string& input);
bool b64encodeA(BYTE * data, DWORD dataSize, LPSTR * dataOut);
bool b64decodeA(LPCSTR data, DWORD dataSize, LPBYTE * dataOut, DWORD * dataSizeOut);

#define OS_VERSION_VISTA 6

DWORD getOsVersion();