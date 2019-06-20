#include "multiplexPs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ES2PS.h"
#include "ES2TS.h"

struct h264FRAME_INFO
{
	int framelen;
	int frametype;
};
int get_idr_frame(unsigned char *buf,int pos,int &out_len,int file_len)
{
	int start_pos=pos;
	for(;pos<file_len-3;pos++){
		if ( (buf[pos] == 0x00 && 
			  buf[pos+1] == 0x00 && 
			  buf[pos+2] == 0x00 && 
			  buf[pos+3] == 0x01)){
			  int frame_type=(buf[pos+4] & 0x1F);
			  if(frame_type==1){
			  	out_len=pos-start_pos;					
				break;
			  }
		}
	}
	
	return 0;
}
int get_pps_frame(unsigned char *buf,int pos,int &out_len,int file_len)
{
	int start_pos=pos;
	for(;pos<file_len-3;pos++){
		if ( (buf[pos] == 0x00 && 
			  buf[pos+1] == 0x00 && 
			  buf[pos+2] == 0x00 && 
			  buf[pos+3] == 0x01)){
		  	out_len=pos-start_pos;					
			break;
		}
	}

	return 0;
}

int get_sps_frame(unsigned char *buf,int pos,int &out_len,int file_len)
{
	int start_pos=pos;
	for(;pos<file_len-3;pos++){
		if ( (buf[pos] == 0x00 && 
			  buf[pos+1] == 0x00 && 
			  buf[pos+2] == 0x00 && 
			  buf[pos+3] == 0x01)){
		  	out_len=pos-start_pos;					
			break;
		}
	}
	return 0;
}

int get_i_frame(unsigned char *buf,int pos,int &out_len,int file_len)
{
	int start_pos=pos;
	for(;pos<file_len-3;pos++){
		if ( (buf[pos] == 0x00 && 
			  buf[pos+1] == 0x00 && 
			  buf[pos+2] == 0x00 && 
			  buf[pos+3] == 0x01)){
		  	out_len=pos-start_pos;					
			break;
		}
	}


	return 0;
}

int get_p_frame(unsigned char *buf,int pos,int &out_len,int file_len)
{
	int start_pos=pos;
	for(;pos<file_len-3;pos++){
		if ( (buf[pos] == 0x00 && 
			  buf[pos+1] == 0x00 && 
			  buf[pos+2] == 0x00 && 
			  buf[pos+3] == 0x01)){
		  	out_len=pos-start_pos;					
			break;
		}
	}


	return 0;
}
int get_b_frame(unsigned char *buf,int pos,int &out_len,int file_len)
{
	int start_pos=pos;
	for(;pos<file_len-3;pos++){
		if ( (buf[pos] == 0x00 && 
			  buf[pos+1] == 0x00 && 
			  buf[pos+2] == 0x00 && 
			  buf[pos+3] == 0x01)){
		  	out_len=pos-start_pos;					
			break;
		}
	}
	return 0;
}
void generate_frame_index()
{

	struct h264FRAME_INFO frameinfo;
	FILE* fp_h264 = NULL;
	int ret = 0;

	fp_h264 = fopen("720p.h264", "rb");
	fseek(fp_h264, 0, SEEK_END);
	int size_h264=ftell(fp_h264);
	fprintf(stderr,"size_h264=%lf M \n",(size_h264*1.0)/(1048576*1.0));
	fseek(fp_h264, 0, SEEK_SET);
	unsigned char* ph264Buf = new unsigned char[size_h264];
	ret = fread(ph264Buf, 1, size_h264, fp_h264);

	FILE* fp_index = fopen("720p.index", "wb+");
	int last_pos = 0;
	int frame_count = 0;
	int frame_type = -1;
	int frame_len=0;
	int i=0;
	for (; i < size_h264 - 3 ; i++){
		if ( (ph264Buf[i] == 0x00 && ph264Buf[i+1] == 0x00	&& ph264Buf[i+2] == 0x00 && ph264Buf[i+3] == 0x01)){
			if (frame_type >= 0 && frame_len>0){
				frame_count++;
				frameinfo.frametype = frame_type;
				frameinfo.framelen = frame_len;
				fwrite(&frameinfo, sizeof(frameinfo), 1, fp_index);
				last_pos = i;
			}
			frame_type=(ph264Buf[i+4] & 0x1F);
			if(frame_type==7){
				get_idr_frame(ph264Buf,i+1,frame_len,size_h264);
				frame_type=IDR_FRAMES;
			}
			if(frame_type==1){
				get_p_frame(ph264Buf,i+1,frame_len,size_h264);
				frame_type=P_FRAME;
			}
			if(frame_type==6){
				get_b_frame(ph264Buf,i+1,frame_len,size_h264);
				frame_type=B_FRAME;
			}

			printf("frame len %d \n",frame_len);
			i+=frame_len;
		}
	}
	printf("i %d \n",i);
	frame_count++;
	frameinfo.frametype = frame_type;
	frameinfo.framelen = size_h264-last_pos;
	fwrite(&frameinfo, sizeof(frameinfo), 1, fp_index);

	delete []ph264Buf;
	fclose(fp_h264);
	fclose(fp_index);

	printf("Genrate frame index sucess, frame count[%d]!\n",frame_count);
}

