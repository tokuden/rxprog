#include <stdio.h>
#include <vcl.h>
#include "lusb0_usb.h"
#include <vector>
#include "loadmot.h"

int print_message(const char *format, ... );
void s6w_progress(int pos,int max,const char *msg);

using namespace std;

// 逓倍比
typedef vector<unsigned char>  TClockMultInfo;  // {1 2 4 8}が入る
typedef vector<TClockMultInfo> TClockMultInfos; // {1 2 4 8}{1 2 4 8}が入る

// 動作周波数
typedef struct
{
	unsigned long MinFreq;
	unsigned long MaxFreq;
} TClockFreqInfo;
typedef vector<TClockFreqInfo> TClockFreqs;

// 開始アドレスと終了アドレスのペア
typedef struct {
	unsigned long StartAddr;
	unsigned long LastAddr;
} TAddrPair;

struct {
	char              SupportDevCode[5]; // 通常は4バイトで"6y05"
	char             *SupportDevName;    // 通常は"RX600 Series"
	unsigned char     ClockMode;         // 通常は0x00
	TClockMultInfos   ClockMultInfos;    // クロック逓倍比情報
	TClockFreqs       ClockFreqs;        // クロック周波数情報
	vector<TAddrPair> UserBootMat;       // ユーザブートマット情報
	vector<TAddrPair> UserMat;           // ユーザマット情報
	vector<TAddrPair> EraseBlocks;       // 消去ブロック情報
	unsigned long     WriteSize;         // 書き込みサイズ
} RX62NProgramingInfo;

void show_result(char *message,unsigned char *buf,int len)
{
	if(message) print_message("%s",message);
	for(int i=0;i<len;i++)
	{
		print_message("%02X ",buf[i]);
	}
	print_message(" \"");
	for(int i=0;i<len;i++)
	{
		if(isprint(buf[i])) print_message("%c",buf[i]);
		else                print_message(".");
	}
	print_message("\"\n");
}

unsigned char calculate_sum(unsigned char *buf,int len)
{
	char sum = 0;
	for(int i=0;i<len;i++) sum += buf[i];
	return ~sum + 1;
}

typedef enum {
	ERR_OK      = 0,
	ERR_IO      = -1,
	ERR_CSUM    = -2,
	ERR_ILLRES  = -3,
	ERR_PROTECT = -4,
	ERR_VERIFY  = -5,
	ERR_UNKNOWN = -6,
	ERR_TXSUM   = 0x11,
	ERR_DEVCODE = 0x21,
	ERR_CLKMODE = 0x22,
	ERR_BITRATE = 0x24,
	ERR_INFREQ  = 0x25,
	ERR_MULT    = 0x26,
	ERR_OPFREQ  = 0x27,
	ERR_BLKNUM  = 0x29,
	ERR_ADDR    = 0x2A,
	ERR_DATALEN = 0x2B,
	ERR_ERASE   = 0x51,
	ERR_NOERASE = 0x52,
	ERR_WRITE   = 0x53,
	ERR_SELECT  = 0x54,
	ERR_COMMAND = 0x80,
	ERR_BITR2   = 0xFF,
} ERR_STAT;

#define MY_VID 0x045B
#define MY_PID 0x0025
#define MY_CONFIG 1
#define MY_INTF 0
#define EP_OUT 0x01
#define EP_IN 0x82

usb_dev_handle *dev;

usb_dev_handle *open_dev(void)
{
	struct usb_bus *bus;
    struct usb_device *dev;

    for (bus = usb_get_busses(); bus; bus = bus->next)
	{
        for (dev = bus->devices; dev; dev = dev->next)
		{
            if (dev->descriptor.idVendor == MY_VID
                    && dev->descriptor.idProduct == MY_PID)
            {
				return usb_open(dev);
			}
        }
    }
    return NULL;
}

bool OpenLibUsb() {
	dev = NULL;

	usb_init(); /* initialize the library */
	usb_find_busses(); /* find all busses */
	usb_find_devices(); /* find all connected devices */

	if (!(dev = open_dev()))
	{
		return false;
	}

	if (usb_set_configuration(dev, 1) < 0)
	{
		usb_close(dev);
		return false;
	}

	if (usb_claim_interface(dev, 0) < 0)
	{
		usb_close(dev);
		return false;
	}
	return true;
}


ERR_STAT CommandQuery(unsigned char cmd,unsigned char *rbuf,int *rlen,unsigned char resultexp) {
	int ret;
	unsigned char buf[64];
	unsigned char csum = 0;

	buf[0] = cmd;
	ret = usb_bulk_write(dev, EP_OUT, buf, 1, 5000);
	if(ret < 0) return ERR_IO;

	if(rlen) *rlen = 0;
	unsigned char *p = rbuf;
	do {
		ret = usb_bulk_read(dev, EP_IN, buf, 64, 5000);
		if(ret < 0) return ERR_IO;
		if(ret == FALSE) return ERR_IO;
		for(DWORD i=0;i<ret;i++) {
			csum += buf[i];
		}

		// 結果を格納
		memcpy(p,buf,ret);
		p += ret;
		if(rlen) *rlen += ret;
	} while ((ret == 64) && (cmd == 0x26)); // 消去ブロックの場合でまだ続きがある場合(ちょっと横着)

	if(resultexp == 0x00) return ERR_OK;
	// チェックサム
	ret = usb_bulk_read(dev, EP_IN, buf, 1, 5000);
	if(ret < 0) return ERR_IO;
	if((csum + buf[0]) & 0xff != 0) return ERR_CSUM;

	// 異常な結果をチェック
	if(rbuf[0] != resultexp) return ERR_ILLRES;

	return ERR_OK;
}

