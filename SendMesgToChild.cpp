//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SendMesgToChild.h"
#pragma package(smart_init)
const int BUF_SIZE = 64;
const int MaxThreadCount = 5;
//---------------------------------------------------------------------------

//�����̷߳�����Ϣ��Ҫ������
void SendMessageToChild::retSucMesg()
{
        if(Null == Form1->successList)
        {
                Form1->successList = new TStringList();
        }
        //recvNameΪ���յķ�����������
        Form1->successList->Append(recvName);
}

__fastcall SendMessageToChild::SendMessageToChild(String _recvName,String _addr,bool CreateSuspended)
        : TThread(CreateSuspended),addr(_addr),recvName(_recvName)
{
        FreeOnTerminate = true;
        sHost;  //�������׽���
        if(IniSocket()==-1)
        {
                WSACleanup();
                Terminate();
        }
        //�����׽���
        sHost = socket(AF_INET , SOCK_STREAM ,IPPROTO_TCP);
        if(INVALID_SOCKET == sHost)
        {
                WSACleanup();
                Terminate();
        }
        //���÷�������ַ
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.S_un.S_addr = inet_addr(addr.c_str());
        servAddr.sin_port = htons((short)7801);
        int nServAddlen = sizeof(servAddr);
}
//---------------------------------------------------------------------------
__fastcall SendMessageToChild::~SendMessageToChild()
{
        closesocket(sHost);     //�ر��׽���
        WSACleanup();           //�ͷ��׽�����Դ
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
        //���ӷ�����
        retVal = ::connect(sHost,(LPSOCKADDR)&servAddr,sizeof(servAddr));
        if(SOCKET_ERROR == retVal)
        {
                //����ʧ��
                closesocket(sHost);     //�ر��׽���
                WSACleanup();
                Terminate();
        }

        //send
        //����xml�Լ������ļ���������
        const int filesLen = fileStr.Length();
        retVal = send(sHost,fileStr.c_str(),filesLen+1,0);
        if(SOCKET_ERROR == retVal)
        {
                //д����־
                closesocket(sHost);
                WSACleanup();
                Terminate();
        }

        char bufRecv[BUF_SIZE];//�������ݻ�����
        //recv
        memset(bufRecv,0,BUF_SIZE * sizeof(char));
        recv(sHost, bufRecv , BUF_SIZE , 0 );   // ����child���ݣ�ֻ����5���ַ�
        if(strcmp(bufRecv,"ok")==0)
        {
                //�ɹ�,֪ͨ���У��ĳɿͻ��˲���http���ص�ģʽ
        }
        else
        {
                //ʧ��
        }

        //֪ͨnotify
        threadCount--;
        pthread_cond_signal(&cond);
}
//---------------------------------------------------------------------------
int SendMessageToChild::IniSocket()
{
        WSADATA wsd;
        //��ʼ���׽��ֶ�̬��
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
//�����ļ����¼�������
void SendMessageToChild::sendFile()
{
}
