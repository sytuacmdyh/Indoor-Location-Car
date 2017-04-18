#include "vs10xx.h"
#include "exfuns.h"
#include "mp3player.h"
#include "ff.h"
#include "usart.h"

#define buf_lenth 128

char song_path[100];

u8 mp3_playing=0;
FIL *fmp3,mp3;
u8 res;
u8 databuf[buf_lenth+5];
u32 i;

void mp3_init(void)
{
	song_path[0]=0;
	fmp3=&mp3;
}

void mp3_play(void)
{
	if(song_path[0]!=0)
	{
		//先停止上一次播放
		mp3_playing=0;
		i=0;
		if(fmp3)
			f_close(fmp3);
		
		VS_Restart_Play();  					//重启播放
		VS_Set_All();        					//设置音量等信息
		VS_Reset_DecodeTime();					//复位解码时间
		res=f_open(fmp3,(const TCHAR*)song_path,FA_READ);//打开文件
		if(res==0)//打开成功
		{
			mp3_playing=1;
		}
		else{
			mp3_playing=0;
		}
		song_path[0]=0;
	}
	
	if(mp3_playing)
	{	
		res=f_read(fmp3,databuf,buf_lenth,(UINT*)&br);//读出4096个字节
		//printf("res:%d\n",res);
		i=0;
		while(i<br)//给VS10XX发送音频数据
		{
			if(VS_Send_MusicData(databuf+i)==0)
				i+=32;
		}
		if(br!=buf_lenth||res!=0)//读完了.
		{
			mp3_playing=0;
		}
	}
}