ERR_STAT CommandSend(unsigned char cmd,unsigned char datalen,unsigned char *data) {
	int ret;
	unsigned char buf[64];

	buf[0] = cmd;
	buf[1] = datalen; // データの長さ
	memcpy(&buf[2],data,datalen);
	buf[2 + datalen] = calculate_sum(buf,2 + datalen);
	ret = usb_bulk_write(dev, EP_OUT, buf, datalen + 3, 5000);
	if(ret < 0) return ERR_IO;
	ret = usb_bulk_read(dev, EP_IN, buf, 64, 5000);
	if(buf[0] == 0x06) return ERR_OK;
	else {
		if(buf[1] == 0x11) return ERR_TXSUM;
		if(buf[1] == 0x21) return ERR_DEVCODE;
		if(buf[1] == 0x22) return ERR_CLKMODE;
		if(buf[1] == 0x24) return ERR_BITRATE;
		if(buf[1] == 0x25) return ERR_INFREQ;
		if(buf[1] == 0x26) return ERR_MULT;
		if(buf[1] == 0x27) return ERR_OPFREQ;
		if(buf[1] == 0x29) return ERR_BLKNUM;
		if(buf[1] == 0x2A) return ERR_ADDR;
		if(buf[1] == 0x2B) return ERR_DATALEN;
		if(buf[1] == 0x51) return ERR_ERASE;
		if(buf[1] == 0x52) return ERR_NOERASE;
		if(buf[1] == 0x53) return ERR_WRITE;
		if(buf[1] == 0x54) return ERR_SELECT;
		if(buf[1] == 0x80) return ERR_COMMAND;
		if(buf[1] == 0xFF) return ERR_BITR2;
	}
	return ERR_UNKNOWN;
}

ERR_STAT SendProgData(unsigned char cmd,unsigned long addr,unsigned char *data,int datalen) {
	int ret;
	unsigned char buf[1024+64];

	buf[0] = cmd;
	buf[1] = addr >> 24;
	buf[2] = addr >> 16;
	buf[3] = addr >> 8;
	buf[4] = addr & 0xff;
	if(datalen) memcpy(&buf[5],data,datalen);
	buf[5 + datalen] = calculate_sum(buf,5 + datalen);
	ret = usb_bulk_write(dev, EP_OUT, buf, datalen + 6, 5000);
	if(ret < 0) return ERR_IO;
	ret = usb_bulk_read(dev, EP_IN, buf, 64, 5000);
	if(buf[0] == 0x06) return ERR_OK;
	else {
		if(buf[1] == 0x11) return ERR_TXSUM;
		if(buf[1] == 0x2A) return ERR_ADDR;
		if(buf[1] == 0x53) return ERR_WRITE;
	}
	return ERR_UNKNOWN;
}

bool ShowErrorStat(const char *cmd,ERR_STAT err)
{
	if(err == ERR_OK) return true;
	print_message("Error:\"%s\"の操作で",cmd);
	switch(err)
	{
		case ERR_IO:
			print_message("I/Oエラーが発生しました。\n");
			break;
		case ERR_CSUM:
			print_message("受信チェックサムエラーが発生しました。\n");
			break;
		case ERR_ILLRES:
			print_message("異常なレスポンスが返りました。\n");
			break;
		case ERR_PROTECT:
			print_message("IDコードプロテクトがかけられています。\n");
			break;
		case ERR_VERIFY:
			print_message("ベリファイでエラーが発生しました。\n");
			break;

		case ERR_TXSUM:
			print_message("送信チェックサムエラーが発生しました。\n");
			break;
		case ERR_DEVCODE:
			print_message("デバイスコード不一致エラーが発生しました。\n");
			break;
		case ERR_CLKMODE:
			print_message("クロックモード不一致エラーが発生しました。\n");
			break;
		case ERR_BITRATE:
			print_message("ビットレート選択不可エラーが発生しました。\n");
			break;
		case ERR_INFREQ:
			print_message("入力周波数エラーが発生しました。\n");
			break;
		case ERR_MULT:
			print_message("逓倍比エラーが発生しました。\n");
			break;
		case ERR_OPFREQ:
			print_message("動作周波数エラーが発生しました。\n");
			break;
		case ERR_BLKNUM:
			print_message("ブロック番号エラーが発生しました。\n");
			break;
		case ERR_ADDR:
			print_message("アドレスエラーが発生しました。\n");
			break;
		case ERR_DATALEN:
			print_message("データ長エラーが発生しました。\n");
			break;
		case ERR_ERASE:
			print_message("消去エラーが発生しました。\n");
			break;
		case ERR_NOERASE:
			print_message("未消去エラーが発生しました。\n");
			break;
		case ERR_WRITE:
			print_message("書き込みエラーが発生しました。\n");
			break;
		case ERR_SELECT:
			print_message("選択処理エラーが発生しました。\n");
			break;
		case ERR_COMMAND:
			print_message("コマンドエラーが発生しました。\n");
			break;
		case ERR_BITR2:
			print_message("ビットレート合わせ込み確認エラーが発生しました。\n");
			break;
		default:
			print_message("不明なエラーが発生しました。\n");
			break;
	}
	return false;
}

