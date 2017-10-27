#pragma once

#define HTTP_ERR_SUCCESS 0x0 
#define HTTP_ERR_COULD_NOT_CONNECT 0x1
#define HTTP_ERR_NO_SESSION 0x2
#define HTTP_ERR_COULD_NOT_SEND_REQUEST 0x3
#define HTTP_ERR_QUERY_FAILED 0x4
#define HTTP_ERR_INVALID_CONNECTION 0x5
#define HTTP_ERR_COULD_NOT_READ_FILE 0x6
#define HTTP_ERR_HEADER_NOT_SET 0x7


namespace Http
{
	typedef struct {
		LPWSTR url;
		HINTERNET request;
		HINTERNET connection;
	} HTTP_REQUEST;

	DWORD HTTPInitRequest(HTTP_REQUEST* id, LPCWSTR url);
	DWORD HTTPCloseRequest(HTTP_REQUEST* id);
	DWORD HTTPSetHeader(HTTP_REQUEST* id, LPCWSTR name, LPCWSTR data);
	DWORD HTTPSendRequest(HTTP_REQUEST* id, LPDWORD responseCode, LPDWORD contentLength, LPCWSTR verb, const char * postData = NULL);
	DWORD HTTPReadRequest(HTTP_REQUEST* id, LPSTR responseBuffer, DWORD bufferLength, LPDWORD bytesRead);
	
	BOOL  HTTPCheckInternet();
};