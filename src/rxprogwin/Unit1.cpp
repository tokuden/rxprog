//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <inifiles.hpp>
#include <stdio.h>

#include "Unit1.h"
#include "UnitWait.h"

#include "rxprog.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------

int r6n_printmsg(const char *, va_list arglist);
void r6n_progress(int pos,int max,const char *);
char *r6n_get_textdb(const char *target);

const char VersionStr[] = "RX62N Alternate Programer for Windows, Version 0.60,  (C)Copyright 2012 TokushuDenshiKairo Inc.";

AnsiString exe_file_path;
bool cont;

__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	exe_file_path = ExtractFilePath(Application->ExeName);

	//コールバック関数の設定
//	set_callback_jtag_progress_message_func(show_progress_message);

	cont = false;
}
//---------------------------------------------------------------------------


// メモに情報を表示
int r6n_print_message(const char *format, va_list arglist)
{
	if(!format || !*format)
	{
		Form1->Memo1->Lines->Add(" ");
		return 0;
	}

	AnsiString A;
	int len = A.vprintf(format,arglist);
	if(!A.Trim().IsEmpty())
	{
//		A = DateTimeToStr(Now()) + "  " + A;
	}
	Form1->Memo1->Lines->Add(A);
	return len;
}

int print_message(const char *format, ... )
{
	int len;
	va_list arglist;
	va_start(arglist, format);

	len = 0;
	if(format && strlen(format))
	{
		len = r6n_print_message(format,arglist);
	}
	va_end(arglist);

	return len;
}

// 進捗バーの表示
void s6w_progress(int pos,int max,const char *msg)
{
	AnsiString A;
	A.printf("%d/%d",pos,max);
	Form1->LabelProgress->Caption = A;
	Form1->ProgressBar1->Max = max;
	Form1->ProgressBar1->Position = pos;
	Application->ProcessMessages();
}


// メモに情報を表示
void mjPrintSPI(const AnsiString format, ... )
{
	AnsiString A;

	if(format.IsEmpty())
	{
		Form1->Memo1->Lines->Add(" ");
		return;
	}

	va_list arglist;
	va_start(arglist, format);
	A.vprintf(format.c_str(),arglist);
	va_end(arglist);

	Form1->Memo1->Lines->Add(A);
}

// 進捗バーの表示
void mjProgressSPI(AnsiString caption,int pos,int max)
{
	AnsiString A;
	A.printf("%d/%d",pos,max);
	Form1->LabelProgress->Caption = A;
	Form1->ProgressBar1->Max = max;
	Form1->ProgressBar1->Position = pos;
	Application->ProcessMessages();
}

static char *s6w_get_textdb(const char *target)
{
//	AnsiString sys_path = "sys\\";

/*
	exe_file_path            = "";//ExtractFilePath(Application->ExeName);
	sys_file_path            = exe_file_path + sys_path;//"sys\\";
	cable_file_path          = exe_file_path + sys_path;//"sys\\";
	ddb_file_search_path     = exe_file_path + sys_path;//"sys\\";
	bsdl_file_search_path    = exe_file_path + "bsdl\\";
	package_file_search_path = exe_file_path + "pkg\\";
*/

	if(!stricmp(target,"bsdl_file_path"))
	{
		return exe_file_path.c_str();
	}

	if(!stricmp(target,"ddb_file_path"))
	{
		return exe_file_path.c_str();
	}

	if(!stricmp(target,"package_file_path"))
	{
		return exe_file_path.c_str();
	}

	if(!stricmp(target,"exe_file_path"))
	{
		return exe_file_path.c_str();
	}

	if(!stricmp(target,"sys_file_path"))
	{
		return exe_file_path.c_str();
	}

	if(!stricmp(target,"data_file_path"))
	{
		return exe_file_path.c_str();
	}

	if(!stricmp(target,"cable_plugin_dir"))
	{
		return exe_file_path.c_str();
	}

	if(!stricmp(target,"overwrite"))
	{
		if(MessageBox(NULL,"Over write OK ?","query",MB_OKCANCEL) == IDOK)
		{
			return "Y";
		}
		else
		{
			return "N";
		}
	}

	return "";
}