const char *ConvProgramStatus(unsigned char embstat)
{
	switch(embstat)
	{
		case 0x11:
			return "デバイス選択待ち";
		case 0x12:
			return "クロックモード選択待ち";
		case 0x13:
			return "ビットレート選択待ち";
		case 0x1F:
			return "書き込み／消去ホストコマンド待ち状態への遷移待ち";
		case 0x31:
			return "ユーザマット／ユーザブートマットの消去中";
		case 0x3F:
			return "書き込み消去／ホストコマンド待ち";
		case 0x4F:
			return "書き込みデータ受信待ち";
		case 0x5F:
			return "消去ブロック指定待ち";
		default:
			return "不明";
	}
}

// サポートデバイス問い合わせ
bool RX62NQuerySupportDevice() {
	unsigned char buf[64];
	int rlen;
	ERR_STAT err = CommandQuery(0x20,buf,&rlen,0x30);
	if(err != ERR_OK) {
		ShowErrorStat("サポートデバイス問い合わせ",err);
		return false;
	}
//	int devnum = buf[2];
	int slen   = buf[3];
	// デバイスコードをコピー
	memcpy(RX62NProgramingInfo.SupportDevCode,&buf[4],4);
	RX62NProgramingInfo.SupportDevCode[4] = '\0';
	// デバイス名をコピー
	RX62NProgramingInfo.SupportDevName = (char *)malloc(slen-4+1);
	strncpy(RX62NProgramingInfo.SupportDevName,&buf[8],slen-4);
	RX62NProgramingInfo.SupportDevName[slen-4] = '\0';
	return true;
}

bool RX62NSelectDevice(char *devcode)
{
	ERR_STAT err = CommandSend(0x10,4,RX62NProgramingInfo.SupportDevCode);
	if(err != ERR_OK) {
		ShowErrorStat("デバイス選択",err);
		return false;
	}
	return true;
}

bool RX62NSelectClockMode(unsigned char clockmode) {
	ERR_STAT err = CommandSend(0x11,2,&clockmode);
	if(err != ERR_OK) {
		ShowErrorStat("クロックモード選択",err);
		return false;
	}
	return true;
}

bool RX62NSelectBitRate(unsigned short bitrate,unsigned short infreq,unsigned char clknum,unsigned char mul1,unsigned char mul2) {
	unsigned char data[7];
	data[0] = bitrate >> 8;
	data[1] = bitrate & 0xff;
	data[2] = infreq >> 8;
	data[3] = infreq & 0xff;
	data[4] = clknum;
	data[5] = mul1;
	data[6] = mul2;
	ERR_STAT err = CommandSend(0x3f,7,data);
	if(err != ERR_OK) {
		ShowErrorStat("新ビットレート選択",err);
		return false;
	}

	// 確認とレスポンス
	data[0] = 0x06;
	int ret;
	ret = usb_bulk_write(dev, EP_OUT, data, 1,5000); // 1バイト送信
	if(ret < 0) err = ERR_IO;
	ret = usb_bulk_read(dev, EP_IN , data, 1,5000); // 1バイト受信
	if(ret < 0) err = ERR_IO;
	if(data[0] != 0x06) err = ERR_BITR2;

	if(err != ERR_OK) {
		ShowErrorStat("新ビットレート選択確認",err);
		return false;
	}
	return true;
}

// クロックモード問い合わせ
bool RX62NQueryClockMode() {
	unsigned char buf[64];
	int rlen;
	ERR_STAT err = CommandQuery(0x21,buf,&rlen,0x31);
	if(err != ERR_OK) {
		ShowErrorStat("クロックモード問い合わせ",err);
		return false;
	}
	RX62NProgramingInfo.ClockMode = buf[2];
	return true;
}

// 逓倍比問い合わせ
bool RX62NQueryClockMultiply() {
	unsigned char buf[64];
	int rlen;
	ERR_STAT err = CommandQuery(0x22,buf,&rlen,0x32);
	if(err != ERR_OK) {
		ShowErrorStat("逓倍比問い合わせ",err);
		return false;
	}
	int clocknum = buf[2];
	unsigned char *p = &buf[3];
	for(int i=0;i<clocknum;i++) {
		int numtype = *p++;
		TClockMultInfo mul;
		for(int j=0;j<numtype;j++) {
			mul.push_back(*p++);
		}
		RX62NProgramingInfo.ClockMultInfos.push_back(mul);
	}
	return true;
}

