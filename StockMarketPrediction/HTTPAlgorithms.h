#ifndef HTTP_ALGORITHMS
#define HTTP_ALGORITHMS

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <string>
#include <sstream>
#include <WinSock2.h>
#include <minwinbase.h>
#include "StringAlgorithms.h"

#pragma comment(lib,"ws2_32.lib")

using namespace std;

string RepairLinks(string webpagedata, string hostname)
{
	int Index = 0;
	while ((Index = IndexOf(webpagedata, "=\"//&", Index, '&')) != -1)
		webpagedata.insert(Index - 3, "http:");

	Index = 0;
	while ((Index = IndexOf(webpagedata, "=\"/&", Index, '&')) != -1)
		webpagedata.insert(Index - 2, "http://" + hostname);

	return webpagedata;
}

string RemoveHTTPHeader(string webpagedata)
{
	int Index = IndexOf(webpagedata, "\r\n\r\n", 0);
	webpagedata.erase(0, Index - 1);

	return webpagedata;
}

string WebPageTitle(string WebPageData)
{
	stringstream ss;

	int initialIndex = IndexOf(WebPageData, "<title", 0) - 1;

	if (initialIndex > -1)
	{
		int startingIndex = IndexOf(WebPageData, ">", initialIndex);
		int endingIndex = IndexOf(WebPageData, "<", startingIndex);

		if (endingIndex - startingIndex == 1)
			ss << "No Title Found";

		for (int i = startingIndex; i < endingIndex - 1; i++)
		{
			if (WebPageData[i] == '<')
				i = IndexOf(WebPageData, ">", i) - 1;
			else
				ss << WebPageData[i];
		}
	}
	else
		ss << "No Title Found";

	return ss.str();
}

string RetrieveWebpage(string hostname, string webpage, int * sent = NULL, int * recieved = NULL)
{
	if (sent != NULL)
		*sent = 0;
	if (recieved != NULL)
		*recieved = 0;

	WSADATA wsaData;

	char * sendbuf = new char[sizeof("GET / HTTP/1.1\r\nHost: \r\nConnection: close\r\n\r\n") + strlen(hostname.c_str()) + strlen(webpage.c_str())];
	if (sent != NULL)
		*sent = sizeof("GET / HTTP/1.1\r\nHost: \r\nConnection: close\r\n\r\n") + (int)strlen(hostname.c_str()) + (int)strlen(webpage.c_str());
	sprintf(sendbuf, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", webpage.c_str(), hostname.c_str());

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct hostent *host;
	host = gethostbyname(hostname.c_str());

	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(80);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr));

	send(Socket, sendbuf, (int)strlen(sendbuf), 0);

	char buffer[512];

	stringstream ss;

	int rec;
	while ((rec = recv(Socket, buffer, sizeof(buffer), 0)) > 0)
	{
		int i = 0;
		while (buffer[i] != '\0' && i < 512)
		{
			ss << buffer[i++];
			if (recieved != NULL)
				*recieved += 1;
		}
		ZeroMemory(buffer, sizeof(buffer));
	}

	closesocket(Socket);
	WSACleanup();
	delete[] sendbuf;
	return ss.str();
}

void ParseURL(string URL, string * hostname, string * webpage)
{
	int i = 0, forwardslashes = 2;
	stringstream hn, wn;
	for (int i = 0; i < (int)URL.length(); i++)
	{
		if (URL[i] == '/' && forwardslashes > 0) forwardslashes--;
		else if (URL[i] == '/' && forwardslashes == 0) { forwardslashes--; i--; }
		else if (forwardslashes == 0) hn << URL[i];
		else if (forwardslashes == -1) wn << URL[i];
	}

	*hostname = hn.str();
	*webpage = wn.str();
}

int AnalyzeHeader(string input)
{
	stringstream ss;
	ss << input[9]; ss << input[10]; ss << input[11];

	int output;

	ss >> output;

	return output;
}

string HTMLTextParser(string input)
{
	stringstream ss;

	int tags = 0; bool didWrite = false;
	for (int i = 0; i < (int)input.length() - 5; i++)
	{
		if (input[i] == '<' && input[i + 1] == 'p' && tags == 0) { tags++; i = IndexOf(input, ">", i) - 1; }
		else if (input[i] == '<' && input[i + 1] == '/' && input[i + 2] == 'p'&& tags == 1) { tags--; i = IndexOf(input, ">", i) - 1; if (didWrite == true) { ss << "\r\n\r\n"; didWrite = false; } }
		else if (input[i] == '<' && input[i + 1] == 'h' && (input[i + 2] == '1' || input[i + 2] == '2' || input[i + 2] == '3' || input[i + 2] == '4' || input[i + 2] == '5' || input[i + 2] == '6') && tags == 0) { tags++; i = IndexOf(input, ">", i) - 1; }
		else if (input[i] == '<' && input[i + 1] == '/' && input[i + 2] == 'h' && (input[i + 3] == '1' || input[i + 3] == '2' || input[i + 3] == '3' || input[i + 3] == '4' || input[i + 3] == '5' || input[i + 3] == '6') && tags == 1) { tags--; i = IndexOf(input, ">", i) - 1; if (didWrite == true) { ss << "\r\n\r\n"; didWrite = false; } }
		else if (input[i] == '<' && tags == 1) tags++;
		else if (input[i] == '>' && tags == 2) tags--;
		else if (tags == 1) if (input[i] != '\r' && input[i] != '\n' && input[i] != '\t') { ss << input[i]; didWrite = true; }
	}
	return ss.str();
}

#endif