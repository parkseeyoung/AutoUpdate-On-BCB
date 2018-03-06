//---------------------------------------------------------------------------

#ifndef UpdaterFormH
#define UpdaterFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "AdvGlowButton.hpp"
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <IniFiles.hpp>
#include "Unit1.h"
#include "MyLog.h"
#include <jpeg.hpp>
#include "AdvGlassButton.hpp"
//---------------------------------------------------------------------------
class TForm2 : public TForm
{
__published:	// IDE-managed Components
    TLabel *lbl1;
    TLabel *lbl2;
    TImage *img1;
    TLabel *lbl3;
    TAdvGlassButton *AdvGlassButton1;
    TAdvGlassButton *AdvGlassButton2;
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall AdvGlassButton1Click(TObject *Sender);
    void __fastcall AdvGlassButton2Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TForm2(TComponent* Owner,String runpath);
    void __fastcall TForm2::ShowMesg(int index);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm2 *Form2;
//---------------------------------------------------------------------------
#endif
