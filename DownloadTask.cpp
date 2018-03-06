#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <WinSock2.h>
#include "HttpDownload.h"
void usage() {
	//printf("Usage: %s http://www.xxx.com/filename %s\n", g_progName, g_saveFileName);
}

void parseURL(const char* url, char* hostAddr, int& port, char* getPath) {
	if (url == NULL || hostAddr == NULL || getPath == NULL)
		return;
	const char* temp = strstr(url, "http://");
	if (temp == NULL)
		return;
	const char* hostStart = temp + strlen("http://");
	const char* colon = strchr(hostStart, ':');
	if (colon != NULL)    //表示存在冒号,有端口号
		sscanf(hostStart, "%[^:]:%d%s", hostAddr, &port, getPath);
	else
		sscanf(hostStart, "%[^/]%s", hostAddr, getPath);
	//通过主机名转IP地址
	struct hostent* hostEntry;
	hostEntry = gethostbyname(hostAddr);
	if (hostEntry == NULL)
	{
		printf("Hostname not available!\n");
		return;
	}
	struct in_addr inAddr = {0};
	memcpy(&inAddr.s_addr, hostEntry->h_addr, sizeof(inAddr.s_addr));
	strcpy(hostAddr, inet_ntoa(inAddr));
}

void* task(void* arg) {
	HttpDownload* pObj = (HttpDownload*)arg;
	if (pObj->start())
		return ((void*)1);
	else
		return ((void*)0);
}
