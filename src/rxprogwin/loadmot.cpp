#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static unsigned long hextoint(const char *str)
{
	unsigned long val = 0;
	while(*str)
	{
		val <<= 4;
		if((*str >= '0') && (*str <= '9')) val |= (*str - '0');
		if((*str >= 'A') && (*str <= 'F')) val |= (*str - 'A' + 10);
		if((*str >= 'a') && (*str <= 'f')) val |= (*str - 'a' + 10);
		str++;
	}
	return val;
}

static unsigned char getb(char *src)
{
	unsigned char a=0;
	char tmp[3];
	if(!tmp) return 0;
	tmp[0] = src[0];
	tmp[1] = src[1];
	tmp[2] = '\0';
	return hextoint(tmp);
}

static void trim(char *buffer)
{
	unsigned int w_ptr,r_ptr;
	unsigned int buflen;
	if(!buffer) return;
	if(!buffer[0]) return;

    for(w_ptr = strlen(buffer)-1 ; w_ptr != 0 ;w_ptr--)
    {
		if(iscntrl(buffer[w_ptr]) || isspace(buffer[w_ptr]))
		{
			buffer[w_ptr] = '\0';
		}
		else
		{
			break;
		}
    }

    for(r_ptr=0;r_ptr<strlen(buffer);r_ptr++)
    {
		//先頭の表示可能文字を探す
		if(isprint(buffer[r_ptr]) && !isspace(buffer[r_ptr]))
			break;
	}

	w_ptr = 0;
	buflen = strlen(buffer);
	while(r_ptr < buflen)
	{
		buffer[w_ptr] = buffer[r_ptr];
		if(buffer[r_ptr] == '\0')
		{
			break;
		}
		w_ptr++;
		r_ptr++;
    }
	buffer[w_ptr] = '\0';
}

bool load_mot_file(const char *filename,
				   unsigned char *buffer,
				   unsigned int buffer_size,
				   unsigned char default_padding,
				   unsigned long *pstartaddr,
				   unsigned long *plastaddr,
				   unsigned long *pentryaddr,
				   char *message_buf,
				   unsigned long message_bufsize)
{
	char tmp[256];
	unsigned long rwaddr;
	unsigned char len,ext,ctemp;
	unsigned long lastaddr=0;
	unsigned long startaddr;
	unsigned long entryaddr;
	unsigned long offset;
	char *tmpbuf;
	int i,p;
	char code;
	unsigned char csum;
	int msgpos;

	FILE *fp = fopen(filename,"rt");

	msgpos = 0;
	bool state = true;
	startaddr = 0xffffffff;	//ファイルに記述されている先頭アドレス

	if(fp == NULL){
		return false; // file not found
	}

	for(i=0;i<buffer_size;i++)
	{
		buffer[i] = default_padding;
	}

	if(pstartaddr) offset = *pstartaddr;
	else offset = 0;

	while(fgets(tmp,255,fp))
	{
		trim(tmp);
		if(tmp[0] != 'S') continue;
		if(tmp[1]<'0' || tmp[1]>'9') continue;
		code = tmp[1] - '0';
		csum = 0;
		len  = getb(tmp+2);
		switch(code){
			case 0:
				tmpbuf = (char *)malloc(len + 1);
				for(i=0;i<len-2;i++)
				{
					if(msgpos >= message_bufsize-1) message_buf[msgpos] = '\0';
					else if(message_buf)
					{
						message_buf[msgpos] = getb(tmp+8+i*2);
						msgpos++;
					}
				}
				if(msgpos >= message_bufsize-1) message_buf[msgpos] = '\0';
				else if(message_buf)
				{
					message_buf[msgpos] = ' ';
					msgpos++;
				}

				p=0;
				break;
			case 1:
				rwaddr = (getb(tmp+4)<<8) + getb(tmp+6);
				csum += getb(tmp+4) + getb(tmp+6);
				p=8;
				break;
			case 2:
				rwaddr = (getb(tmp+4)<<16) + (getb(tmp+6)<<8) + getb(tmp+8);
				csum += getb(tmp+4) + getb(tmp+6) + getb(tmp+8);
				p=10;
				break;
			case 3:
				rwaddr = (getb(tmp+4)<<24) + (getb(tmp+6)<<16) + (getb(tmp+8)<<8) + getb(tmp+10);
				csum += getb(tmp+4) + getb(tmp+6) + getb(tmp+8) + getb(tmp+10);
				p=12;
				break;

			case 9:
				entryaddr = (getb(tmp+4)<<8) + getb(tmp+6);
				csum += getb(tmp+4) + getb(tmp+6);
				p = 0;
				break;
			case 8:
				entryaddr = (getb(tmp+4)<<16) + (getb(tmp+6)<<8) + getb(tmp+8);
				csum += getb(tmp+4) + getb(tmp+6) + getb(tmp+8);
				p = 0;
				break;
			case 7:
				entryaddr = (getb(tmp+4)<<24) + (getb(tmp+6)<<16) + (getb(tmp+8)<<8) + getb(tmp+10);
				csum += getb(tmp+4) + getb(tmp+6) + getb(tmp+8) + getb(tmp+10);
				p = 0;
				break;
			default:
				p = 0;
		}

		if(p==0) continue;

		for(i=0;i<len-2-code;i++){
			ctemp = getb(tmp + p + i*2);
			csum += getb(tmp + p + i*2);
			if(rwaddr-offset >= buffer_size)
			{
				//アドレスがオーバーフローしたら
				state = false;
			}
			else
			{
				//アドレスがオーバーフローしなければ
				buffer[rwaddr-offset] = ctemp;
			}

			//書き込んだバイト数計算のためにMaxAddrを調べる
			if(rwaddr >= lastaddr)  lastaddr = rwaddr;
			if(rwaddr <= startaddr) startaddr = rwaddr;
			rwaddr++;
		}
		getb(tmp + p + i*2);	//チェックサムを読む
		csum++;					//1の補数だから1を足す
								//チェックサムが0になってなければエラー^
//		if(csum) state = false;
	}

	fclose(fp);

	if(pstartaddr) *pstartaddr = startaddr;
	if(plastaddr)  *plastaddr  = lastaddr;
	if(pentryaddr) *pentryaddr = entryaddr;

	return state;
}
