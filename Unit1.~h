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
//建立一个简单的线程池来管理SendMesgToChild线程
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
//给线程用的记录需要更新软件的个数的成员
    int softcount;
    void progressBar(float percent);
    __fastcall TForm1(TComponent* Owner);
    int TForm1::DownLoadFile(const char*url,const char*downloadname);
    bool TForm1::AutoRun(bool value);
    bool TForm1::LoadIni(String filename,String* _port,String* _ip,String* _dwname);
    void TForm1::GetRegInfo(TStringList*list);

    //文件名列表
    TStringList *fileList;
    //处理xml文件从而得出到底是什么软件
    int dealWithUploadXml();
    //给xml文件中的其他服务器发送消息
    int sendMessageToChild(String _recvname , String _addr);

    //递归找node
    void findNode(String nodeName,_di_IXMLNodeList nodeList,_di_IXMLNode& node);

    //在数据库中找到单位名称所对应的ip地址
    String findIpByDwname(String _dwname);

 //-----------------------------

    //发送消息成功表
    TStringList* successList;

    //需要发送的所有文件列表
    TStringList* recvList;
    //本机dwname
    String m_dwname;
    //当前软件名称
    String m_softname;
    //软件操作列表
    TStringList *fileOpList;
    //单位编码和上级编码
    String m_code,m_parentCode;

    CBaseDb* bdb;
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