void h264_es_2_ps_md()
{
	struct h264FRAME_INFO frameinfo;
	
	//!ES ת PS
	CES2PS es2ps;
	FILE* fp_h264 = NULL;
	int ret = 0;

	char* pBuf= new char[1024*1024];
	fp_h264 = fopen("720p.h264", "rb");
	FILE* fp_index = fopen("720p.index", "rb");

	if(!fp_h264){
		fprintf(stderr,"open file error \n");
		return;
	}
	FILE *fp_mpg=fopen("720p.mpg","wb");
	if(!fp_mpg){
		return;
	}
	int framecount = 0;
	while (1){
		ret = fread(&frameinfo, sizeof(frameinfo), 1, fp_index);
		if (ret <= 0){
			printf("read frame index over, total:%d\n", framecount);
			break;
		}
		framecount++;
		ret = fread(pBuf, 1, frameinfo.framelen, fp_h264);
		byte *out_buf=NULL;
		int out_len=0;
		
		es2ps.ES2PS((byte *)pBuf,frameinfo.framelen,out_buf,out_len,0,0,0);
		
		if(fp_mpg && out_buf && out_len>0){
			fwrite(out_buf, out_len, 1, fp_mpg);
			fflush(fp_mpg);
		}
	}

	fclose(fp_h264);
	fclose(fp_index);

	delete[] pBuf;
	printf("[%s][%d] \n",__FUNCTION__,__LINE__);

}
void h264_es_2_ts_md()
{
	struct h264FRAME_INFO frameinfo;
	
	//!ES ת PS
	CES2TS es2ts;
	FILE* fp_h264 = NULL;
	int ret = 0;

	char* pBuf= new char[1024*1024];
	fp_h264 = fopen("720p.h264", "rb");
	FILE* fp_index = fopen("720p.index", "rb");

	if(!fp_h264){
		fprintf(stderr,"open file error \n");
		return;
	}
	FILE *fp_mpg=fopen("720p.ts","wb");
	if(!fp_mpg){
		return;
	}
	int framecount = 0;
	while (1){
		ret = fread(&frameinfo, sizeof(frameinfo), 1, fp_index);
		if (ret <= 0){
			printf("read frame index over, total:%d\n", framecount);
			break;
		}
		framecount++;
		ret = fread(pBuf, 1, frameinfo.framelen, fp_h264);

		
		es2ts.InputESData((byte *)pBuf,frameinfo.framelen);
		
		byte *out_buf=NULL;
		int out_len=0;
		es2ts.GetTSData(out_buf,out_len);
		if(fp_mpg && out_buf && out_len>0){
			fwrite(out_buf, out_len, 1, fp_mpg);
			fflush(fp_mpg);
		}
	}

	fclose(fp_h264);
	fclose(fp_index);

	delete[] pBuf;
	printf("[%s][%d] \n",__FUNCTION__,__LINE__);

}

void h264_es_2_ps()
{
	struct h264FRAME_INFO frameinfo;
	//!es תps
	MultiplexPs muPs(NULL,NULL);
	FILE* fp_h264 = NULL;
	int ret = 0;

	char* pBuf= new char[1024*1024];
	fp_h264 = fopen("720p.h264", "rb");
	FILE* fp_index = fopen("720p.index", "rb");

	if(!fp_h264){
		fprintf(stderr,"open file error \n");
		return;
	}
	int framecount = 0;
	while (1){
		ret = fread(&frameinfo, sizeof(frameinfo), 1, fp_index);
		if (ret <= 0){
			printf("read frame index over, total:%d\n", framecount);
			break;
		}
		framecount++;
		ret = fread(pBuf, 1, frameinfo.framelen, fp_h264);
		muPs.gb28181AddFrame((char*)pBuf, frameinfo.framelen,frameinfo.frametype);
	}

	fclose(fp_h264);
	fclose(fp_index);

	delete[] pBuf;
}

int main(int argc,char *argv[])
{
	generate_frame_index();
	//h264_es_2_ps();
	//h264_es_2_ps_md();
	h264_es_2_ts_md();
	return 0;
}
