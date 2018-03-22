//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include <cstdio>
#include "InitWinSocket.h"
#include "DownloadTask.cpp"
#include "SendMesgToChild.h"
#include <pthread.h>
#include <vector>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "trayicon"
#pragma resource "*.dfm"

pthread_mutex_t mutex;
pthread_cond_t cond;
int threadCount = 0;

const char* g_progName = NULL;
const char* g_saveFileName = "savefilename";
InitWinSocket init;
TForm1 *Form1;
//��¼���ص��������Ҫ���µ�run����
TStringList * runs = new TStringList();
MyLog log("log.txt");

//ȫ�ּ�¼��ǰ���ӣ�һ��������һ���߳�
//---------------------------------------------------------------------------
String getHostIP()
{
        char Buffer[64];
        HOSTENT FAR *pHost;
        AnsiString sIPAddress;
        gethostname(Buffer,64);
        pHost = gethostbyname(Buffer);
        sIPAddress = inet_ntoa(*(struct in_addr*)pHost->h_addr_list[0]);
        return sIPAddress;
}
//��ʼ���̳߳�
void iniThreadPool()
{
        pthread_mutex_init(&mutex,NULL);
        pthread_cond_init(&cond,NULL);
}
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
        this->IdTCPServer1->Bindings->Add();
        this->IdTCPServer1->Bindings->Items[0]->Port = 19999;
        this->IdTCPServer1->Bindings->Items[0]->IP = getHostIP();
        this->IdTCPServer1->Active = true;
        String mesg ("�����������ɹ���");
        recvList = new TStringList();
        fileOpList = new TStringList();
        iniThreadPool();
}

//��������
bool TForm1::AutoRun(bool value)
{
    int ret = 0;
    TRegistry * Reg = new TRegistry;
    AnsiString keyval="\""+ ParamStr(0)+ "\"";
    AnsiString key1="Software\\Microsoft\\Windows\\CurrentVersion\\Run\\";
    Reg->RootKey=HKEY_CURRENT_USER;
    if( !Reg->OpenKey(key1,false))
    {
    //Show msg
    }else
    {
         if(value)
         {
               AnsiString curvalstr = Reg->ReadString("MYAPP");
              if(curvalstr!=keyval)
                         Reg->WriteString("MYAPP",keyval);
               Reg->CloseKey();
               ret = 1;
         }else
         {
              if(Reg->DeleteValue("MYAPP"))
                   ret = 1;
         }
    }
    delete Reg;
    return ret;
}

//---------------------------------------------------------------------------

