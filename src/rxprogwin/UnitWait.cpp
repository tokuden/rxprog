//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UnitWait.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormWait *FormWait;
//---------------------------------------------------------------------------
__fastcall TFormWait::TFormWait(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormWait::Timer1Timer(TObject *Sender)
{
	Shape1->Tag++;
    if(Shape1->Tag >= 16) Shape1->Tag = 0;

    Shape1->Brush->Color = Shape1->Tag * 0x1000;

	int fh = FileOpen(TargetFileName,0);
	if(fh < 0) return;
	if(FileGetDate(fh) > filedate)
    {
    	ModalResult = mrOk;
	}
	FileClose(fh);
}
//---------------------------------------------------------------------------
void __fastcall TFormWait::FormCreate(TObject *Sender)
{
	filedate = 0;
}
//---------------------------------------------------------------------------
void __fastcall TFormWait::Button1Click(TObject *Sender)
{
	ModalResult = mrCancel;
}
//---------------------------------------------------------------------------

void __fastcall TFormWait::CheckFileDate(AnsiString FileName)
{
	TargetFileName = FileName;
	Memo1->Lines->Add("Waiting for file updated.");
	Memo1->Lines->Add("\"" + TargetFileName + "\"");
	int fh = FileOpen(TargetFileName,0);
	if(fh < 0) return;
	filedate = FileGetDate(fh);
	FileClose(fh);
	Memo1->Lines->Add("Current timestamp is " + FileDateToDateTime(filedate));
}