void __fastcall TForm1::FormCreate(TObject *Sender)
{
	Memo1->Lines->Add(VersionStr);
	Image1->Picture->LoadFromFile(exe_file_path + "none.bmp");
	Caption = VersionStr;
	connected = false;

	TIniFile *ini = new TIniFile(exe_file_path + "rxprogw.ini");
	RadioGroup1->ItemIndex = ini->ReadInteger("target","board",0);
	delete ini;
	ShowBoardPicture();

	ChangeToolButtonEnables();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ToolButtonIDCODEClick(TObject *Sender)
{
	ShowFileDate();
	Connect();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ToolButtonSIGClick(TObject *Sender)
{
	ShowFileDate();
	Connect();

//	print_message("DeviceDNA:%08lx-%08lx",d2,d1);

	Disconnect();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ShowBoardPicture()
{
	if(RadioGroup1->ItemIndex == 0)
	{
		Image1->Hint = "究極のRX62N評価ボード";
		Image1->Picture->LoadFromFile(exe_file_path + "np1040.bmp");
	}
	if(RadioGroup1->ItemIndex == 1)
	{
		Image1->Hint = "豪華拡張ボードRX-MEGA";
		Image1->Picture->LoadFromFile(exe_file_path + "np1041.bmp");
	}
	if(RadioGroup1->ItemIndex == 2)
	{
		Image1->Hint = "RaXino♪";
		Image1->Picture->LoadFromFile(exe_file_path + "np1050.bmp");
	}
}


void __fastcall TForm1::Button1Click(TObject *Sender)
{
	if((EditFileName->Text.SubString(1,1) == '\\') && !FileExists(EditFileName->Text))
	{
		EditFileName->Text = ExtractFilePath(Application->ExeName);
	}
	if(FileExists(EditFileName->Text))
	{
		OpenDialog1->FileName = EditFileName->Text;
	}
	else
	{
		EditFileName->Text = ExtractFilePath(Application->ExeName);
	}

	OpenDialog1->InitialDir = ExtractFilePath(EditFileName->Text);
	if(OpenDialog1->Execute() == false) return;
	EditFileName->Text = OpenDialog1->FileName;
	ShowFileDate();
	ChangeToolButtonEnables();
	SetRecentUsedFile(EditFileName->Text);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ShowFileDate()
{
	int age = FileAge(EditFileName->Text);
	if(age == -1) return;
	TDateTime D = FileDateToDateTime(age);
	unsigned short year,month,day;
	unsigned short hour,min,sec,msec;

	D.DecodeDate(&year, &month, &day);
	D.DecodeTime(&hour, &min, &sec, &msec);

	const char monstr[][4] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC",};
	if((month < 1) || (month > 12)) return;
	AnsiString A;
	A.printf("%02d/%s/%04d %02d:%02d:%02d",day,monstr[month-1],year,hour,min,sec);
	StaticText1->Caption = A;
}

void __fastcall TForm1::DisableAllToolButtons()
{
	ToolButtonFileOpen->Enabled = false;
	ToolButtonConnect->Enabled = false;
	ToolButtonBLANK->Enabled = false;
	ToolButtonDUMP->Enabled = false;
	ToolButtonERASE->Enabled = false;
	ToolButtonIDCODE->Enabled = false;
	ToolButtonREAD->Enabled = false;
	ToolButtonVERIFY->Enabled = false;
	ToolButtonWRITE->Enabled = false;
	ToolButtonCancel->Enabled = true;
}

void __fastcall TForm1::ChangeToolButtonEnables()
{
	ToolButtonFileOpen->Enabled = true;
	ToolButtonConnect->Enabled = true;
	ToolButtonConnect->Down = connected;
/*
	if(!connected)
	{
		ToolButtonBLANK->Enabled = false;
		ToolButtonDUMP->Enabled = false;
		ToolButtonERASE->Enabled = false;
		ToolButtonIDCODE->Enabled = false;
		ToolButtonREAD->Enabled = false;
		ToolButtonVERIFY->Enabled = false;
		ToolButtonWRITE->Enabled = false;
		ToolButtonCancel->Enabled = false;
		return;
	}
*/

	ToolButtonBLANK->Enabled = true;
	ToolButtonERASE->Enabled = true;
//	ToolButtonIDCODE->Enabled = true;
//	ToolButtonDUMP->Enabled = true;
//	ToolButtonREAD->Enabled = false;
	ToolButtonCancel->Enabled = false;

	if(FileExists(EditFileName->Text)) // ファイルが存在している
	{
		ToolButtonVERIFY->Enabled = true;
		ToolButtonWRITE->Enabled = true;
	}
	else // ファイルが存在していない
	{
		ToolButtonVERIFY->Enabled = false;
		ToolButtonWRITE->Enabled = false;
	}
}

void __fastcall TForm1::Connect()
{
	ShowFileDate();
	print_message("Connecting to RX62N board...");

	if(connected)
	{
		Disconnect();
		connected = false;
	}

	FormWait = new TFormWait(this);
	FormWait->Memo1->Lines->Add("Wait for connection to RX62N");
	int i = 0;
	FormWait->Timer1->Enabled = false;
	FormWait->Show();
	while(FormWait->ModalResult != mrCancel)
	{
		Application->ProcessMessages();
		connected = RX62NConnect();
		if(connected) break;
		FormWait->Shape1->Brush->Color = (i*16 << 16) | i*16;
		i++;
		i &= 15;
		Sleep(100);
	}
	FormWait->Close();
	FormWait->Release();
	delete FormWait;

	ChangeToolButtonEnables();
	ShowBoardPicture();

	if(!connected)
	{
		print_message("Connection failed.");
		ToolButtonConnect->Down = false;
	}
}

void __fastcall TForm1::Disconnect()
{
	RX62NDisconnect();
	connected = false;
}

void __fastcall TForm1::ToolButtonConnectClick(TObject *Sender)
{
	if(!connected) Connect();
	else           Disconnect();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Newfile1Click(TObject *Sender)
{
	Button1->Click();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::PopupMenu1Popup(TObject *Sender)
{
	TIniFile *ini = new TIniFile(exe_file_path + "rxprogw.ini");
	for(int i=0;i<10;i++)
	{
		AnsiString A;
		A = "&" + AnsiString(i+1) + " ";
		A += ini->ReadString("recent","File" + AnsiString(i+1),"");
		PopupMenu1->Items->Items[2+i]->Caption = A;
		PopupMenu1->Tag = i;
	}
	delete ini;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::OpenRecentUsedFile(int i)
{
	TIniFile *ini = new TIniFile(exe_file_path + "rxprogw.ini");
	AnsiString A = ini->ReadString("recent","File" + AnsiString(i),"");
	delete ini;
	EditFileName->Text = A;
	ShowFileDate();
	ChangeToolButtonEnables();
	SetRecentUsedFile(A);
}

void __fastcall TForm1::SetRecentUsedFile(AnsiString FileName)
{
	TStringList *sl = new TStringList();
	TIniFile *ini = new TIniFile(exe_file_path + "rxprogw.ini");

	for(int i=0;i<10;i++)
	{
		AnsiString A = "File" + AnsiString(i+1);
		A = ini->ReadString("recent",A,"");
		sl->Add(A);
	}

	int index = sl->IndexOf(FileName);
	if(index >= 0)
	{
		sl->Delete(index);
	}
	sl->Insert(0,EditFileName->Text);

	for(int i=0;i<10;i++)
	{
		AnsiString A = "File" + AnsiString(i+1);
		ini->WriteString("recent",A,sl->Strings[i]);
	}

	delete sl;
	delete ini;
}

void __fastcall TForm1::Popup1Click(TObject *Sender)
{
	OpenRecentUsedFile(1);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Popup2Click(TObject *Sender)
{
	OpenRecentUsedFile(2);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Popup3Click(TObject *Sender)
{
	OpenRecentUsedFile(3);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Popup4Click(TObject *Sender)
{
	OpenRecentUsedFile(4);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Popup5Click(TObject *Sender)
{
	OpenRecentUsedFile(5);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Popup6Click(TObject *Sender)
{
	OpenRecentUsedFile(6);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Popup7Click(TObject *Sender)
{
	OpenRecentUsedFile(7);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Popup8Click(TObject *Sender)
{
	OpenRecentUsedFile(8);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Popup9Click(TObject *Sender)
{
	OpenRecentUsedFile(9);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Popup10Click(TObject *Sender)
{
	OpenRecentUsedFile(10);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ToolButtonCancelClick(TObject *Sender)
{
	ShowFileDate();
	cont = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ToolButtonDUMPClick(TObject *Sender)
{
	ShowFileDate();
	Connect();

//	SpiDump();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ToolButtonWRITEClick(TObject *Sender)
{
	if(cont) return;
	cont = true;

	if(CheckBoxWait->Checked)
	{
		FormWait = new TFormWait(this);
		FormWait->CheckFileDate(EditFileName->Text);
		int result = FormWait->ShowModal();
		FormWait->Close();
		FormWait->Release();
		delete FormWait;
		if(result == mrCancel)
		{
			printf("書き込みはキャンセルされました");
			Disconnect();
			return;
		}
	}
	ShowFileDate();

	print_message("RX62Nへの接続を待っています");
	DisableAllToolButtons();
	Connect();
	while(!connected && cont)
	{
		connected = RX62NConnect();
		if(connected) break;
		Disconnect();
		Application->ProcessMessages();
	}
	if(!cont)
	{
		ShowIspResult(-2,"Connect");
		ChangeToolButtonEnables();
		return;
	}

	if(RX62NEraseUserMat()) {
		print_message("ユーザマットを消去しました。\n");
	}
	else {
		print_message("ユーザマットの消去で失敗しました。\n");
		ShowIspResult(-1,"Erase");
		ChangeToolButtonEnables();
		cont = false;
		return;
	}

	if(RX62NWriteUserMat(EditFileName->Text.c_str())) {
		print_message("書き込み完了。\n");
	}
	else {
		print_message("書き込み失敗。\n");
		ShowIspResult(-1,"Write");
		ChangeToolButtonEnables();
		cont = false;
		return;
	}

	if(RX62NVerifyUserMat(EditFileName->Text.c_str())) {
		print_message("ベリファイ成功。\n");
	}
	else {
		print_message("ベリファイ失敗。\n");
		ShowIspResult(-1,"Verify");
		ChangeToolButtonEnables();
		cont = false;
		return;
	}

	cont = false;
	ShowIspResult(1,"Program");

	ChangeToolButtonEnables();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ToolButtonVERIFYClick(TObject *Sender)
{
	if(cont) return;
	cont = true;

	ShowFileDate();

	print_message("RX62Nへの接続を待っています");
	DisableAllToolButtons();
	Connect();
	while(!connected && cont)
	{
		connected = RX62NConnect();
		if(connected) break;
		Disconnect();
		Application->ProcessMessages();
	}
	if(!cont)
	{
		ShowIspResult(-2,"Connect");
		ChangeToolButtonEnables();
		return;
	}

	if(RX62NVerifyUserMat(EditFileName->Text.c_str())) {
		print_message("ベリファイ成功。\n");
	}
	else {
		print_message("ベリファイ失敗。\n");
		ShowIspResult(-1,"Verify");
		ChangeToolButtonEnables();
		cont = false;
		return;
	}

	cont = false;
	ShowIspResult(1,"Verify");

	ChangeToolButtonEnables();}
//---------------------------------------------------------------------------

void __fastcall TForm1::ToolButtonERASEClick(TObject *Sender)
{
	if(cont) return;
	cont = true;

	print_message("RX62Nへの接続を待っています");
	DisableAllToolButtons();
	Connect();
	while(!connected && cont)
	{
		connected = RX62NConnect();
		if(connected) break;
		Disconnect();
		Application->ProcessMessages();
	}
	if(!cont)
	{
		ShowIspResult(-2,"Connect");
		ChangeToolButtonEnables();
		return;
	}

	if(RX62NEraseUserMat()) {
		print_message("ユーザマットを消去しました。\n");
	}
	else {
		print_message("ユーザマットの消去で失敗しました。\n");
		ShowIspResult(-1,"Erase");
		ChangeToolButtonEnables();
		cont = false;
		return;
	}

	cont = false;
	ShowIspResult(1,"Erase");

	ChangeToolButtonEnables();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ToolButtonBLANKClick(TObject *Sender)
{
	if(cont) return;
	cont = true;

	print_message("RX62Nへの接続を待っています");
	DisableAllToolButtons();
	Connect();
	while(!connected && cont)
	{
		connected = RX62NConnect();
		if(connected) break;
		Disconnect();
		Application->ProcessMessages();
	}
	if(!cont)
	{
		ShowIspResult(-2,"Connect");
		ChangeToolButtonEnables();
		return;
	}

	if(RX62NQueryBlankCheck()) {
		print_message("デバイスはブランクです。\n");
	}
	else {
		print_message("デバイスはブランクではありません。\n");
		ShowIspResult(-1,"Blank");
		ChangeToolButtonEnables();
		cont = false;
		return;
	}

	cont = false;
	ShowIspResult(1,"Blank");

	ChangeToolButtonEnables();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ToolButtonREADClick(TObject *Sender)
{
	ShowFileDate();
	Connect();

	Disconnect();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Label3Click(TObject *Sender)
{
	if(CheckBoxWait->Checked) CheckBoxWait->Checked = false;
	else CheckBoxWait->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ShowIspResult(int result,char *mes)
{
	HDC hDC;
	TCanvas *DesktopCanvas;
	Graphics::TBitmap *backup = NULL;
	AnsiString A = AnsiString(mes);

	// 実行していることの表示
	try
	{
		hDC = GetDC(0);
		DesktopCanvas = new TCanvas();
		DesktopCanvas->Handle = hDC;

		int x = this->Left + this->Width/2;
		int y = this->Top + this->Height/2;

		if(result == 1)
		{
			A += " Success!!";
			DesktopCanvas->Brush->Color = clAqua;
			DesktopCanvas->Font->Color = clBlack;
		}
		else if(result == -2)
		{
			A += " Canceled";
			DesktopCanvas->Brush->Color = clYellow;
			DesktopCanvas->Font->Color = clBlack;
		}
		else if(result == -1)
		{
			A += " Failed!!";
			DesktopCanvas->Brush->Color = clFuchsia;
			DesktopCanvas->Font->Color = clBlack;
		}

		DesktopCanvas->Font->Size = 24;

		int w = DesktopCanvas->TextWidth(A);
		int h = DesktopCanvas->TextHeight(A);

        TRect rect;
		rect.left   = x-w*0.5*1.5;
		rect.top    = y-h*0.5*1.5;
		rect.right  = x+w*0.5*1.5;
		rect.bottom = y+h*0.5*1.5;

		backup = new Graphics::TBitmap();
		backup->Width = rect.Width();
		backup->Height = rect.Height();

		backup->Canvas->CopyRect(TRect(0,0,backup->Width,backup->Height),
			DesktopCanvas,rect);

		DesktopCanvas->Ellipse(rect.left, rect.top, rect.right, rect.bottom);
		DesktopCanvas->TextOutA(x-w/2,y-h/2,A);

		Sleep(1000);

		DesktopCanvas->CopyRect(rect,backup->Canvas,TRect(0,0,backup->Width,backup->Height));
		delete backup;
	}
	__finally
	{
		ReleaseDC(0,DesktopCanvas->Handle);
		delete DesktopCanvas;
	}
}


void __fastcall TForm1::CheckBoxWaitClick(TObject *Sender)
{
	ShowFileDate();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::RadioGroup1Click(TObject *Sender)
{
	TIniFile *ini = new TIniFile(exe_file_path + "rxprogw.ini");
	ini->WriteInteger("target","board",RadioGroup1->ItemIndex);
	delete ini;

	ShowBoardPicture();
}
//---------------------------------------------------------------------------



void __fastcall TForm1::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	cont = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
	cont = false;
}
//---------------------------------------------------------------------------

