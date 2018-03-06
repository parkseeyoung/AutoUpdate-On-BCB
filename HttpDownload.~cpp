//---------------------------------------------------------------------------


#pragma hdrstop

#include "HttpDownload.h"
#include <assert.h>
#define BUFFER_SIZE 1024

//---------------------------------------------------------------------------
HttpDownload::HttpDownload(const char* hostAddr, const int port, const char* getPath, const char* saveFileName)
{
	m_hostAddr = hostAddr;
	m_port = port;
	m_getPath = getPath;
	m_saveFileName = saveFileName;
	m_sock = 0;
	m_fp = NULL;
	m_fileTotalSize = 1;    //没给0,因为分母
	m_receivedDataSize = 0;
	m_cancelFlag = false;
}
HttpDownload::~HttpDownload()
{

}
bool HttpDownload::initSocket() 
{
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock < 0)
		return false; 

	//设置Socket为非阻塞模式
	unsigned long mode = 1;
	if (ioctlsocket(m_sock, FIONBIO, &mode) < 0)
		//return false;


	if (m_hostAddr.empty())
		return false;

	struct sockaddr_in destaddr;
	destaddr.sin_family = AF_INET;
	destaddr.sin_port = htons(m_port);
	destaddr.sin_addr.s_addr = inet_addr(m_hostAddr.c_str());

	int nRet = connect(m_sock, (struct sockaddr*)&destaddr, sizeof(destaddr));
	if (nRet == 0)    //如果立即连接成功
		return true;
	//虽直接返回,但未立即成功,用select等待看socket是否可写来判断connect是否成功
	if (WSAGetLastError() != WSAEWOULDBLOCK)
		return false;
	int retryCount = 0;
	while(1)
	{
		fd_set writeSet, exceptSet;
		FD_ZERO(&writeSet);
		FD_SET(m_sock, &writeSet);
		exceptSet = writeSet;

		struct timeval timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		int err = select((int)m_sock+1, NULL, &writeSet, &exceptSet, &timeout);
		if (err < 0)    //出错
			break;
		if (err == 0)    //超时
		{
			if (++retryCount > 10)    //重试10次
				return false;
			continue;
		}
		if (FD_ISSET(m_sock, &writeSet))
			return true;
		if (FD_ISSET(m_sock, &exceptSet))
			break;
	}
	return false;
}

bool HttpDownload::sendRequest() 
{
	if (m_getPath.empty())
		return false;

	char requestHeader[256];
	//格式化请求头
	int len = sprintf(requestHeader, 
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Range: bytes=%I64d-\r\n"    //从m_receivedDataSize位置开始
		"Connection: close\r\n"
		"\r\n",
		m_getPath.c_str(), m_hostAddr.c_str(), m_receivedDataSize);    

	int nSendBytes = 0;    //已发送字节数
	while(1)
	{
		fd_set writeSet;
		FD_ZERO(&writeSet);
		FD_SET(m_sock, &writeSet);

		struct timeval timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		int err = select((int)m_sock+1, NULL, &writeSet, NULL, &timeout);
		if (err < 0)
			break;
		if (err == 0)
			continue;
		int nBytes = send(m_sock, requestHeader+nSendBytes, len, 0);
		if (nBytes < 0)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				break;
			nBytes = 0;
		}
		nSendBytes += nBytes;    //若一次未发完,累计,循环send
		len -= nBytes;
		if (len == 0)
			return true;
	}
	return false;
}