// 動作周波数問い合わせ
bool RX62NQueryFreq() {
	unsigned char buf[64];
	int rlen;
	ERR_STAT err = CommandQuery(0x23,buf,&rlen,0x33);
	if(err != ERR_OK) {
		ShowErrorStat("動作周波数問い合わせ",err);
		return false;
	}
	int clocknum = buf[2];
	unsigned char *p = &buf[3];
	for(int i=0;i<clocknum;i++) {
		TClockFreqInfo freq;
		freq.MinFreq = *p++;
		freq.MinFreq <<= 8;
		freq.MinFreq |= *p++;
		freq.MinFreq *= 10000;
		freq.MaxFreq = *p++;
		freq.MaxFreq <<= 8;
		freq.MaxFreq |= *p++;
		freq.MaxFreq *= 10000;
		RX62NProgramingInfo.ClockFreqs.push_back(freq);
	}
	return true;
}

// ユーザブートマット問い合わせ
bool RX62NQueryUserBootMat() {
	unsigned char buf[64];
	int rlen;
	ERR_STAT err = CommandQuery(0x24,buf,&rlen,0x34);
	if(err != ERR_OK) {
		ShowErrorStat("ユーザブートマット問い合わせ",err);
		return false;
	}
	int areas = buf[2];
	unsigned char *p = &buf[3];
	for(int i=0;i<areas;i++) {
		TAddrPair ap;
		ap.StartAddr = *p++;
		ap.StartAddr <<= 8;
		ap.StartAddr |= *p++;
		ap.StartAddr <<= 8;
		ap.StartAddr |= *p++;
		ap.StartAddr <<= 8;
		ap.StartAddr |= *p++;
		ap.LastAddr = *p++;
		ap.LastAddr <<= 8;
		ap.LastAddr |= *p++;
		ap.LastAddr <<= 8;
		ap.LastAddr |= *p++;
		ap.LastAddr <<= 8;
		ap.LastAddr |= *p++;
		RX62NProgramingInfo.UserBootMat.push_back(ap);
	}
	return true;
}

// ユーザマット問い合わせ
bool RX62NQueryUserMat() {
	unsigned char buf[64];
	int rlen;
	ERR_STAT err = CommandQuery(0x25,buf,&rlen,0x35);
	if(err != ERR_OK) {
		ShowErrorStat("ユーザマット問い合わせ",err);
		return false;
	}
	int areas = buf[2];
	unsigned char *p = &buf[3];
	for(int i=0;i<areas;i++) {
		TAddrPair ap;
		ap.StartAddr = *p++;
		ap.StartAddr <<= 8;
		ap.StartAddr |= *p++;
		ap.StartAddr <<= 8;
		ap.StartAddr |= *p++;
		ap.StartAddr <<= 8;
		ap.StartAddr |= *p++;
		ap.LastAddr |= *p++;
		ap.LastAddr <<= 8;
		ap.LastAddr |= *p++;
		ap.LastAddr <<= 8;
		ap.LastAddr |= *p++;
		ap.LastAddr <<= 8;
		ap.LastAddr |= *p++;
		RX62NProgramingInfo.UserMat.push_back(ap);
	}
	return true;
}

// 消去ブロック問い合わせ
bool RX62NQueryEraseBlock() {
	unsigned char buf[1024];
	int rlen;
	ERR_STAT err = CommandQuery(0x26,buf,&rlen,0x36);
	if(err != ERR_OK) {
		ShowErrorStat("消去ブロック問い合わせ",err);
		return false;
	}
//	int size   = (buf[1] << 8) | buf[2];
	int blocks = buf[3];
	unsigned char *p = &buf[4];
	for(int i=0;i<blocks;i++) {
		TAddrPair ap;
		ap.StartAddr = *p++;
		ap.StartAddr <<= 8;
		ap.StartAddr |= *p++;
		ap.StartAddr <<= 8;
		ap.StartAddr |= *p++;
		ap.StartAddr <<= 8;
		ap.StartAddr |= *p++;
		ap.LastAddr = *p++;
		ap.LastAddr <<= 8;
		ap.LastAddr |= *p++;
		ap.LastAddr <<= 8;
		ap.LastAddr |= *p++;
		ap.LastAddr <<= 8;
		ap.LastAddr |= *p++;
		RX62NProgramingInfo.EraseBlocks.push_back(ap);
	}
	return true;
}

// 書き込みサイズ問い合わせ
bool RX62NQueryWriteSize() {
	unsigned char buf[1024];
	int rlen;
	ERR_STAT err = CommandQuery(0x27,buf,&rlen,0x37);
	if(err != ERR_OK) {
		ShowErrorStat("書き込みサイズ問い合わせ",err);
		return false;
	}
	unsigned char *p = &buf[2];
	RX62NProgramingInfo.WriteSize = *p++;
	RX62NProgramingInfo.WriteSize <<= 8;
	RX62NProgramingInfo.WriteSize |= *p++;
	return true;
}

