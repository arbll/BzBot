#include "stdafx.h"
#include "http.h"
#include "cryptedstrings.h"
#include "debug.h"
#include "tools.h"

BOOL isHttps(LPCWSTR url)
{
	CSTRW(https, protocol_https);

	for (int i = 0; i < lstrlen(https); i++) if (url[i] != https[i]) return FALSE;

	return TRUE;
}

BOOL isHttp(LPCWSTR url)
{
	CSTRW(http, protocol_http);

	for (int i = 0; i < lstrlen(http); i++) if (url[i] != http[i]) return FALSE;

	return TRUE;
}

BOOL isHttpProtocol(LPCWSTR url)
{
	return isHttps(url) ? TRUE : isHttp(url);
}

BOOL strEqual(LPCWSTR a, LPCWSTR b) 
{
	return !lstrcmp(a, b);
}

LPWSTR getDomain(LPCWSTR url) 
{
	CSTRW(http, protocol_http);
	CSTRW(https, protocol_https);
	CSTRW(separator, http_separator);

	LPCWSTR sig = isHttps(url) ? https : http;

	LPCWSTR index = url;

	for (int i = 0; i < lstrlen(sig); i++) {
		if (url[i] == sig[i]) {
			index++;
			continue;
		}
		else return NULL;
	}

	DWORD domainLength = 0;

	int i = 0;
	int size = lstrlen(index);
	while (size > i && index[i++] != separator[0]) domainLength++;

	LPWSTR result = new WCHAR[domainLength + 1];

	memcpy(result, index, domainLength * sizeof(WCHAR));

	result[domainLength] = NULL;

	return result;
}

LPWSTR getPath(LPCWSTR url) 
{

	CSTRW(http, protocol_http);
	CSTRW(https, protocol_https);
	CSTRW(separator, http_separator);

	LPCWSTR sig = isHttps(url) ? https : http;

	LPCWSTR index = url;

	for (int i = 0; i < lstrlen(sig); i++) 
	{
		if (url[i] == sig[i]) 
		{
			index++;
			continue;
		}
		else return NULL;
	}

	DWORD domainLength = 0;

	int i = 0;
	int size = lstrlen(index);
	while (size > i  && index[i++] != separator[0]) domainLength++;
	i = 0;

	index += domainLength;

	DWORD pathLength = 0;

	while (index[i++] != NULL) pathLength++;

	if (pathLength == 0)
		pathLength = 1;

	LPWSTR result = new WCHAR[pathLength+1];

	if (pathLength == 1)
		lstrcpy(result, separator);
	else
		lstrcpy(result, index);

	return result;
}

DWORD Http::HTTPInitRequest(HTTP_REQUEST* id, LPCWSTR url) 
{
	id->url = new WCHAR[lstrlen(url) + 1];

	lstrcpy(id->url, url);

	BOOL https = isHttps(id->url);

	if (!isHttpProtocol(id->url)) return FALSE;

	CSTRW(userAgent, http_user_agent);

	HINTERNET session = InternetOpen(userAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);

	if (!session) 
	{
		return HTTP_ERR_NO_SESSION;
	}

	BOOL ba;
	if (getOsVersion() >= OS_VERSION_VISTA)
	{
		BOOL b = TRUE;
		ba = InternetSetOption(session, INTERNET_OPTION_HTTP_DECODING, &b, sizeof(b));
	}

	LPWSTR domain = getDomain(id->url);
	id->connection = InternetConnect(session, domain, https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	delete domain;

	if (!id->connection)
	{
		return HTTP_ERR_COULD_NOT_CONNECT;
	}

	return HTTP_ERR_SUCCESS;
}

DWORD Http::HTTPCloseRequest(HTTP_REQUEST * id)
{
	InternetCloseHandle(id->request);
	return HTTP_ERR_SUCCESS;
}

DWORD Http::HTTPSetHeader(HTTP_REQUEST* id, LPCWSTR name, LPCWSTR data) 
{
	CSTRW(separator, http_header_separator);
	LPWSTR header = new WCHAR[lstrlen(name) + lstrlen(separator) + lstrlen(data) + 1];

	lstrcpy(header, name);
	lstrcat(header, separator);
	lstrcat(header, data);
	header[lstrlen(header)] = NULL;

	if (!HttpAddRequestHeaders(id->request, header, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE)) {
		delete header;
		return HTTP_ERR_HEADER_NOT_SET;
	}

	delete header;
	return HTTP_ERR_SUCCESS;
}

DWORD Http::HTTPSendRequest(HTTP_REQUEST* id, LPDWORD responseCode, LPDWORD contentLength, LPCWSTR verb, const char * postData) 
{
	CSTRW(post_method, http_method_post);
	CSTRW(http_version, http_version);
	CSTRW(postMIME, http_header_content_type_post);

	BOOL post = strEqual(verb, post_method);
	BOOL https = isHttps(id->url);

	LPWSTR path = getPath(id->url);

	
	id->request = HttpOpenRequest(id->connection, verb, path, http_version, NULL, NULL,  https ? INTERNET_FLAG_SECURE : NULL, NULL);
	
	delete path;
	delete id->url;

	if (!HttpSendRequest(id->request, post ? postMIME : NULL, post ? lstrlen(postMIME) : NULL, (LPVOID)(post ? postData : NULL), post ? strlen(postData) : NULL)) {
		return HTTP_ERR_COULD_NOT_SEND_REQUEST;
	}

	DWORD buffSize = sizeof(DWORD);

	if (!HttpQueryInfo(id->request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, responseCode, &buffSize, NULL)) {
		return HTTP_ERR_QUERY_FAILED;
	}

	DWORD a;

	if (!HttpQueryInfo(id->request, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, contentLength, &buffSize, NULL)) {
		*contentLength = 0;
		a = GetLastError();
	}

	return HTTP_ERR_SUCCESS;
}

DWORD Http::HTTPReadRequest(HTTP_REQUEST* id, LPSTR responseBuffer, DWORD bufferLength, LPDWORD bytesRead) {
	if (!id->request || !id->connection) return HTTP_ERR_INVALID_CONNECTION;

	if (!InternetReadFile(id->request, responseBuffer, bufferLength, bytesRead)) {
		InternetCloseHandle(id->request);
		InternetCloseHandle(id->connection);
		return HTTP_ERR_COULD_NOT_READ_FILE;
	}


	responseBuffer[*bytesRead] = NULL;

	InternetCloseHandle(id->request);
	InternetCloseHandle(id->connection);

	return HTTP_ERR_SUCCESS;
}

BOOL Http::HTTPCheckInternet()
{
	CSTRW(url, http_test_url);
	return InternetCheckConnection(url, FLAG_ICC_FORCE_CONNECTION, 0);
}