//����Ini�ļ�
bool TForm1::LoadIni(String filename,String* _port,String* _ip,String* _dwname)
{
  try
  {
    TIniFile * ini = new TIniFile(filename.c_str());
    AnsiString ip = ini->ReadString("Connect","ip","");
    AnsiString port = ini->ReadString("Connect","port","");
    AnsiString dwname = System::Utf8ToAnsi(ini->ReadString("Connect","dw",""));


    *_ip=ip;
    *_port=port;
    *_dwname=dwname;

    delete ini;
  }
  catch(...)
  {
    return false;
  }
  return true;
}
//����ע���
void TForm1::GetRegInfo(TStringList*list)
{
    TRegistry& regkey = * new TRegistry();
    bool key = regkey.OpenKey("Software\\HL Software\\updatesoft",false);
    if(key)
    {
      regkey.GetValueNames(list) ;
    }
}
//GB2312��UTF-8��ת��
char* G2U(const char* gb2312)
{
  int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
  wchar_t* wstr = new wchar_t[len+1];
  memset(wstr, 0, len+1);
  MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
  len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  char* str = new char[len+1];
  memset(str, 0, len+1);
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
  if(wstr) delete[] wstr;
  return str;
}
//UTF8ת�ٷ��Ʊ���
char* UrlEncode(const char* pURL)
{  
    // ��ת����UTF-8
    char* pUTF8 = G2U(pURL);
    int nUTF8Len = strlen(pUTF8);
    if(0 == nUTF8Len)  
    {  
        return 0;  
    }  
    char* pEncode = new char[3 * nUTF8Len + 1];  
    memset(pEncode, 0, 3 * nUTF8Len + 1);  

    int i = 0;  
    int nEncodeIndex = 0;
    unsigned char cTemp;  
    for(i = 0; i < nUTF8Len; ++i)
    {  
        cTemp = pUTF8[i];
        if(cTemp==0x3A||cTemp==0x2E||cTemp==0x2F)
        {
            pEncode[nEncodeIndex] = cTemp;
            ++nEncodeIndex;
            continue;
        }
        if(::isalpha(cTemp) || ::isdigit(cTemp) || '-' == cTemp ||
            '.' == cTemp || '~' == cTemp || '_' == cTemp)  
        {
            pEncode[nEncodeIndex] = cTemp;
            ++nEncodeIndex;
        }
        else if(' ' == cTemp)  
        {  
            pEncode[nEncodeIndex] = '+';  
            ++nEncodeIndex;  
        }  
        else  
        {  
            pEncode[nEncodeIndex] = '%';  
            pEncode[nEncodeIndex + 1] = (0xA0 <= cTemp) ? ((cTemp >> 4) - 0x0A + 'A') : ((cTemp >> 4) + '0');  
            pEncode[nEncodeIndex + 2] = (0x0A <= (0x0F & cTemp)) ? ((0x0F & cTemp)  - 0x0A + 'A') : ((0x0F & cTemp) + '0');  
            nEncodeIndex += 3;
        }
    }  
    delete [] pUTF8;
    return pEncode;
}

//������
void TForm1::progressBar(float percent) {

}
//
int TForm1::DownLoadFile(const char*url,const char*downloadname)
{
    if(strncmp(url,"http://",strlen("http://"))!=0)
    {
        return -1;
    }
    //const char* temp = url+7;

   //
    g_saveFileName = downloadname;
    char hostAddr[256] = {0};
    int port = 244;
    char getPath[256] = {0};


    parseURL(url,hostAddr,port,getPath);
    url= UrlEncode(url);

    HttpDownload obj(hostAddr,port ,getPath,g_saveFileName);
    pthread_t tid;
    int err = pthread_create(&tid,NULL,task,&obj);
    if (err != 0)
        //д������־
		log.WriteLog("��������ʧ��");

	ULONGLONG totalSize = 1;
	ULONGLONG downloadSize = 0;
	float percent = 0;
	while (1) {
		obj.getPos(totalSize, downloadSize);
		percent = downloadSize/(float)totalSize;
		progressBar(percent);
		if (downloadSize == totalSize)
			break;
		Sleep(500);
	}

	void* ret = NULL;
	pthread_join(tid, &ret);
	if (ret)
        //д�ɹ���־
        {
            String mesg = downloadname;
            mesg += "���سɹ�";
            log.WriteLog(mesg.c_str());
        }
	else
        //д������־
        {
            String mesg = downloadname;
            mesg += "����ʧ��";
                    log.WriteLog(mesg.c_str());
        }
	return 0;
}


