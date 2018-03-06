//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <msxmldom.hpp>
#include <XMLDoc.hpp>
#include <xmldom.hpp>
#include <XMLIntf.hpp>
#include <registry.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include "InstallUpdate.h"
#include "trayicon.h"
#include "GetServMesgThread.h"
#include "SendMesgToChild.h"
#include "BaseDb.h"
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdTCPServer.hpp>
#include <pthread.h>
#define MAXSENDTHREADCOUNT 6

//---------------------------------------------------------------------------
//����һ���򵥵��̳߳�������SendMesgToChild�߳�
extern pthread_mutex_t mutex;
extern pthread_cond_t cond;
extern int threadCount;

class TForm1 : public TForm
{
__published:	// IDE-managed Components
    TButton *Button1;
    TXMLDocument *XMLDocument1;
    TTimer *Timer1;
    TTimer *Timer2;
        TIdTCPServer *IdTCPServer1;
        TButton *Button2;
    void __fastcall Button1Click(TObject *Sender);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall Timer2Timer(TObject *Sender);
    void __fastcall FormCreate(TObject *Sender);
        void __fastcall IdTCPServer1Connect(TIdPeerThread *AThread);
        void __fastcall IdTCPServer1Execute(TIdPeerThread *AThread);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations

    GetServMesgThread *gsmt;
public:		// User declarations
//���߳��õļ�¼��Ҫ��������ĸ����ĳ�Ա
    int softcount;
    void progressBar(float percent);
    __fastcall TForm1(TComponent* Owner);
    int TForm1::DownLoadFile(const char*url,const char*downloadname);
    bool TForm1::AutoRun(bool value);
    bool TForm1::LoadIni(String filename,String* _port,String* _ip,String* _dwname);
    void TForm1::GetRegInfo(TStringList*list);

    //�ļ����б�
    TStringList *fileList;
    //����xml�ļ��Ӷ��ó�������ʲô���
    int dealWithUploadXml();
    //��xml�ļ��е�����������������Ϣ
    int sendMessageToChild(String _recvname , String _addr);

    //�ݹ���node
    void findNode(String nodeName,_di_IXMLNodeList nodeList,_di_IXMLNode& node);

    //�����ݿ����ҵ���λ��������Ӧ��ip��ַ
    String findIpByDwname(String _dwname);

 //-----------------------------

    //������Ϣ�ɹ���
    TStringList* successList;

    //��Ҫ���͵������ļ��б�
    TStringList* recvList;
    //����dwname
    String m_dwname;
    //��ǰ�������
    String m_softname;
    //��������б�
    TStringList *fileOpList;
    //��λ������ϼ�����
    String m_code,m_parentCode;

    CBaseDb* bdb;
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
