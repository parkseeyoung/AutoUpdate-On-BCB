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
        //����socket
        SOCKET sHost;
        SOCKADDR_IN servAddr;  //��������ַ
        int retVal;
        //��ǰ������
        String addr;

        int IniSocket();
        
private:
        String createMesg();
        String recvName;
        void retSucMesg();
        void sendFile();
public:
        //���÷��͵��ļ������б�
        void setFileNames(TStringList * _filenames);
};
//---------------------------------------------------------------------------
#endif
 