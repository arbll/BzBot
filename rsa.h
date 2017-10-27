#pragma once

bool rsaCryptBE(HCRYPTKEY key, BYTE * data, DWORD dataSize, BYTE ** dataOut, DWORD * dataOutSize);