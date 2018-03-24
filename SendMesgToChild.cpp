//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SendMesgToChild.h"
#pragma package(smart_init)
const int BUF_SIZE = 64;
const int MaxThreadCount = 5;
//---------------------------------------------------------------------------

//给主线程返回消息，要求上锁
void SendMessageToChild::retSucMesg()
{
        if(Null == Form1->successList)
        {
                Form1->successList = new TStringList();
        }
        //recvName为接收的服务器的名字
        Form1->successList->Append(recvName);
}

__fastcall SendMessageToChild::SendMessageToChild(String _recvName,String _addr,bool CreateSuspended)
        : TThread(CreateSuspended),addr(_addr),recvName(_recvName)
{
        FreeOnTerminate = true;
        sHost;  //服务器套接字
        if(IniSocket()==-1)
        {
                WSACleanup();
                Terminate();
        }
        //创建套接字
        sHost = socket(AF_INET , SOCK_STREAM ,IPPROTO_TCP);
        if(INVALID_SOCKET == sHost)
        {
                WSACleanup();
                Terminate();
        }
        //设置服务器地址
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.S_un.S_addr = inet_addr(addr.c_str());
        servAddr.sin_port = htons((short)7801);
        int nServAddlen = sizeof(servAddr);
}
//---------------------------------------------------------------------------
__fastcall SendMessageToChild::~SendMessageToChild()
{
        closesocket(sHost);     //关闭套接字
        WSACleanup();           //释放套接字资源
        delete fileNames;
}

void __fastcall SendMessageToChild::Execute()
{
        if(threadCount > MaxThreadCount)
        {
          pthread_cond_wait(&cond,&mutex);
        }
        else
        {
          threadCount ++;
        }
        if(fileNames == NULL)
                return;
        String fileStr = "updated";
        //连接服务器
        retVal = ::connect(sHost,(LPSOCKADDR)&servAddr,sizeof(servAddr));
        if(SOCKET_ERROR == retVal)
        {
                //连接失败
                closesocket(sHost);     //关闭套接字
                WSACleanup();
                Terminate();
        }

        //send
        //发送xml以及更新文件给服务器
        const int filesLen = fileStr.Length();
        retVal = send(sHost,fileStr.c_str(),filesLen+1,0);
        if(SOCKET_ERROR == retVal)
        {
                //写入日志
                closesocket(sHost);
                WSACleanup();
                Terminate();
        }

        char bufRecv[BUF_SIZE];//接收数据缓冲区
        //recv
        memset(bufRecv,0,BUF_SIZE * sizeof(char));
        recv(sHost, bufRecv , BUF_SIZE , 0 );   // 接收child数据，只接收5个字符
        if(strcmp(bufRecv,"ok")==0)
        {
                //成功,通知就行，改成客户端采用http下载的模式
        }
        else
        {
                //失败
        }

        //通知notify
        threadCount--;
        pthread_cond_signal(&cond);
}
//---------------------------------------------------------------------------
int SendMessageToChild::IniSocket()
{
        WSADATA wsd;
        //初始化套接字动态库
        if(WSAStartup(MAKEWORD(2,2),&wsd)!=0)
        {
                return -1;
        }
}
String SendMessageToChild::createMesg()
{
        String temp;
        temp = "FileNameList:";
        for(int i = 0 ; i < fileNames->Count ; i++)
        {
                temp += fileNames[0][i];
                if( i!=fileNames->Count-1)
                {
                        temp+="?";
                }
        }
        return temp;
}
void SendMessageToChild::setFileNames(TStringList * _filenames)
{
        fileNames = _filenames;
}
//发送文件给下级服务器
void SendMessageToChild::sendFile()
{
}