bool HttpDownload::receiveData()
{
	char responseHeader[BUFFER_SIZE] = {0};

	struct timeval timeout;
	timeout.tv_sec = 3;    
	timeout.tv_usec = 0;

	//接收响应头
	int retryCount = 0;
	int nRecvBytes = 0;    //已接收字节数
	while (1)    
	{
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(m_sock, &readSet);
		int nRet = select(m_sock+1, &readSet, NULL, NULL, &timeout);
		if (nRet < 0)    //出错
			return false;
		if (nRet == 0)    //超时
		{
			if (++retryCount > 10)
				return false;
			continue;
		}
		retryCount = 0;
		if (recv(m_sock, responseHeader+nRecvBytes, 1, 0) <= 0)
			return false;
		nRecvBytes++;
		if (nRecvBytes >= BUFFER_SIZE)
			return false;
		if (nRecvBytes >= 4 &&
			responseHeader[nRecvBytes-4]=='\r' && responseHeader[nRecvBytes-3]=='\n' &&
			responseHeader[nRecvBytes-2]=='\r' && responseHeader[nRecvBytes-1]=='\n')
			break;
	}
	responseHeader[nRecvBytes] = '\0';

	if (strncmp(responseHeader, "HTTP/", 5))
		return false;
	int status = 0; 
	float version = 0.0;
	ULONGLONG startPos, endPos, totalLength;
	startPos = endPos = totalLength = 0;
	if (sscanf(responseHeader, "HTTP/%f %d ", &version, &status) != 2)
		return false;
	char* findStr = strstr(responseHeader, "Content-Range: bytes ");
	if (findStr == NULL)
	  	return false;
	if (sscanf(findStr, "Content-Range: bytes %I64d-%I64d/%I64d", 
		&startPos, &endPos, &totalLength) != 3)
		return false;
	if (status != 200 && status != 206 || totalLength == 0)
		return false;
	if (m_fileTotalSize == 1)    //第一次获取HTTP响应头,保存目标文件总大小
		m_fileTotalSize = totalLength;
	if (m_receivedDataSize != startPos)
		return false;

	//接收目标文件数据
	retryCount = 0;
	while (1)
	{
		char buf[BUFFER_SIZE] = {0};
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(m_sock, &readSet);

		int nRet = select((int)m_sock+1, &readSet, NULL, NULL, &timeout);
		if (nRet < 0)
			break;
		if (nRet == 0) {
			if (++retryCount > 10)
				break;
			continue;
		}
		int length = recv(m_sock, buf, BUFFER_SIZE, 0);
		if(length < 0)    //出错
			return false;
		if (length == 0)    //socket被优雅关闭
			return true;
		size_t written = fwrite(buf, sizeof(char), length, m_fp);
		if(written < length)
			return false;
		m_receivedDataSize += length;
		if (m_receivedDataSize == m_fileTotalSize)    //文件接收完毕
		{
			return true;
		}
	}
	return false;
}

bool HttpDownload::closeTransfer()
{
	if (m_sock > 0) {
		if (closesocket(m_sock) < 0)
			return false;
		m_sock = 0;
	}
	else
		m_sock = 0;
	return true;
}

bool HttpDownload::start()
{
    //创建多层文件夹
    int pos = m_saveFileName.find_first_of('/');
    string filepath = m_saveFileName;
    while(pos!=-1)
    {
      string temp = filepath.substr(0,pos) ;
      if(access(filepath.c_str(),0)==-1)
      {
         mkdir(temp.c_str());
      }
      string calpos = filepath.substr(pos+1,filepath.length());
      int tmppos = calpos.find_first_of('/');

      if(tmppos!=-1)
        pos+=tmppos+1;
      else
        pos=-1;
    }

    m_fp = fopen(m_saveFileName.c_str(), "wb");    //创建文件
    if (m_fp == NULL)
            return false;
    bool errFlag = false;
    while(1)
    {
            if (!initSocket() || !sendRequest() || !receiveData())
            {
                    if (m_cancelFlag)
                    {
                            errFlag = true;
                            break;
                    }
                    if (!closeTransfer())
                    {
                            errFlag = true;
                            break;
                    }
                    Sleep(1000);
                    continue;
            }
            break;
    }
    if(m_fp != NULL)
    {
            fclose(m_fp);
            m_fp = NULL;
    }
    if (errFlag) 
            return false;
    return true;
}

void HttpDownload::cancel() 
{
	m_cancelFlag = true;
	closeTransfer();
}

void HttpDownload::getPos(ULONGLONG& totalSize, ULONGLONG& downloadSize) 
{
	totalSize = m_fileTotalSize;
	downloadSize = m_receivedDataSize;
}


#pragma package(smart_init)
