//---------------------------------------------------------------------------

#ifndef HttpDownloadH
#define HttpDownloadH
#include <cstdio>
#include <stdio.h>
#include <string>
#include <winsock2.h>
#include <io.h>
#include <direct.h>
using std::string;

class HttpDownload {
public:
	HttpDownload(const char* hostAddr, const int port,
		const char* getPath, const char* saveFileName);
	~HttpDownload();
	bool start();
	void cancel();
	void getPos(ULONGLONG& totalSize, ULONGLONG& downloadSize);
protected:
	bool initSocket();                        //初始化Socket
	bool sendRequest();                        //发送请求头
	bool receiveData();                        //接收数据
	bool closeTransfer();                    //关闭传输
private:
	std::string m_hostAddr;                    //目标主机IP
	int m_port;                                //HTTP端口号
	std::string m_getPath;                    //目标文件相对路径
	std::string m_saveFileName;                //保存文件路径
	SOCKET m_sock;                            //Socket
	FILE* m_fp;                                //保存文件指针
	ULONGLONG m_fileTotalSize;                //目标文件总大小
	ULONGLONG m_receivedDataSize;            //已接收数据大小
	bool m_cancelFlag;                        //取消下载标记
};
//---------------------------------------------------------------------------
#endif