void __fastcall TForm1::Button1Click(TObject *Sender)
{
   softcount = 0;
   Timer1->Enabled = false;
   //��ʾ�Ժ����
   String laterinipath = ExtractFilePath(Application->ExeName)+"InstallLater.ini";
   TIniFile * LaterIni = new TIniFile(laterinipath);
   int sfcount=1;
   if(LaterIni->ReadString("LaterUpdate",IntToStr(sfcount),"")!="")
   {
     try
     {
       for(;LaterIni->ReadString("LaterUpdate",IntToStr(sfcount),"")!="";sfcount++,Sleep(5000))
       {
          //���Ժ���£���ȡ�����ļ���ֱ�Ӵ����߳�
         String softpath = LaterIni->ReadString("LaterUpdate",IntToStr(sfcount),"");
         runs->Add(softpath);
         Timer2->Enabled = true;
       }
     }
     __finally
     {
       delete LaterIni;
       return;
     }
   }
   delete LaterIni;
   //�ȶ�ȡ�������ϵ������ļ�
   String inifile = ExtractFilePath(Application->ExeName)+"Config.ini";
   String ip,port,dwname;
   LoadIni(inifile,&port,&ip,&dwname);

   //String url = "http://"+ip+":"+port+"/downfile/" ;
   TStringList * list = new TStringList();
   GetRegInfo(list);
   if(list)
   {
     int listcount = list->Count;
     for(int i = 0;i<listcount; i++)
     {
        String url = "http://"+ip+":"+port+"/downfile/autoupdate/" ;
        String swname = list[0][i];
        url+=swname+"/"+dwname+"/UpdaterList.xml";
        String savepath = "downloadfile/";
        savepath+=swname;
        savepath+="/temp/UpdaterList.xml";
        DownLoadFile(url.c_str(),savepath.c_str());
        String xmlurl = url;
        //����temp�е�xml�ļ�
        XMLDocument1->Active = false;
        String path = ExtractFilePath(Application->ExeName)+ savepath;
        XMLDocument1->LoadFromFile(path);
        XMLDocument1->Active = true;
         //TStringList* UpProg = new TStringList();
         _di_IXMLNode node = XMLDocument1->DocumentElement;
         _di_IXMLNodeList nodelist = node->ChildNodes;
         _di_IXMLNode _ver = nodelist->FindNode("Application")->GetChildNodes()->FindNode("Ver");
         String ver = _ver->Text;
         //run
         _di_IXMLNode _run = nodelist->FindNode("Application")->GetChildNodes()->FindNode("Run");
         String run = _run->Text;
         //
         _di_IXMLNode _url = nodelist->FindNode("Updater")->GetChildNodes()->FindNode("Url");
         url = _url->Text;
         url+="/"+ver;
         _di_IXMLNode __files =  nodelist->FindNode("Files");
         int filescount = __files->GetChildNodes()->GetCount();

         //����֮ǰ��xml�ļ�
         XMLDocument1->Active = false;
         String oldpath = "downloadfile/"+swname+"/UpdaterList.xml";
            //����ļ�����
         if(FileExists(oldpath))
         {
           XMLDocument1->LoadFromFile(oldpath);
           XMLDocument1->Active = true;
          // TStringList* UpProg = new TStringList();
           _di_IXMLNode node = XMLDocument1->DocumentElement;
           _di_IXMLNodeList nodelist = node->ChildNodes;
           _di_IXMLNode old_ver = nodelist->FindNode("Application")->GetChildNodes()->FindNode("Ver");
           String oldver = old_ver->Text;
           if(oldver==ver)
           {
              continue;
           }
           else
           DeleteFileA(oldpath);
         }
         for(int i =0 ;i<filescount;i++)
         {
            String filename= __files->GetChildNodes()->Nodes[i]->GetAttribute("name");
            String downurl = url+"/"+filename;
            String softsavepath = "downloadfile/"+swname+"/"+ver+"/"+filename;
            DownLoadFile(downurl.c_str(),softsavepath.c_str());
         }
         String verxml = "downloadfile/"+swname+"/"+ver+"/UpdaterList.xml";
         DownLoadFile(xmlurl.c_str(),verxml.c_str());
         //ɾ��temp���滻��oldxml
         CopyFile(savepath.c_str(),oldpath.c_str(),false);
         DeleteFileA(savepath);
         //дһ�������ļ���Ϊ��¼
         String loginipath = "downloadfile/"+swname+"/DownloadLog.ini";
         if(!FileExists(loginipath))
         WritePrivateProfileStringA("log","version",ver.c_str(),loginipath.c_str());
         else
         {
           TIniFile * ini = new TIniFile(loginipath.c_str());
           ini->WriteString("log","version",ver);
         }
         String runpath =ExtractFilePath(Application->ExeName)+"downloadfile\\"+swname+"\\"+ver+"\\"+run;    //
         //��Ϊ����ʾ��װ���棬ֱ�ӽ������˵���Ϣ��¼��runs��
         runs->Add(runpath);
     }
   }
   delete list;
   Timer2->Enabled = true ;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
    Button1->Click();
    Timer1->Interval = 7200000;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer2Timer(TObject *Sender)
{
    Timer2->Enabled = false;
    int count = runs->Count;
    if(count!=0)
    {

        InstallUpdate * install = new InstallUpdate(true,runs[0][--count]);
        install->Resume();
        runs->Delete(count);
        Timer2->Enabled = true;
    }
    else
    {
        Timer1->Enabled=true ;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender)
{
    DWORD dwExStyle=GetWindowLong(Application->Handle,GWL_EXSTYLE); //�����������к�������
    dwExStyle |=WS_EX_TOOLWINDOW;                                 //�޷���������
    SetWindowLong(Application->Handle,GWL_EXSTYLE,dwExStyle);     //Alt+TabҲ��������ֻ�������

    String path = "log.txt";
    FILE* temp = fopen(path.c_str(),"w");
    fclose(temp);
    AutoRun(true);
}
//---------------------------------------------------------------------------


void __fastcall TForm1::IdTCPServer1Connect(TIdPeerThread *AThread)
{
        String name=AThread->Connection->LocalName;
        AThread->Start();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::IdTCPServer1Execute(TIdPeerThread *AThread)
{
        if(!AThread->Connection->Connected())
                return;
        String mesg = AThread->Connection->CurrentReadBuffer();
        if(mesg=="updated")
        {
                String respond("ok");
                AThread->Connection->Write(respond);
                this->Button1->Click();
        }
        //�����ļ����б�
        else
        {
                gsmt = new GetServMesgThread(true);
                if(gsmt->getFileNames(mesg) == 0)
                {
                        String respond("ok");
                        AThread->Connection->Write(respond);
                        gsmt->Resume();
                }
        }
        AThread->Terminate();
}
//---------------------------------------------------------------------------
//���յ����е��ļ�
void __fastcall TForm1::Button2Click(TObject *Sender)
{
        //ֱ��ʹ�ÿؼ����ϴ����������ı���
        String sql = "";
        bdb = new CBaseDb();
        bool conn = bdb->connectToOracle("manager","zbxhzbxh","192.168.10.236:1521:zbxh");

        //��ȡ����dwname
        TOraQuery* query = bdb->dbQy1;
        String p_openSql = "select ����,����,�ϼ����� from BD_AU_��λ�ֵ�� where ip='"+getHostIP()+"'";
        bdb->OpenSql(query, p_openSql);
        m_dwname = query->FieldByName("����")->AsString;
        m_code = query->FieldByName("����")->AsString;
        m_parentCode = query->FieldByName("�ϼ�����")->AsString;
        if(conn == false)
        {
                //д��־,���������ݷ�����
                return;
        }
        //�ȴ���xml
        dealWithUploadXml();
        //������¼�¼����Ҫ�ϱ���SoftUploadRecord

        TStringList *tableList = new TStringList();
        bdb->GetOracleTableNameList(tableList);
        String tableName("AU_UploadRecord");
        //û��Ļ��Ƚ���
        if(tableList->IndexOf(tableName)==-1)
        {
                //����Ҳд���������ˣ�����Ҳ��ֻ��һ��
                TStringList* tableFieldName = new TStringList();
                tableFieldName->Append("PGUID");
                tableFieldName->Append("ISDELETE");
                tableFieldName->Append("S_UDTIME");
                tableFieldName->Append("SYNC_TIME");
                tableFieldName->Append("����������");
                tableFieldName->Append("IP��ַ");
                tableFieldName->Append("����汾");
                tableFieldName->Append("���³ɹ�");
                tableFieldName->Append("�������");
                tableFieldName->Append("����");
                tableFieldName->Append("�ϼ�����");
                tableFieldName->Append("��λ����");

                TStringList* FieldType = new TStringList();
                FieldType->Append("VARCHAR2");
                FieldType->Append("NUMBER");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");

                TStringList* FieldLength = new TStringList();
                FieldLength->Append("255");
                FieldLength->Append("38");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");

                bdb->CreateOracleTableStructInfo(tableName,tableFieldName,FieldType,FieldLength);
        }
        //����б����������(�ļ��б�)
        if(fileList)
        {
                for(int index = 0 ; index < fileList->Count ; index ++)
                {
                        GUID uid;
                        CoCreateGuid(&uid);
                        String pguid = Sysutils::GUIDToString(uid) ;
                        String isdelete = "0";
                        String sudtime = Now();
                        String synctime = "";
                        //������Ҫ����xml�ļ�
                        String servName = "fwq";
                        String ip = getHostIP();
                        String ver = "";
                        String success = "��";
                        String softname = m_softname;
                        //����
                        m_code;
                        //�ϼ�����
                        m_parentCode;
                        //��λ����
                        m_dwname;

                        String p_sql = "insert into "+tableName + " VALUES('"+pguid+"',"+
                        isdelete+",'"+sudtime+"','"+synctime+"','"+servName+"','"+
                        ip+"','"+ver+"','"+success+"','"+softname+"','"+m_code+"','"+m_parentCode+
                        "','"+m_dwname+"')";
                        if (bdb->ToExecOracleSql(p_sql) == false)
                        {
                                //��־��¼
                                break;
                        }
                }
        }

//���ǵڶ�����
        tableName = "AU_SoftUploadRecord";
        if(tableList->IndexOf(tableName)==-1)
        {
                //����Ҳд���������ˣ�����Ҳ��ֻ��һ��
                TStringList* tableFieldName = new TStringList();
                tableFieldName->Append("PGUID");
                tableFieldName->Append("ISDELETE");
                tableFieldName->Append("S_UDTIME");
                tableFieldName->Append("SYNC_TIME");
                tableFieldName->Append("����������");
                tableFieldName->Append("IP��ַ");
                tableFieldName->Append("����汾");
                tableFieldName->Append("���³ɹ�");
                tableFieldName->Append("�������");
                tableFieldName->Append("�����ļ�����");
                tableFieldName->Append("�����ļ�����");

                TStringList* FieldType = new TStringList();
                FieldType->Append("VARCHAR2");
                FieldType->Append("NUMBER");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");
                FieldType->Append("VARCHAR2");

                TStringList* FieldLength = new TStringList();
                FieldLength->Append("255");
                FieldLength->Append("38");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");
                FieldLength->Append("255");

                bdb->CreateOracleTableStructInfo(tableName,tableFieldName,FieldType,FieldLength);
        }
        //����б����������(�ļ��б�)
        //��AU_SoftUploadRecord
        int fileCount = fileList->Count;
        if(fileList)
        {
                for(int index = 0 ; index < fileCount ; index ++)
                {
                        GUID uid;
                        CoCreateGuid(&uid);
                        String pguid = Sysutils::GUIDToString(uid) ;
                        String isdelete = "0";
                        String sudtime = Now();
                        String synctime = "";
                        //������Ҫ����xml�ļ�
                        String servName = "fwq";
                        String ip = getHostIP();
                        String ver = "";
                        String success = "��";
                        String softname = m_softname;
                        String filename = fileList[0][index];
                        String fileop = fileOpList[0][index];
                        WideString ss(filename);
                        int lg=filename.Length();
                        AnsiString last = softname+"','"+filename + "','"+fileop+"')";
                        if(softname=="")
                                return;
                        String p_sql = "insert into "+tableName + " VALUES('"+pguid+"',"+
                        isdelete+",'"+sudtime+"','"+synctime+"','"+servName+"','"+
                        ip+"','"+ver+"','"+success+"','"+ last;
                        if (bdb->ToExecOracleSql(p_sql) == false)
                        {
                                //��־��¼
                                return;
                        }
                }
        }

//**********************************
/*
        �������ˣ���ʼ������Ϣ���¼�������
*/
        const int count_recv = recvList->Count;
        if( count_recv == 0 )
                return;
        for( int i = 0 ; i < count_recv ; i++)
        {
                String _recvname = recvList[0][i];
                String _addr = findIpByDwname(_recvname);
                sendMessageToChild(_recvname ,_addr);
        }

        delete fileList;
}
//---------------------------------------------------------------------------

//��SendMessageToChild������������ʵ���̳߳�
int TForm1::sendMessageToChild(String _recvname , String _addr)
{
        //Ҳ���ȷ�����Ϣ���ļ������ͻ��ˣ�Ȼ���ڽ��д����ļ�

        //׼������sendMesgToChild��дSendFile
        SendMessageToChild *stc = new SendMessageToChild(_recvname,_addr,true);
        stc->setFileNames(recvList);
        stc->Resume();
}

int TForm1::dealWithUploadXml()
{
        //Ū������ʣ��ļ����Ƿ��͵�һ���ģ�����ֻҪŪ����������﷢������
        //ͨ��Personinfo��
        String softname,desciption,ver;
        m_softname = "";
        //����xml�ļ�
        CoInitialize(NULL);
        this->XMLDocument1->Active = false;
        this->XMLDocument1->LoadFromFile("./uploadFile/UpdaterList.xml");
        this->XMLDocument1->Active = true;

        //�汾����Application��Ver����

        //��ս��յ�λ�б�
        recvList->Clear();

        //���op�б�
        fileOpList->Clear();

        _di_IXMLNode Node = this->XMLDocument1->DocumentElement;
        if(Node)
        {
                _di_IXMLNodeList nodeList = Node->GetChildNodes();
                int count = nodeList->GetCount();       //��¼����
                //ֱ���ҳ�softname
                _di_IXMLNode node_softname = nodeList->FindNode("SoftName");
                softname = node_softname->GetText();

                _di_IXMLNode node_desc = nodeList->FindNode("Desciption");
                desciption = node_desc->GetText();

                //�ҳ�Ŀ��
                _di_IXMLNode node_aims = nodeList->FindNode("Aims");
                _di_IXMLNodeList aimList = node_aims->GetChildNodes();
                int count_aim = aimList->Count;
                //ͨ����λ���Ƶݹ���ҳ���Ӧ��node
                _di_IXMLNode node = NULL;
                findNode(m_dwname,aimList,node);
                if(!node)
                        return -1;
                _di_IXMLNodeList theList = node->GetChildNodes();
                for( int i = 0 ;i < count_aim ; i++)
                {
                        _di_IXMLNode temp_node = theList->Nodes[i];
                        String temp_serv= temp_node->GetAttribute("server");
                        if(temp_serv.ToIntDef(-1)==0)
                                continue;
                        recvList->Append(temp_node->GetAttribute("DwName"));
                }
                //�ҳ�files�ڵ�
                _di_IXMLNode node_files = nodeList->FindNode("Files");
                _di_IXMLNodeList fileNodeList = node_files->GetChildNodes();
                int count_file = fileNodeList->GetCount();
                for( int i = 0 ;i<count_file;i++)
                {
                        _di_IXMLNode temp_node = fileNodeList->Nodes[i];
                        fileOpList->Append(temp_node->GetAttribute("options"));
                }
                //�ҳ�application�ڵ�
                _di_IXMLNode node_application = nodeList->FindNode("Application");
                _di_IXMLNodeList applicationNodeList = node_application->GetChildNodes();
                _di_IXMLNode temp_ver = applicationNodeList->FindNode("Ver");
                ver = temp_ver->Text;
        }

        m_softname = softname;
        //����ver�½�һ���ļ���,��ת���ļ�
        makeVerDir(ver);
}
void TForm1::makeVerDir(String ver)
{
        String verPath = ".\\downfile\\autoupdate\\"+m_softname+"\\"+m_dwname+"\\"+ver;
        CopyFolder(".\\uploadFile",verPath);
        //Ȼ���xml�ļ��ŵ����渴���滻���ڰ�uploadFileɾ��������
        String xmlFilePath = ".\\downfile\\autoupdate\\"+m_softname+"\\"+m_dwname+"\\UpdaterList.xml";
        CopyFile(".\\uploadFile\\UpdaterList.xml",xmlFilePath.c_str(),0);
        DeleteFolder(".\\uploadFile");
}
//�ݹ鴴���ļ���
void TForm1::CopyFolder(String pathFrom,String pathTo)
{
        TSearchRec sr;
        if(!DirectoryExists(pathFrom))
        {
                return;
        }
        if(!DirectoryExists(pathTo))
        {
                ForceDirectories(pathTo.c_str());
        }
        if (FindFirst(pathFrom + "//*.*", faAnyFile, sr) == 0)
        {  
                do{  
                        try{  
                                if ((sr.Attr & faDirectory) != 0){  
                                      //folder  
                                      if (sr.Name != "." && sr.Name != ".."){  
                                        CopyFolder(pathFrom+"//"+sr.Name, pathTo+"//"+sr.Name);
                                      }
                                }else{  
                                    //file  
                                    CopyFile((pathFrom + "//" + sr.Name).c_str(),
                                             (pathTo + "//" + sr.Name).c_str(),
                                              0);  
                                }  
                        }catch(...){}  
                } while (FindNext(sr) == 0);  
                FindClose(sr);  
        }
}
//ɾ���ļ�
void TForm1::DeleteFolder(String srcPath)
{
        TSearchRec sr;
        if (!DirectoryExists(srcPath)){
            return ;
        }

        if (FindFirst(srcPath + "//*.*", faAnyFile, sr) == 0){
                do{
                        try{
                                if ((sr.Attr & faDirectory) != 0){
                                      //folder
                                      if (sr.Name != "." && sr.Name != ".."){
                                        DeleteFolder(srcPath+"//"+sr.Name);
                                      }
                                }else{
                                    //file
                                    DeleteFileA((srcPath + "//" + sr.Name).c_str());
                                }
                        }catch(...){}
                } while (FindNext(sr) == 0);
                FindClose(sr);
        }
}
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
        if(NULL!=successList)
                delete successList;
        if(NULL!=recvList)
                delete recvList;
        if(NULL!=fileOpList)
                delete fileOpList;
}
//---------------------------------------------------------------------------
void TForm1::findNode(String nodeName,_di_IXMLNodeList nodeList,_di_IXMLNode &node)
{
        int listCount = nodeList->Count;
        for(int i = 0; i < listCount ;i++)
        {
                _di_IXMLNode tempNode_aim = nodeList->Nodes[i];
                String aim = tempNode_aim->GetAttribute("DwName");

                if(aim == m_dwname)
                {
                        node = tempNode_aim;
                }
                else if(tempNode_aim->ChildNodes)
                {
                        findNode(nodeName,tempNode_aim->ChildNodes,node);
                }
        }
}
String TForm1::findIpByDwname(String _dwname)
{
        if(!bdb->IsOracleConnected())
        {
          return "ERROR";
        }
        String sql = "select IP from BD_AU_��λ�ֵ�� where ���� = '"+_dwname +"'";
                //��ȡ����dwname
        TOraQuery* query = bdb->dbQy2;
        bdb->OpenSql(query,sql);
        String ip = query->FieldByName("IP")->AsString;
        return ip;
}