bool RX62NEnterWriteAndErase() {
	int ret;
	unsigned char buf[64];
	ERR_STAT err = ERR_OK;
	buf[0] = 0x40;

	do {
		ret = usb_bulk_write(dev, EP_OUT, buf, 1, 5000); // 1バイト送信
		if(ret < 0) {
			err = ERR_IO;
			break;
		}
		ret = usb_bulk_read(dev, EP_IN, buf, 64, 5000); // 1バイト送信
		if(ret == FALSE) {
			err = ERR_IO;
			break;
		}
		if(buf[0] == 0x06) { // なぜか0x06が返る？？？★
			err = ERR_OK;
			break;
		}
		if(buf[0] == 0x26) {
			err = ERR_OK;
			break;
		}
		if(buf[0] == 0x16) {
			err = ERR_PROTECT;
			break;
		}
		if((buf[0] == 0xC0) && (buf[1] == 0x51)) {
			err = ERR_ERASE;
			break;
		}
		err = ERR_UNKNOWN;
	} while(0);
	if(err != ERR_OK) {
		ShowErrorStat("書き込み／消去ステータス遷移",err);
		return false;
	}
	return true;
}

// 組み込みプログラムステータス問い合わせ
ERR_STAT RX62NQueryProgramStatus(unsigned char *embstat) {
	unsigned char buf[64];
	int rlen;
	ERR_STAT err = CommandQuery(0x4f,buf,&rlen,0x5f);
	if(err != ERR_OK) {
		ShowErrorStat("組み込みプログラムステータス問い合わせ",err);
		return err;
	}
	if(embstat) *embstat = buf[2];
	if(buf[3] != 0x00) {
		ShowErrorStat("組み込みプログラムステータス問い合わせ",err);
		return buf[3];
	}
//	print_message("組み込みプログラムステータス=[%s]\n",ConvProgramStatus(buf[2]));
	return ERR_OK;
}

bool setup_rx62n()
{
	unsigned char buf[1024+64];
	int ret;
	DWORD size;

	// 1byte書き込む
	buf[0] = 0x55;
	ret = usb_bulk_write(dev, EP_OUT, buf, 1, 5000);
	if(ret < 0) return false;
	ret = usb_bulk_read(dev, EP_IN, buf, 64, 5000);
	if(ret < 0) return false;

	if(buf[0] != 0xe6)
	{
		if((buf[0] == 0x80) && (buf[1] == 0x55)) // エラー
		{
			print_message("RX62Nはすでにセットアップされているため続行できません。デフォルト値を採用します\n");
			strcpy(RX62NProgramingInfo.SupportDevCode,"6y05");
			RX62NProgramingInfo.SupportDevName = strdup("RX600 Series");
			RX62NProgramingInfo.ClockMode = 0;
			TClockMultInfo clkmultinfo;
			clkmultinfo.push_back(1);
			clkmultinfo.push_back(2);
			clkmultinfo.push_back(4);
			clkmultinfo.push_back(8);
			RX62NProgramingInfo.ClockMultInfos.push_back(clkmultinfo);
			TClockFreqInfo freqinfo;
			freqinfo.MinFreq = 8000000;
			freqinfo.MaxFreq = 100000000;
			RX62NProgramingInfo.ClockFreqs.push_back(freqinfo);
			freqinfo.MinFreq = 8000000;
			freqinfo.MaxFreq = 50000000;
			RX62NProgramingInfo.ClockFreqs.push_back(freqinfo);
			TAddrPair ap;
			ap.StartAddr = 0xfff80000;
			ap.LastAddr = 0xffffffff;
			RX62NProgramingInfo.UserMat.push_back(ap);
			ap.StartAddr = 0;
			ap.LastAddr = 0;
			for(int i=0;i<54;i++) RX62NProgramingInfo.EraseBlocks.push_back(ap);
			RX62NProgramingInfo.WriteSize = 0x400;
			return true;
		}
		else
		{
			print_message("RX62Nがおかしなモードになっています。リセットしてください\n");
			return FALSE;
		}
	}

	// サポートデバイス問い合わせ
	if(!RX62NQuerySupportDevice()) return FALSE;

	// デバイス選択
	if(!RX62NSelectDevice(RX62NProgramingInfo.SupportDevCode)) return FALSE;

	// クロックモード問い合わせ
	if(!RX62NQueryClockMode()) return FALSE;

	// クロックモード選択
	if(!RX62NSelectClockMode(RX62NProgramingInfo.ClockMode)) return FALSE;

	// 逓倍比問い合わせ
	if(!RX62NQueryClockMultiply()) return FALSE;

	// 動作周波数問い合わせ
	if(!RX62NQueryFreq()) return FALSE;

    // ユーザブートマット情報問い合わせ
	if(!RX62NQueryUserBootMat()) return FALSE;

	// ユーザマット情報問い合わせ
	if(!RX62NQueryUserMat()) return FALSE;

	// 消去ブロック情報問い合わせ
	if(!RX62NQueryEraseBlock()) return FALSE;

    // 書き込みサイズ問い合わせ
	if(!RX62NQueryWriteSize()) return FALSE;

	// 新ビットレート選択
	unsigned short bitrate = 115200 / 100; // ビットレート値を1/100した値を設定
	unsigned short infreq  = 12 * 100;     // 入力周波数の小数点第2位までを100倍した値を設定
	int clknum  = RX62NProgramingInfo.ClockFreqs.size();
	int mul1 = 8; // システムクロック 12MHz*8=96MHz
	int mul2 = 4; // 周辺クロック     12MHz*4=48MHz
	if(!RX62NSelectBitRate(bitrate,infreq,clknum,mul1,mul2)) return FALSE;

	// 組み込みプログラムステータス問い合わせ
	if(RX62NQueryProgramStatus(NULL) != ERR_OK) return FALSE;

	// 書き込み／消去ステータス遷移
	if(!RX62NEnterWriteAndErase()) return FALSE;

	// 組み込みプログラムステータス問い合わせ
	if(RX62NQueryProgramStatus(NULL) != ERR_OK) return FALSE;

	return TRUE;
}

