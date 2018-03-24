//---------------------------------------------------------------------------

#ifndef SendMesgToChildH
#define SendMesgToChildH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <WinSock2.h>
#include "pthread.h"
#include "Unit1.h"
#pragma comment(lib,"ws2_32.lib")
//---------------------------------------------------------------------------

class SendMessageToChild : public TThread
{            
private:
protected:
        void __fastcall Execute();
public:
        __fastcall SendMessageToChild(String _recvName,String _addr,bool CreateSuspended);
        __fastcall ~SendMessageToChild();
        TStringList *fileNames;
private:
        //本机socket
        SOCKET sHost;
        SOCKADDR_IN servAddr;  //服务器地址
        int retVal;
        //当前服务器
        String addr;

        int IniSocket();
        
private:
        String createMesg();
        String recvName;
        void retSucMesg();
        void sendFile();
public:
        //设置发送的文件名称列表
        void setFileNames(TStringList * _filenames);
};
//---------------------------------------------------------------------------
#endif
 