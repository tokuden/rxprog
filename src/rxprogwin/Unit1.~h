//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ImgList.hpp>
#include <ToolWin.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <Menus.hpp>

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE 管理のコンポーネント
	TMemo *Memo1;
	TPanel *Panel1;
	TLabel *Label6;
	TLabel *Label7;
	TLabel *LabelProgress;
	TImage *Image1;
	TEdit *EditFileName;
	TStaticText *StaticText1;
	TProgressBar *ProgressBar1;
	TImageList *ImageList1;
	TLabel *Label2;
	TToolBar *ToolBar2;
	TToolButton *ToolButton14;
	TToolButton *ToolButtonIDCODE;
	TToolButton *ToolButtonDUMP;
	TToolButton *ToolButton15;
	TToolButton *ToolButtonWRITE;
	TToolButton *ToolButtonVERIFY;
	TToolButton *ToolButtonERASE;
	TToolButton *ToolButtonBLANK;
	TToolButton *ToolButtonREAD;
	TToolButton *ToolButton16;
	TToolButton *ToolButtonCancel;
	TButton *Button1;
	TOpenDialog *OpenDialog1;
	TToolButton *ToolButtonFileOpen;
	TToolButton *ToolButtonConnect;
	TCheckBox *CheckBoxWait;
	TPopupMenu *PopupMenu1;
	TMenuItem *Newfile1;
	TMenuItem *N1;
	TMenuItem *N11;
	TMenuItem *N21;
	TMenuItem *N31;
	TMenuItem *N41;
	TMenuItem *N51;
	TMenuItem *N61;
	TMenuItem *N71;
	TMenuItem *N81;
	TMenuItem *N91;
	TMenuItem *N101;
	TRadioGroup *RadioGroup1;
	TSplitter *Splitter1;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall ToolButtonIDCODEClick(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall ToolButtonConnectClick(TObject *Sender);
	void __fastcall ToolButtonSIGClick(TObject *Sender);
	void __fastcall Newfile1Click(TObject *Sender);
	void __fastcall PopupMenu1Popup(TObject *Sender);
	void __fastcall Popup1Click(TObject *Sender);
	void __fastcall Popup2Click(TObject *Sender);
	void __fastcall Popup3Click(TObject *Sender);
	void __fastcall Popup4Click(TObject *Sender);
	void __fastcall Popup5Click(TObject *Sender);
	void __fastcall Popup6Click(TObject *Sender);
	void __fastcall Popup7Click(TObject *Sender);
	void __fastcall Popup8Click(TObject *Sender);
	void __fastcall Popup9Click(TObject *Sender);
	void __fastcall Popup10Click(TObject *Sender);
	void __fastcall ToolButtonCancelClick(TObject *Sender);
	void __fastcall ToolButtonDUMPClick(TObject *Sender);
	void __fastcall ToolButtonWRITEClick(TObject *Sender);
	void __fastcall ToolButtonVERIFYClick(TObject *Sender);
	void __fastcall ToolButtonERASEClick(TObject *Sender);
	void __fastcall ToolButtonBLANKClick(TObject *Sender);
	void __fastcall ToolButtonREADClick(TObject *Sender);
	void __fastcall Label3Click(TObject *Sender);
	void __fastcall CheckBoxWaitClick(TObject *Sender);
	void __fastcall RadioGroup1Click(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// ユーザー宣言
	AnsiString DetectedDevice;
	void __fastcall ShowFileDate();
	void __fastcall ChangeToolButtonEnables();
	void __fastcall DisableAllToolButtons();
	void __fastcall Connect();
	void __fastcall Disconnect();
	void __fastcall ShowBoardPicture();
	void __fastcall OpenRecentUsedFile(int i);
	void __fastcall SetRecentUsedFile(AnsiString FileName);
	bool connected;
	void __fastcall ShowIspResult(int result,char *mes);

public:		// ユーザー宣言
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