BOOL RX62NQueryBlankCheck()
{
	unsigned char buf[64];
	int rlen;
	ERR_STAT err = CommandQuery(0x4d,buf,&rlen,0x00);
	if(err != ERR_OK) {
		ShowErrorStat("ユーザマットブランクチェック",err);
		return false;
	}
	if(buf[0] == 0x06) return true;
	return false;
}

BOOL RX62NEraseUserMat() {
	unsigned char buf[64];
	int rlen;
	buf[0] = 0x48;
	ERR_STAT err = CommandQuery(0x48,buf,&rlen,0x00);
	if(err != ERR_OK) {
		ShowErrorStat("消去選択",err);
		return false;
	}

	// 組み込みプログラムステータス問い合わせ
	if(RX62NQueryProgramStatus(NULL) != ERR_OK) return FALSE;

	print_message("ブロック消去開始 ");
	for(unsigned int i=0;i<RX62NProgramingInfo.EraseBlocks.size();i++)
	{
		// ブロック消去
		buf[0] = i;
		err = CommandSend(0x58,1,buf);
		if(err != ERR_OK) {
			ShowErrorStat("ブロック消去",err);
			return false;
		}
		s6w_progress(i,RX62NProgramingInfo.EraseBlocks.size(),"Block Erase");
	}
	s6w_progress(0,0,"Block Erase");

	// ブロック消去
	buf[0] = 0xff; // 終了
	err = CommandSend(0x58,1,buf);
	if(err != ERR_OK) {
		ShowErrorStat("ブロック消去",err);
		return false;
	}

	// 組み込みプログラムステータス問い合わせ
	if(RX62NQueryProgramStatus(NULL) != ERR_OK) return FALSE;
	return TRUE;
}

bool RX62NWriteUserMat(const char *filename)
{
	unsigned char *progdata;
	unsigned char msgbuf[1024];

	int bufsize = RX62NProgramingInfo.UserMat[0].LastAddr - RX62NProgramingInfo.UserMat[0].StartAddr + 1;
	progdata = new unsigned char [bufsize]; // RX62Nでは524288になるはず
	unsigned long startaddr,lastaddr,entryaddr;

	startaddr = RX62NProgramingInfo.UserMat[0].StartAddr;
	lastaddr = RX62NProgramingInfo.UserMat[0].LastAddr;

	if(!load_mot_file(filename,progdata,bufsize,0xff,&startaddr,&lastaddr,&entryaddr,msgbuf,1024)) {
		print_message("ファイル%sの読み込みで失敗しました\n",filename);
		return false;
	}

	// ユーザマット選択
	unsigned char buf[64];
	int rlen;
	ERR_STAT err = CommandQuery(0x43,buf,&rlen,0x00);
	if(err != ERR_OK) {
		ShowErrorStat("ユーザマット選択",err);
		return false;
	}

	// 組み込みプログラムステータス問い合わせ
	if(RX62NQueryProgramStatus(NULL) != ERR_OK) return FALSE;

	print_message("書き込み開始 ");
	unsigned char *p = progdata;
	unsigned long addr = 0xfff80000;
	while(addr != 0x00000000) {
		// 1024バイト書き込み
		err = SendProgData(0x50,addr,p,RX62NProgramingInfo.WriteSize);
		if(err != ERR_OK) {
			ShowErrorStat("書き込みデータ転送",err);
			return false;
		}

		addr += RX62NProgramingInfo.WriteSize;
		p +=  RX62NProgramingInfo.WriteSize;
		s6w_progress(addr - 0xfff80000,0x80000,"Programing");
	}

	// 1024バイト書き込み
	SendProgData(0x50,0xffffffff,NULL,0);
	s6w_progress(0x80000,0x80000,"Programing");

	// 組み込みプログラムステータス問い合わせ
	if(RX62NQueryProgramStatus(NULL) != ERR_OK) return FALSE;

	return TRUE;
}

