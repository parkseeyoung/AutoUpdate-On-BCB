//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit2.h"
#include "Unit1.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
//给主线程返回消息，要求上锁
void SendMessageToChild::retSucMesg()
{
        if(Null == Form1->successList)
        {
                Form1->successList = new TStringList();
        }
        Form1->successList->Append(recvName);
}

__fastcall SendMessageToChild::SendMessageToChild(String _recvName,String _addr,bool CreateSuspended)
        : TThread(CreateSuspended),addr(_addr),recvName(_recvName)
{
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
        servAddr.sin_port = htons((short)19999);
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
        String files = createMesg();
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

