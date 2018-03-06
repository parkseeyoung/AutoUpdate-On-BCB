//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UpdaterForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "AdvGlassButton"
#pragma resource "*.dfm"
TForm2 *Form2;
//记录所需要运行的软件
String runfilepath;
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner,String runpath)
    : TForm(Owner)
{
  runfilepath = runpath;
  //根据文件路径找到软件名称
  int temppos = runpath.LastDelimiter('\\');
  String temp = runpath.SubString(0,temppos-1);
  temppos = temp.LastDelimiter("\\");
  temp =  temp.SubString(0,temppos-1);
  temppos = temp.LastDelimiter("\\");
  temp = temp.SubString(temppos+1,temp.Length());
  //
  runpath = ExtractFileName(runpath);
  int pos = runpath.LastDelimiter('.');
  runpath = runpath.SubString(0,pos-1);
  this->Caption = runpath;
  this->lbl1->Caption=temp+"软件需要更新";
}
//---------------------------------------------------------------------------
void __fastcall TForm2::ShowMesg(int index)
{
  switch(index)
  {
    case 0:
        ShowMessage("找不到更新程序，请检查更新文件是否删除或移动");
  }
}



void __fastcall TForm2::FormCreate(TObject *Sender)
{
    HWND   m_hWnd=GetSystemMenu(this-> Handle,false);
    EnableMenuItem(m_hWnd,SC_CLOSE,MF_GRAYED);
      
}
//---------------------------------------------------------------------------




void __fastcall TForm2::AdvGlassButton1Click(TObject *Sender)
{
    if(runfilepath!="")
    {
        WinExec(runfilepath.c_str(),SW_SHOWNORMAL);
        //删除稍后更新
        String path = ExtractFilePath(Application->ExeName);
        path += "InstallLater.ini";
        FILE* temp = fopen(path.c_str(),"w");
        fclose(temp);
    }
    else
        ShowMesg(0);
    Close();
}
//---------------------------------------------------------------------------

void __fastcall TForm2::AdvGlassButton2Click(TObject *Sender)
{
    String path = ExtractFilePath(Application->ExeName);
    path += "InstallLater.ini";
    TIniFile * ini = new TIniFile(path);
    ini->WriteString("LaterUpdate",IntToStr(++Form1->softcount),runfilepath);
    ini->Free();
    Close();    
}
//---------------------------------------------------------------------------