bool RX62NVerifyUserMat(const char *filename)
{
	unsigned char *progdata;
	unsigned char msgbuf[1024];

	int bufsize = RX62NProgramingInfo.UserMat[0].LastAddr - RX62NProgramingInfo.UserMat[0].StartAddr + 1;
	progdata = new unsigned char [bufsize]; // RX62Nでは524288になるはず
	unsigned long startaddr,lastaddr,entryaddr;

	startaddr = RX62NProgramingInfo.UserMat[0].StartAddr;
	lastaddr = RX62NProgramingInfo.UserMat[0].LastAddr;

	if(!load_mot_file(filename,progdata,bufsize,0xff,&startaddr,&lastaddr,&entryaddr,msgbuf,1024)) {
		print_message("ファイル%sの読み込みで失敗しました\n",filename);
		return false;
	}

	print_message("ベリファイ開始 ...\n");
	unsigned char buf[64];
	unsigned long len = lastaddr - startaddr;
	buf[0] = 0x52; // ユーザマット
	buf[1] = 0x09;
	buf[2] = 0x01; // ユーザマット
	buf[3] = startaddr >> 24;
	buf[4] = startaddr >> 16;
	buf[5] = startaddr >> 8;
	buf[6] = startaddr >> 0;
	buf[7] = len >> 24;
	buf[8] = len >> 16;
	buf[9] = len >> 8;
	buf[10] = len >> 0;
	buf[11] = calculate_sum(buf,11);

	ERR_STAT err = ERR_OK;
	int ret;
	ret = usb_bulk_write(dev, EP_OUT, buf, 12, 5000);

	ret = usb_bulk_read(dev, EP_IN, buf, 64, 5000);
	if(buf[0] == 0xD2) {
		if(buf[1] == 0x11) err = ERR_TXSUM;
		if(buf[1] == 0x2A) err = ERR_ADDR;
		if(buf[1] == 0x2B) err = ERR_DATALEN;
	}
	if(err != ERR_OK) {
		ShowErrorStat("データ読み出し",err);
		return false;
	}

	unsigned char *rbuf = new unsigned char [len];
    int p = 0;
	while(len)
    {
		s6w_progress(p,lastaddr - startaddr,"Verify");
    	int current_len = len;
        if(current_len > 4096) current_len = 4096;
		ret = usb_bulk_read(dev, EP_IN, &rbuf[p], current_len, 5000);
        len -= current_len;
        p   += current_len;
	}
	s6w_progress(0,0,"Verify");

	if(memcmp(rbuf,progdata,len))
	{
		err = ERR_VERIFY;
	}
	delete[] rbuf;
	ret = usb_bulk_read(dev, EP_IN, buf, 1, 5000);

	// 組み込みプログラムステータス問い合わせ
	if(RX62NQueryProgramStatus(NULL) != ERR_OK) return false;

	if(err != ERR_OK) {
		ShowErrorStat("データ読み出し",err);
		return false;
	}

	return true;
}

void dump(unsigned char *buf,int len,unsigned long addr)
{
	int i;
	for(i=0;i<len;i++)
	{
		if(((addr+i) & 15) == 0) print_message("%08X ",addr+i);
		print_message("%02X ",buf[i]);
		if(((addr+i) & 15) == 15) print_message("\n");
	}
}

#if 0
BOOL readback_rx62n(HANDLE hWrite,HANDLE hRead)
{
	unsigned char buf[1024+64];
	int ret;
	DWORD size;

	// データ読み出し
	buf[0] = 0x52;
	buf[1] = 0x09;
	buf[2] = 0x01; // ユーザマット
	buf[3] = 0xff;
	buf[4] = 0xf8;
	buf[5] = 0x00;
	buf[6] = 0x00;
	buf[7] = 0x00;
	buf[8] = 0x08;
	buf[9] = 0x00;
	buf[10] = 0x00;
	buf[11] = calculate_sum(buf,11);
	ret = WriteFile(hWrite, buf, 12, &size, NULL);

	ret = ReadFile(hRead, buf, 5, &size, NULL);

	show_result("リードバック",buf,size);

	int len = 524288;
	unsigned long addr=0xfff80000;
	unsigned char *rbuf = (unsigned char *)malloc(524288);
/*
	while(len)
	{
		int current=len;
		if(current >= 64) current = 64;
		ret = ReadFile(hRead, buf, current, &size, NULL);
		dump(buf,current,addr);
		len -= current;
		addr += current;
	}
*/
	ret = ReadFile(hRead, rbuf, 524288, &size, NULL);
 //   dump(rbuf,524288,0xfff80000);
	free(rbuf);

	ret = ReadFile(hRead, buf, 1, &size, NULL);
	return TRUE;
}
#endif

void RX62NDisconnect() {
	free(RX62NProgramingInfo.SupportDevName);
	RX62NProgramingInfo.ClockMultInfos.clear();
	RX62NProgramingInfo.ClockFreqs.clear();
	RX62NProgramingInfo.UserBootMat.clear();
	RX62NProgramingInfo.UserMat.clear();
	RX62NProgramingInfo.EraseBlocks.clear();

	if(dev) usb_close(dev);
	dev = NULL;
}

