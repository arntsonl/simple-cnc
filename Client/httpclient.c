#include "httpclient.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define MAX_BUFLEN 1024
#define MAX_RECVLEN 4096 // only accept 4kb for commands only
#define MAX_FILELEN 10485760 // up to 10MB for a file

#define USER_AGENT "User - Agent : Mozilla / 5.0 (Windows NT 6.1; WOW64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 39.0.2171.95 Safari / 537.36\r\n"

char * connectUrl(char * url)
{
	char * retBuf = NULL;

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	//struct sockaddr_in clientService;

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char sendBuf[MAX_RECVLEN];
	strcpy(sendBuf, "GET ");
	strcat(sendBuf, url);
	strcat(sendBuf, " HTTP/1.1\r\n");
	strcat(sendBuf, "Host: ");

	char * startHost = strstr(url, "://");
	if (startHost == NULL)
		return retBuf;
	char * endHost = strstr(startHost + 3, "/");
	if (endHost == NULL)
		return retBuf;
	strncat(sendBuf, startHost, endHost - startHost);
	
	strcat(sendBuf, "\r\n");
	strcat(sendBuf, "Connection : close\r\n"
		"Accept : text / html\r\n"
		"User - Agent : Mozilla / 5.0 (Windows NT 6.1; WOW64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 39.0.2171.95 Safari / 537.36\r\n"
		"\r\n");

	char recvbuf[MAX_BUFLEN];
	char httpbuf[MAX_RECVLEN];
	int iResult;
	int iTotal = 0;
	int recvbuflen = MAX_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return retBuf;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("localhost", "3000", &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return retBuf;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return retBuf; // dont fail
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return retBuf;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendBuf, (int)strlen(sendBuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return retBuf;
	}

	printf("Bytes Sent: %ld\n", iResult);

	memset(httpbuf, 0, MAX_RECVLEN*sizeof(char));

	// Receive until the peer closes the connection
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			iTotal += iResult;
			if (iTotal > MAX_RECVLEN)
				break; // no more!
			memcpy(httpbuf + (iTotal-iResult), recvbuf, sizeof(char)*iResult);
			httpbuf[iTotal] = '\0';
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());
	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	// find connection close, give everything after that
	char * offset = strstr(httpbuf, "Connection: close\r\n\r\n");
	if (offset != NULL)
	{
		retBuf = malloc(sizeof(char)*MAX_RECVLEN);
		strncpy(retBuf, offset+21, MAX_RECVLEN);
	}

	return retBuf;
}

void postUrl(char * url, char * data)
{
	char * retBuf = NULL;

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	//struct sockaddr_in clientService;

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char sendBuf[MAX_RECVLEN];
	strcpy(sendBuf, "POST ");
	strcat(sendBuf, url);
	strcat(sendBuf, " HTTP/1.1\r\n");
	strcat(sendBuf, "Host: ");

	char * startHost = strstr(url, "://");
	if (startHost == NULL)
		return;
	char * endHost = strstr(startHost + 3, "/");
	if (endHost == NULL)
		return;
	strncat(sendBuf, startHost, endHost - startHost);

	strcat(sendBuf, "\r\n");
	strcat(sendBuf, 
		"Connection: close\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Content-Length: ");

	char tempBuf[1024];
	_itoa(strlen(data), tempBuf, 10);
	strcat(sendBuf, tempBuf);
	strcat(sendBuf, "\r\n"
		USER_AGENT
		"Accept: text/html\r\n"
		"Accept-Language: en-US,en;q=0.8\r\n"
		"\r\n");
	strcat(sendBuf, data);
	strcat(sendBuf, "\r\n");

	char recvbuf[MAX_BUFLEN];
	char httpbuf[MAX_RECVLEN];
	int iResult;
	int iTotal = 0;
	int recvbuflen = MAX_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("localhost", "3000", &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return; // dont fail
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendBuf, (int)strlen(sendBuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return;
	}

	printf("Bytes Sent: %ld\n", iResult);

	memset(httpbuf, 0, MAX_RECVLEN*sizeof(char));

	// Receive until the peer closes the connection
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			iTotal += iResult;
			if (iTotal > MAX_RECVLEN)
				break; // no more!
			memcpy(httpbuf + (iTotal - iResult), recvbuf, sizeof(char)*iResult);
			httpbuf[iTotal] = '\0';
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());
	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
}


