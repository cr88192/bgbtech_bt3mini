typedef struct BGBCC_BITMAPINFOHEADER_s BGBCC_BITMAPINFOHEADER;

struct BGBCC_BITMAPINFOHEADER_s {
	u32	biSize;
	s32	biWidth;
	s32	biHeight;
	u16	biPlanes;
	u16	biBitCount;
	u32	biCompression;
	u32	biSizeImage;
	s32	biXPelsPerMeter;
	s32	biYPelsPerMeter;
	u32	biClrUsed;
	u32	biClrImportant;
};

/* Load 4 or 8 bpp BMP image, decoded as 8bpp */
byte *BTM_LoadBmpIndex8(byte *imgbuf, byte *opal, int *rxs, int *rys)
{
	BGBCC_BITMAPINFOHEADER *bmi;
	byte t_pal[1024];
	byte *dat, *cs, *pal;
	byte *buf, *ct;
	int ofs_bmi;
	int ofs_dat;
	int xstr, bc, bc1, xs, ys, flip;
	int p0, p1, p2, p3;
	int cr, cg, cb, ca;
	int x, y;
	int i, j, k;

	ofs_bmi=0;

	if((imgbuf[0]=='B') && (imgbuf[1]=='M'))
	{
		ofs_bmi=0x0E;
		ofs_dat=
			(imgbuf[0x0A]<< 0) | (imgbuf[0x0B]<< 8) |
			(imgbuf[0x0C]<<16) | (imgbuf[0x0D]<<24) ;
	}

	if(!ofs_bmi)
		return(NULL);
	
	bmi=(BGBCC_BITMAPINFOHEADER *)(imgbuf+ofs_bmi);
	dat=(byte *)(imgbuf+ofs_dat);
	pal=(byte *)(imgbuf+ofs_bmi+bmi->biSize);
	
	if(bmi->biCompression!=0)
	{
		printf("BTM_LoadBmpIndex8: Unsupported biCompression %08X\n",
			bmi->biCompression);
		return(NULL);
	}

	bc=bmi->biBitCount;
	if((bc!=4) && (bc!=8))
	{
		printf("BTM_LoadBmpIndex8: Unsupported biBitCount %d\n", bc);
		return(NULL);
	}
	
	k=(1<<bc);
	for(i=0; i<k; i++)
	{
		opal[i*4+0]=pal[i*4+2];
		opal[i*4+1]=pal[i*4+1];
		opal[i*4+2]=pal[i*4+0];
		opal[i*4+3]=pal[i*4+3];
	}

	xstr=(((bmi->biWidth*bc)+31)&(~31))>>3;

	xs=bmi->biWidth;
	ys=bmi->biHeight;
	flip=0;
	
	if(ys<0)
	{
		ys=-ys;
		flip=!flip;
	}
	
	buf=btm_malloc(xs*ys);
	
	if(bc==8)
	{
		for(y=0; y<ys; y++)
		{
			ct=buf+(y*xs);
			cs=dat+(y*xstr);
			if(flip)
				cs=buf+((ys-y-1)*xstr);
			memcpy(ct, cs, xs);
		}
	}
	
	if(bc==4)
	{
		for(y=0; y<ys; y++)
		{
			ct=buf+(y*xs);
			cs=dat+(y*xstr);
			if(flip)
				cs=buf+((ys-y-1)*xstr);
			for(x=0; x<xs; x+=2)
			{
				k=cs[x>>1];
				ct[x+0]=(k>>4)&15;
				ct[x+1]=(k>>0)&15;
			}
		}
	}
	
	*rxs=xs;
	*rys=ys;
	return(buf);
}
