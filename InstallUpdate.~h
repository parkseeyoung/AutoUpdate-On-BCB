//---------------------------------------------------------------------------

#ifndef InstallUpdateH
#define InstallUpdateH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "UpdaterForm.h"
//---------------------------------------------------------------------------
class InstallUpdate : public TThread
{            
private:
protected:
    void __fastcall Execute();
    String runpath;
public:
    __fastcall InstallUpdate(bool CreateSuspended,String FilePath);
    void __fastcall InstallUpdate::CreateForm();
};
//---------------------------------------------------------------------------
#endif