bool RX62NConnect() {
	// USBデバイスをオープンする
	if(!OpenLibUsb())
	{
//		print_message("RX62Nがみつかりません\n");
		return false;
	}

	if(setup_rx62n()) { // セットアップ成功
		print_message("--------------------------------------------------------\n",RX62NProgramingInfo.SupportDevCode);
		print_message("デバイスコード     \"%s\"\n",RX62NProgramingInfo.SupportDevCode);
		print_message("デバイス名         \"%s\"\n",RX62NProgramingInfo.SupportDevName);
		print_message("クロックモード     %d\n",RX62NProgramingInfo.ClockMode);
		print_message("クロック逓倍比情報 %d個\n",RX62NProgramingInfo.ClockMultInfos.size());
		AnsiString A;
		for(int i=0;i<RX62NProgramingInfo.ClockMultInfos.size();i++) {
			A.printf("                   %d種類 ",RX62NProgramingInfo.ClockMultInfos[i].size());
			for(int j=0;j<RX62NProgramingInfo.ClockMultInfos[i].size();j++) {
				A.cat_printf("%d倍 ",RX62NProgramingInfo.ClockMultInfos[i][j]);
			}
			print_message(A.c_str());
		}
		for(int i=0;i<RX62NProgramingInfo.ClockFreqs.size();i++) {
			A.printf("                   クロック(%d) ",i+1);
			A.cat_printf("最小周波数%5.2fMHz 最大周波数%5.2fMHz \n",
				RX62NProgramingInfo.ClockFreqs[i].MinFreq / 1000000.,
				RX62NProgramingInfo.ClockFreqs[i].MaxFreq / 1000000.);
			print_message(A.c_str());
		}
		print_message("ユーザブートマット %d種類\n",RX62NProgramingInfo.UserBootMat.size());
		for(int i=0;i<RX62NProgramingInfo.UserBootMat.size();i++) {
			A.printf("                   %d:%08X-%08X\n",i,
				RX62NProgramingInfo.UserBootMat[i].StartAddr,
				RX62NProgramingInfo.UserBootMat[i].LastAddr);
			print_message(A.c_str());
		}
		print_message("ユーザマット       %d種類\n",RX62NProgramingInfo.UserMat.size());
		for(int i=0;i<RX62NProgramingInfo.UserMat.size();i++) {
			A.printf("                   %d:%08X-%08X\n",i,
				RX62NProgramingInfo.UserMat[i].StartAddr,
				RX62NProgramingInfo.UserMat[i].LastAddr);
			print_message(A.c_str());
		}
		print_message("書き込みサイズ     %dバイト\n",RX62NProgramingInfo.WriteSize);
		print_message("消去ブロック       %d個\n",RX62NProgramingInfo.EraseBlocks.size());
/*
		for(int i=0;i<RX62NProgramingInfo.EraseBlocks.size();i++) {
			print_message("                   %d:%08X-%08X\n",i,
				RX62NProgramingInfo.EraseBlocks[i].StartAddr,
				RX62NProgramingInfo.EraseBlocks[i].LastAddr);
		}
*/
		print_message("--------------------------------------------------------\n",RX62NProgramingInfo.SupportDevCode);
		return true;
	}
	return false;
}
/*


//	hWrite = Uusbd_OpenPipe(husb, 0, 0); // パイプ0?
//	hRead  = Uusbd_OpenPipe(husb, 0, 1); // パイプ1?

	if(setup_rx62n()) { // セットアップ成功
		if(!strcmp(argv[1],"-s")) {
			return;
		}
	}

	if(!strcmp(argv[1],"-b"))
	{
		if(RX62NQueryBlankCheck()) {
			print_message("デバイスはブランクです\n");
		}
		else {
			print_message("デバイスはブランクではありません\n");
		}
		return;
	}

	if(!strcmp(argv[1],"-e"))
	{
		if(RX62NEraseUserMat()) {
			print_message("ユーザマットを消去しました。\n");
		}
		else {
			print_message("ユーザマットの消去で失敗しました。\n");
		}
		return;
	}

	if(!strcmp(argv[1],"-p"))
	{
		if(argc < 3) {
			print_message("ファイル名を指定してください。\n");
			return;
		}

		if(RX62NEraseUserMat()) {
			print_message("ユーザマットを消去しました。\n");
		}
		else {
			print_message("ユーザマットの消去で失敗しました。\n");
			return;
		}

		if(RX62NWriteUserMat(argv[2])) {
			print_message("書き込み完了。\n");
		}
		else {
			print_message("書き込み失敗。\n");
		}

		if(RX62NVerifyUserMat(argv[2])) {
			print_message("ベリファイ成功。\n");
		}
		else {
			print_message("ベリファイ失敗。\n");
			return;
		}
		return;
	}

	if(!strcmp(argv[1],"-v"))
	{
		if(argc < 3) {
			print_message("ファイル名を指定してください。\n");
			return;
		}

		if(RX62NVerifyUserMat(argv[2])) {
			print_message("ユーザマットをベリファイしました。\n");
		}
		else {
			print_message("ユーザマットのベリファイで失敗しました。\n");
			return;
		}
		return;
	}

	// クローズする
	usb_close(dev);
}
*/

