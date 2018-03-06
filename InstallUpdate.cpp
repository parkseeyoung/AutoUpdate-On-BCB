//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "InstallUpdate.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall InstallUpdate::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall InstallUpdate::InstallUpdate(bool CreateSuspended,String FilePath)
    : TThread(CreateSuspended)
{
    runpath = FilePath;
    FreeOnTerminate = true;
}
//---------------------------------------------------------------------------
void __fastcall InstallUpdate::Execute()
{
    Synchronize(CreateForm);
}
//---------------------------------------------------------------------------
void __fastcall InstallUpdate::CreateForm()
{
    TForm2 * updaterform = new TForm2(NULL,runpath);
    TCanvas::Unlock;
    updaterform->ShowModal();
}