void getTemporaryFile(char * url)
{
	char * retBuf = NULL;

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	//struct sockaddr_in clientService;

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char sendBuf[MAX_RECVLEN];
	strcpy(sendBuf, "GET ");
	strcat(sendBuf, url);
	strcat(sendBuf, " HTTP/1.1\r\n");
	strcat(sendBuf, "Host: ");

	char * startHost = strstr(url, "://");
	if (startHost == NULL)
		return retBuf;
	char * endHost = strstr(startHost + 3, "/");
	if (endHost == NULL)
		return retBuf;
	strncat(sendBuf, startHost, endHost - startHost);

	strcat(sendBuf, "\r\n");
	strcat(sendBuf, "Connection : close\r\n"
		"Accept : text / html\r\n"
		"User - Agent : Mozilla / 5.0 (Windows NT 6.1; WOW64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 39.0.2171.95 Safari / 537.36\r\n"
		"\r\n");

	char recvbuf[MAX_BUFLEN];
	char httpbuf[MAX_RECVLEN];
	int iResult;
	int iTotal = 0;
	int recvbuflen = MAX_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return retBuf;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("localhost", "3000", &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return retBuf;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return retBuf; // dont fail
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return retBuf;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendBuf, (int)strlen(sendBuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return retBuf;
	}

	printf("Bytes Sent: %ld\n", iResult);

	memset(httpbuf, 0, MAX_RECVLEN*sizeof(char));

	// Receive until the peer closes the connection
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			iTotal += iResult;
			if (iTotal > MAX_RECVLEN)
				break; // no more!
			memcpy(httpbuf + (iTotal - iResult), recvbuf, sizeof(char)*iResult);
			httpbuf[iTotal] = '\0';
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());
	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	// find connection close, give everything after that
	char * offset = strstr(httpbuf, "Connection: close\r\n\r\n");
	if (offset != NULL)
	{
		retBuf = malloc(sizeof(char)*MAX_RECVLEN);
		strncpy(retBuf, offset + 21, MAX_RECVLEN);
	}

	return retBuf;
}

// Download a file and return the temporary name
char * downloadFile(char * url)
{
	char * retBuf = NULL;

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	//struct sockaddr_in clientService;

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char sendBuf[MAX_FILELEN];
	strcpy(sendBuf, "GET ");
	strcat(sendBuf, url);
	strcat(sendBuf, " HTTP/1.1\r\n");
	strcat(sendBuf, "Host: ");

	char * startHost = strstr(url, "://");
	if (startHost == NULL)
		return retBuf;
	char * endHost = strstr(startHost + 3, "/");
	if (endHost == NULL)
		return retBuf;
	strncat(sendBuf, startHost, endHost - startHost);

	strcat(sendBuf, "\r\n");
	strcat(sendBuf, "Connection : close\r\n"
		"Accept : text / html\r\n"
		"User - Agent : Mozilla / 5.0 (Windows NT 6.1; WOW64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 39.0.2171.95 Safari / 537.36\r\n"
		"\r\n");

	char recvbuf[MAX_BUFLEN];
	char httpbuf[MAX_FILELEN];
	int iResult;
	int iTotal = 0;
	int recvbuflen = MAX_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return retBuf;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; //AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("localhost", "3000", &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return retBuf;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return retBuf; // dont fail
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return retBuf;
	}

	// Send an initial buffer
	iResult = send(ConnectSocket, sendBuf, (int)strlen(sendBuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return retBuf;
	}

	printf("Bytes Sent: %ld\n", iResult);

	memset(httpbuf, 0, MAX_FILELEN*sizeof(char));

	// Receive until the peer closes the connection
	do {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			iTotal += iResult;
			if (iTotal > MAX_FILELEN)
				break; // no more!
			memcpy(httpbuf + (iTotal - iResult), recvbuf, sizeof(char)*iResult);
			httpbuf[iTotal] = '\0';
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());
	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	// find connection close, give everything after that
	char * offset = strstr(httpbuf, "Connection: close\r\n\r\n");
	if (offset != NULL)
	{
		retBuf = malloc(sizeof(char)*MAX_FILELEN);
		strncpy(retBuf, offset + 21, MAX_FILELEN);
	}

	return retBuf;
}
