//---------------------------------------------------------------------------

#ifndef UnitWaitH
#define UnitWaitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFormWait : public TForm
{
__published:	// IDE 管理のコンポーネント
	TButton *Button1;
	TTimer *Timer1;
	TMemo *Memo1;
	TShape *Shape1;
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
private:	// ユーザー宣言
	int filedate;
    AnsiString TargetFileName;

public:		// ユーザー宣言
	__fastcall TFormWait(TComponent* Owner);
	void __fastcall CheckFileDate(AnsiString FileName);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormWait *FormWait;
//---------------------------------------------------------------------------
#endif
