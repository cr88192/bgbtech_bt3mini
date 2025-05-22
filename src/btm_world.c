/*
Copyright (C) 2022  Brendan G Bohannon

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*
Voxel:
  (31:24): Attrib / Occlusion
  (23:20): Light, Sky
  (19:12): Light Level (Block)
  (11: 8): Attrib / Ext-Type
  ( 7: 0): Block Type

Pos32:
  (31:24): ZPos, 8
  (23:12): YPos, 12
  (11: 0): XPos, 12

Pos64:
  (63:48): ZPos, 8.8
  (47:24): YPos, 16.8
  (23: 0): XPos, 16.8
 */

#include "btm_engine.h"

u64	btm_step_ydir[1024];
u64	btm_step_pdir[1024];
int	btm_step_init=0;

u32	btmra_rcptab[512];
// s32 btmra_cosang[256];
s32 btmra_sincosang[256+64];
s32 *btmra_sinang;
s32 *btmra_cosang;

float btmra_sincosang_f[256+64];
float *btmra_sinang_f;
float *btmra_cosang_f;

u16 btm_pmort2_enc8[256];	// 8->16 bit encode
byte btm_pmort2_dec8[256];	// 8->4 bit decode

u32 btm_pmort3_enc8[256];	// 8->24 bit encode
byte btm_pmort3_dec12[4096];	// 12->4 bit decode


int btm_stat_malloc_tot;
int btm_stat_malloc_vatot;
void * volatile btm_malloc_mutex;

char *btm_malloc_lfnix_name[256];
int btm_malloc_n_lfnix;

void *btm_malloc_chn;

int BTMRA_RastInitTables()
{
	double f, g;
	int i, j, k;
	
	for(i=1; i<512; i++)
	{
		btmra_rcptab[i]=(0x7FFFFFFFU/i)*2;
	}

	for(i=0; i<(256+64); i++)
	{
		f=i*((2*M_PI)/256);
//		g=cos(f);
		g=sin(f);
		j=g*0x7FFFFFFF;
//		btmra_cosang[i]=j;
		btmra_sincosang[i]=j;
		btmra_sincosang_f[i]=g;
	}
	
	btmra_sinang=btmra_sincosang+ 0;
	btmra_cosang=btmra_sincosang+64;

	btmra_sinang_f=btmra_sincosang_f+ 0;
	btmra_cosang_f=btmra_sincosang_f+64;

	return(0);
}

int BTM_RaycastInitTables()
{
	u64 v;
	double f, g, fx, fy;
	int ix, iy;
	int i, j, k;
	
	if(btm_step_init)
		return(0);
	btm_step_init=1;
	
	for(i=0; i<1024; i++)
	{
		f=i*((2*M_PI)/1024);
		fx=sin(f);
		fy=-cos(f);
		
		ix=fx*256;
		iy=fy*256;
		ix&=0xFFFFFF;
		iy&=0xFFFFFF;
		
		v=(((u64)iy)<<24)|(ix);
		btm_step_ydir[i]=v;
	}
	
	for(i=0; i<1024; i++)
	{
		f=i*((2*M_PI)/1024);
		fx=sin(f);
		fy=cos(f);
		ix=fx*256;

		iy=fy*256;
//		iy=fabs(fy)*256;
//		iy=(1.0-fabs(fx))*256;
		iy&=0x00FFFFFF;

		ix&=0x0000FFFF;
		v=(((u64)ix)<<48)|(((u64)iy)<<24)|(iy);
		btm_step_pdir[i]=v;
	}
	
	BTMRA_RastInitTables();
	
	for(i=0; i<256; i++)
	{
		j=0;
		if(i&  1)j|=0x0001;
		if(i&  2)j|=0x0004;
		if(i&  4)j|=0x0010;
		if(i&  8)j|=0x0040;
		if(i& 16)j|=0x0100;
		if(i& 32)j|=0x0400;
		if(i& 64)j|=0x1000;
		if(i&128)j|=0x4000;
		btm_pmort2_enc8[i]=j;
		
		k=0;
		if(i&0x01)k|=1;
		if(i&0x04)k|=2;
		if(i&0x10)k|=4;
		if(i&0x40)k|=8;
		btm_pmort2_dec8[i]=k;


		j=0;
		if(i&  1)j|=0x000001;
		if(i&  2)j|=0x000008;
		if(i&  4)j|=0x000040;
		if(i&  8)j|=0x000200;
		if(i& 16)j|=0x001000;
		if(i& 32)j|=0x008000;
		if(i& 64)j|=0x040000;
		if(i&128)j|=0x200000;
		btm_pmort3_enc8[i]=j;
	}

	for(i=0; i<4096; i++)
	{
		k=0;
		if(i&0x001)k|=1;
		if(i&0x008)k|=2;
		if(i&0x040)k|=4;
		if(i&0x200)k|=8;
		btm_pmort3_dec12[i]=k;
	}

	return(1);
}

int BTM_LockMalloc()
{
	if(!btm_malloc_mutex)
		btm_malloc_mutex=thMutex();
	thLockMutex(btm_malloc_mutex);
	return(0);
}

int BTM_UnlockMalloc()
{
	thUnlockMutex(btm_malloc_mutex);
	return(0);
}

int btm_malloc_lfnixforlfn(char *lfn)
{
	int i;
	
	for(i=0; i<btm_malloc_n_lfnix; i++)
		if(!strcmp(btm_malloc_lfnix_name[i], lfn))
			return(i);

	if(i>=btm_malloc_n_lfnix)
		return(-1);

	i=btm_malloc_n_lfnix++;
	btm_malloc_lfnix_name[i]=strdup(lfn);
	return(i);
}


void *btm_malloc_lln(int sz, char *lfn, int lln)
{
	void *ptr;

	BTM_LockMalloc();
	printf("malloc: %d %s:%d tot=%d\n", sz, lfn, lln, btm_stat_malloc_tot);
	btm_stat_malloc_tot+=sz;

//	ptr=malloc(sz+4*sizeof(void *));
	ptr=malloc(sz);
	
	BTM_UnlockMalloc();

	return(ptr);
}

void *btm_realloc_lln(void *ptr, int sz, char *lfn, int lln)
{
	void *ptr1;

	BTM_LockMalloc();
	printf("realloc: %d %s:%d\n", sz, lfn, lln);
	ptr1=realloc(ptr, sz);
	BTM_UnlockMalloc();

	return(ptr1);
}

void *btm_malloc_va_lln(int sz, char *lfn, int lln)
{
	void *ptr;

	BTM_LockMalloc();
	printf("malloc_va: %d %s:%d tot=%d\n", sz, lfn, lln, btm_stat_malloc_tot);
	btm_stat_malloc_vatot+=sz;

//	ptr=malloc(sz+4*sizeof(void *));
	ptr=malloc(sz);
	
	BTM_UnlockMalloc();

	return(ptr);
}

void *btm_realloc_va_lln(void *ptr, int sz, char *lfn, int lln)
{
	void *ptr1;

	BTM_LockMalloc();
	printf("realloc_va: %d %s:%d\n", sz, lfn, lln);
	ptr1=realloc(ptr, sz);
	BTM_UnlockMalloc();

	return(ptr1);
}

void btm_free(void *ptr)
{
	free(ptr);
}

u64 BTM_RaycastDeltaVector(u64 spos, u64 epos)
{
	u64 step;
	int spx, spy, spz;
	int epx, epy, epz;
	int sx, sy, sz, adx, ady, adz, adsq;
	
	spx=((spos>> 0)&0xFFFFFF);
	spy=((spos>>24)&0xFFFFFF);
	spz=((spos>>48)&0x00FFFF);
	epx=((epos>> 0)&0xFFFFFF);
	epy=((epos>>24)&0xFFFFFF);
	epz=((epos>>48)&0x00FFFF);
	
	sx=epx-spx;
	sy=epy-spy;
	sz=epz-spz;
	adx=sx^(sx>>31);
	ady=sy^(sy>>31);
	adz=sz^(sz>>31);
	
	while((adx|ady|adx)>=512)
	{
		sx>>=1; sy>>=1; sz>>=1;
		adx=sx^(sx>>31);
		ady=sy^(sy>>31);
		adz=sz^(sz>>31);
	}
	
	adsq=(adx*adx)+(ady*ady)+(adz*adz);
	while(adsq>262144)
	{
		sx=(sx*7)/8;
		sy=(sy*7)/8;
		sz=(sz*7)/8;
		adx=sx^(sx>>31);
		ady=sy^(sy>>31);
		adz=sz^(sz>>31);
		adsq=(adx*adx)+(ady*ady)+(adz*adz);
	}

	adsq=(adx*adx)+(ady*ady)+(adz*adz);
	while(adsq>65536)
	{
		sx=(sx*31)/32;
		sy=(sy*31)/32;
		sz=(sz*31)/32;
		adx=sx^(sx>>31);
		ady=sy^(sy>>31);
		adz=sz^(sz>>31);
		adsq=(adx*adx)+(ady*ady)+(adz*adz);
	}

	sx&=0x00FFFFFF;
	sy&=0x00FFFFFF;

	step=(((u64)sz)<<48)|(((u64)sy)<<24)|(sx);
	return(step);
}

u64 BTM_RaycastStepVectorB(int yang, int pang)
{
	float sy, cy, sp, cp, la, lb;
	float fx, fy, fz, fd;
	int vx, vy, vz, sx;
	u64 ystep, pstep, step;

#if 1
	ystep=btm_step_ydir[yang&1023];
	pstep=btm_step_pdir[pang&1023];
	
	vx=(ystep>> 0);
	vy=(ystep>>24);
	vx=((int)(vx<<8))>>8;
	vy=((int)(vy<<8))>>8;

	sx=(pstep>> 0);
	sx=((int)(sx<<8))>>8;
	vz=(pstep>>48);
	
	vx=(vx*sx)>>8;
	vy=(vy*sx)>>8;
	
	vx&=0x00FFFFFF;
	vy&=0x00FFFFFF;

	step=(((u64)vz)<<48)|(((u64)vy)<<24)|(vx);
	return(step);
#endif
}

// #define BTM_RCIX_MORTON

u64 BTM_RaycastStepVector(int yang, int pang)
{
	return(BTM_RaycastStepVectorB(yang*4, pang*4));
}

u64 BTM_BlockCoordsToRcix(int cx, int cy, int cz)
{
	u64 cix, bix, hix, rix, rcix;
	int rx, ry, hx, hy, hz, bx, by, bz;

	if(cz<  0)	cz=  0;
	if(cz>127)	cz=127;

#ifdef BTM_RCIX_MORTON
	rx=(cx>>7)&511;
	ry=(cy>>7)&511;
//	rix=(ry<<9)|rx;

	rix =(btm_pmort2_enc8[rx&255]   );
	rix|=(btm_pmort2_enc8[ry&255]<<1);
	if(rx&256)rix|=0x10000;
	if(ry&256)rix|=0x20000;

	cix =(btm_pmort3_enc8[cx&127]   );
	cix|=(btm_pmort3_enc8[cy&127]<<1);
	cix|=(btm_pmort3_enc8[cz&127]<<2);

	rcix=(rix<<21)|cix;
	return(rcix);
#else

//	rx=(cx>>7)&255;
//	ry=(cy>>7)&255;
//	rix=(ry<<8)|rx;

	rx=(cx>>7)&511;
	ry=(cy>>7)&511;
	rix=(ry<<9)|rx;

//	cx&=127;
//	cy&=127;
//	cz&=127;

	hx=(cx>>4)&7;
	hy=(cy>>4)&7;
	hz=(cz>>4)&7;
	bx=cx&15;
	by=cy&15;
	bz=cz&15;

	hix=(hz<<6)|(hy<<3)|hx;
	bix=(bz<<8)|(by<<4)|bx;
	cix=(hix<<12)|bix;

//	cix=(cz<<14)|(cy<<7)|cx;
//	rcix=(rix<<24)|cix;
	rcix=(rix<<21)|cix;
	return(rcix);
#endif
}


static const u16 btm_corg2rcix_spreadlut_x[128]={
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
	0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
	0x1000, 0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007,
	0x1008, 0x1009, 0x100A, 0x100B, 0x100C, 0x100D, 0x100E, 0x100F,
	0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007,
	0x2008, 0x2009, 0x200A, 0x200B, 0x200C, 0x200D, 0x200E, 0x200F,
	0x3000, 0x3001, 0x3002, 0x3003, 0x3004, 0x3005, 0x3006, 0x3007,
	0x3008, 0x3009, 0x300A, 0x300B, 0x300C, 0x300D, 0x300E, 0x300F,
	0x4000, 0x4001, 0x4002, 0x4003, 0x4004, 0x4005, 0x4006, 0x4007,
	0x4008, 0x4009, 0x400A, 0x400B, 0x400C, 0x400D, 0x400E, 0x400F,
	0x5000, 0x5001, 0x5002, 0x5003, 0x5004, 0x5005, 0x5006, 0x5007,
	0x5008, 0x5009, 0x500A, 0x500B, 0x500C, 0x500D, 0x500E, 0x500F,
	0x6000, 0x6001, 0x6002, 0x6003, 0x6004, 0x6005, 0x6006, 0x6007,
	0x6008, 0x6009, 0x600A, 0x600B, 0x600C, 0x600D, 0x600E, 0x600F,
	0x7000, 0x7001, 0x7002, 0x7003, 0x7004, 0x7005, 0x7006, 0x7007,
	0x7008, 0x7009, 0x700A, 0x700B, 0x700C, 0x700D, 0x700E, 0x700F
};

static const u32 btm_corg2rcix_spreadlut_y[128]={
	0x000000, 0x000010, 0x000020, 0x000030,
	0x000040, 0x000050, 0x000060, 0x000070,
	0x000080, 0x000090, 0x0000A0, 0x0000B0,
	0x0000C0, 0x0000D0, 0x0000E0, 0x0000F0,
	0x008000, 0x008010, 0x008020, 0x008030,
	0x008040, 0x008050, 0x008060, 0x008070,
	0x008080, 0x008090, 0x0080A0, 0x0080B0,
	0x0080C0, 0x0080D0, 0x0080E0, 0x0080F0,
	0x010000, 0x010010, 0x010020, 0x010030,
	0x010040, 0x010050, 0x010060, 0x010070,
	0x010080, 0x010090, 0x0100A0, 0x0100B0,
	0x0100C0, 0x0100D0, 0x0100E0, 0x0100F0,
	0x018000, 0x018010, 0x018020, 0x018030,
	0x018040, 0x018050, 0x018060, 0x018070,
	0x018080, 0x018090, 0x0180A0, 0x0180B0,
	0x0180C0, 0x0180D0, 0x0180E0, 0x0180F0,
	0x020000, 0x020010, 0x020020, 0x020030,
	0x020040, 0x020050, 0x020060, 0x020070,
	0x020080, 0x020090, 0x0200A0, 0x0200B0,
	0x0200C0, 0x0200D0, 0x0200E0, 0x0200F0,
	0x028000, 0x028010, 0x028020, 0x028030,
	0x028040, 0x028050, 0x028060, 0x028070,
	0x028080, 0x028090, 0x0280A0, 0x0280B0,
	0x0280C0, 0x0280D0, 0x0280E0, 0x0280F0,
	0x030000, 0x030010, 0x030020, 0x030030,
	0x030040, 0x030050, 0x030060, 0x030070,
	0x030080, 0x030090, 0x0300A0, 0x0300B0,
	0x0300C0, 0x0300D0, 0x0300E0, 0x0300F0,
	0x038000, 0x038010, 0x038020, 0x038030,
	0x038040, 0x038050, 0x038060, 0x038070,
	0x038080, 0x038090, 0x0380A0, 0x0380B0,
	0x0380C0, 0x0380D0, 0x0380E0, 0x0380F0
};

static const u32 btm_corg2rcix_spreadlut_z[128]={
	0x000000, 0x000100, 0x000200, 0x000300,
	0x000400, 0x000500, 0x000600, 0x000700,
	0x000800, 0x000900, 0x000A00, 0x000B00,
	0x000C00, 0x000D00, 0x000E00, 0x000F00,
	0x040000, 0x040100, 0x040200, 0x040300,
	0x040400, 0x040500, 0x040600, 0x040700,
	0x040800, 0x040900, 0x040A00, 0x040B00,
	0x040C00, 0x040D00, 0x040E00, 0x040F00,
	0x080000, 0x080100, 0x080200, 0x080300,
	0x080400, 0x080500, 0x080600, 0x080700,
	0x080800, 0x080900, 0x080A00, 0x080B00,
	0x080C00, 0x080D00, 0x080E00, 0x080F00,
	0x0C0000, 0x0C0100, 0x0C0200, 0x0C0300,
	0x0C0400, 0x0C0500, 0x0C0600, 0x0C0700,
	0x0C0800, 0x0C0900, 0x0C0A00, 0x0C0B00,
	0x0C0C00, 0x0C0D00, 0x0C0E00, 0x0C0F00,
	0x100000, 0x100100, 0x100200, 0x100300,
	0x100400, 0x100500, 0x100600, 0x100700,
	0x100800, 0x100900, 0x100A00, 0x100B00,
	0x100C00, 0x100D00, 0x100E00, 0x100F00,
	0x140000, 0x140100, 0x140200, 0x140300,
	0x140400, 0x140500, 0x140600, 0x140700,
	0x140800, 0x140900, 0x140A00, 0x140B00,
	0x140C00, 0x140D00, 0x140E00, 0x140F00,
	0x180000, 0x180100, 0x180200, 0x180300,
	0x180400, 0x180500, 0x180600, 0x180700,
	0x180800, 0x180900, 0x180A00, 0x180B00,
	0x180C00, 0x180D00, 0x180E00, 0x180F00,
	0x1C0000, 0x1C0100, 0x1C0200, 0x1C0300,
	0x1C0400, 0x1C0500, 0x1C0600, 0x1C0700,
	0x1C0800, 0x1C0900, 0x1C0A00, 0x1C0B00,
	0x1C0C00, 0x1C0D00, 0x1C0E00, 0x1C0F00,
};


u64 BTM_WorldCorgToRcix(u64 cpos)
{
#ifdef BTM_RCIX_MORTON
	int cx, cy, cz;

	cx=(cpos>> 8)&65535;
	cy=(cpos>>32)&65535;
	cz=(cpos>>56)&255;
	return(BTM_BlockCoordsToRcix(cx, cy, cz));

#else
	u64 cix, bix, hix, rix, rcix;
	int rx, ry, hx, hy, hz, bx, by, bz;
	int cx, cy, cz;

	cx=(cpos>> 8)&65535;
	cy=(cpos>>32)&65535;
	cz=(cpos>>56)&255;

//	if(cz<  0)	cz=  0;
//	if(cz>127)	cz=127;

	rx=(cx>>7)&511;
	ry=(cy>>7)&511;
	rix=(ry<<9)|rx;

#if 0
	hx=(cx>>4)&7;
	hy=(cy>>4)&7;
	hz=(cz>>4)&7;
	bx=cx&15;
	by=cy&15;
	bz=cz&15;

	hix=(hz<<6)|(hy<<3)|hx;
	bix=(bz<<8)|(by<<4)|bx;
	cix=(hix<<12)|bix;
#endif

	bx=btm_corg2rcix_spreadlut_x[cx&127];
	by=btm_corg2rcix_spreadlut_y[cy&127];
	bz=btm_corg2rcix_spreadlut_z[cz&127];
	cix=bz|by|bx;

	rcix=(rix<<21)|cix;
	return(rcix);


//	return(BTM_BlockCoordsToRcix(cx, cy, cz));
#endif
}

int BTM_WorldCorgToRix(u64 spos)
{
	int cx, cy, cz, rx, ry, rix;

	cx=spos>> 8;
	cy=spos>>32;
//	rx=(cx>>7)&255;
//	ry=(cy>>7)&255;
//	rix=(ry<<8)|rx;

#ifdef BTM_RCIX_MORTON
	rx=(cx>>7)&511;
	ry=(cy>>7)&511;
	rix =(btm_pmort2_enc8[rx&255]   );
	rix|=(btm_pmort2_enc8[ry&255]<<1);
	if(rx&256)rix|=0x10000;
	if(ry&256)rix|=0x20000;
#else
	rx=(cx>>7)&511;
	ry=(cy>>7)&511;
	rix=(ry<<9)|rx;
#endif
	
	return(rix);
}

u64 BTM_ConvRcixToBlkPos(u64 rcix)
{
	u64 bpos;
	int bx, by, bz;
	int hx, hy, hz;
	int rx, ry, rix;
	int cx, cy, cz;
	
#ifdef BTM_RCIX_MORTON
	rix=BTM_Rcix2Rix(rcix);
	rx=	(btm_pmort2_dec8[(rix    )&255]   )|
		(btm_pmort2_dec8[(rix>> 8)&255]<<4)|
		(((rix>>16)&1)<<8);
	ry=	(btm_pmort2_dec8[(rix>> 1)&255]   )|
		(btm_pmort2_dec8[(rix>> 9)&255]<<4)|
		(((rix>>17)&1)<<8);
		
	bx=btm_pmort3_dec12[(rcix>>0)&1023];
	by=btm_pmort3_dec12[(rcix>>1)&1023];
	bz=btm_pmort3_dec12[(rcix>>2)&1023];

	hx=btm_pmort3_dec12[(rcix>>12)&255];
	hy=btm_pmort3_dec12[(rcix>>13)&255];
	hz=btm_pmort3_dec12[(rcix>>14)&255];

	cx=(rx<<7)|(hx<<4)|bx;
	cy=(ry<<7)|(hy<<4)|by;
	cz=(hz<<4)|bz;
#else
	rix=BTM_Rcix2Rix(rcix);
	bx=(rcix>> 0)&15;	by=(rcix>> 4)&15;	bz=(rcix>> 8)&15;
	hx=(rcix>>12)&7;	hy=(rcix>>15)&7;	hz=(rcix>>18)&7;
//	rx=(rix>>0)&255;	ry=(rix>>8)&255;
	rx=(rix>>0)&511;	ry=(rix>>9)&511;
	
	cx=(rx<<7)|(hx<<4)|bx;
	cy=(ry<<7)|(hy<<4)|by;
	cz=(hz<<4)|bz;
#endif

	bpos=(((u64)cz)<<32)|(((u64)cy)<<16)|(((u64)cx)<<0);
	return(bpos);
}

u64 BTM_ConvRixToBlkPos(int rix)
{
	return(BTM_ConvRcixToBlkPos(((u64)rix)<<21));
}

u64 BTM_ConvRcixToCorg(u64 rcix)
{
	u64 bpos, cpos;
	int cx, cy, cz;
	
	bpos=BTM_ConvRcixToBlkPos(rcix);
	cx=(bpos>> 0)&0xFFFF;
	cy=(bpos>>16)&0xFFFF;
	cz=(bpos>>32)&0x00FF;
	
	cpos=(((u64)cx)<<8)|(((u64)cy)<<32)|(((u64)cz)<<56);
	return(cpos);
}

u64 BTM_ConvRixToBlkPosCenter(int rix)
{
	u64 cix;
	cix=BTM_BlockCoordsToRcix(64, 64, 64);;
	return(BTM_ConvRcixToBlkPos((((u64)rix)<<21)|cix));
}

u64 BTM_BlockOffsetRcix(u64 rcix, int dx, int dy, int dz)
{
	u64 bpos, rcix1, rcix2, rcix3;
	int cx, cy, cz;

#ifndef BTM_RCIX_MORTON
	rcix1=rcix+dx;
	rcix2=rcix1+(dy<<4);
	rcix3=rcix2+(dz<<8);
	if(	((rcix1&(~  15))==(rcix&(~  15))) &&
		((rcix2&(~ 255))==(rcix&(~ 255))) &&
		((rcix3&(~4095))==(rcix&(~4095))) )
	{
		return(rcix3);
	}
#endif


	bpos=BTM_ConvRcixToBlkPos(rcix);
	cx=(bpos>>0)&65535;
	cy=(bpos>>16)&65535;
	cz=(bpos>>32)&255;
	
	rcix1=BTM_BlockCoordsToRcix(cx+dx, cy+dy, cz+dz);
	return(rcix1);
}

u64 BTM_BlockOffsetRcixPx1(u64 rcix)
{
#ifndef BTM_RCIX_MORTON
	u64 rcix1;
	rcix1=rcix+1;
	if((rcix1&(~  15))==(rcix&(~  15)))
		return(rcix1);
#endif
	return(BTM_BlockOffsetRcix(rcix, 1, 0, 0));
}

u64 BTM_BlockOffsetRcixNx1(u64 rcix)
{
#ifndef BTM_RCIX_MORTON
	u64 rcix1;
	rcix1=rcix-1;
	if((rcix1&(~  15))==(rcix&(~  15)))
		return(rcix1);
#endif
	return(BTM_BlockOffsetRcix(rcix, -1, 0, 0));
}

u64 BTM_BlockOffsetRcixPy1(u64 rcix)
{
#ifndef BTM_RCIX_MORTON
	u64 rcix1;
	rcix1=rcix+(1<<4);
	if((rcix1&(~ 255))==(rcix&(~ 255)))
		return(rcix1);
#endif
	return(BTM_BlockOffsetRcix(rcix, 0, 1, 0));
}

u64 BTM_BlockOffsetRcixNy1(u64 rcix)
{
#ifndef BTM_RCIX_MORTON
	u64 rcix1;
	rcix1=rcix-(1<<4);
	if((rcix1&(~ 255))==(rcix&(~ 255)))
		return(rcix1);
#endif
	return(BTM_BlockOffsetRcix(rcix, 0, -1, 0));
}

u64 BTM_BlockOffsetRcixPz1(u64 rcix)
{
#ifndef BTM_RCIX_MORTON
	u64 rcix1;
	rcix1=rcix+(1<<8);
	if((rcix1&(~4095))==(rcix&(~4095)))
		return(rcix1);
#endif
	return(BTM_BlockOffsetRcix(rcix, 0, 0, 1));
}

u64 BTM_BlockOffsetRcixNz1(u64 rcix)
{
#ifndef BTM_RCIX_MORTON
	u64 rcix1;
	rcix1=rcix-(1<<8);
	if((rcix1&(~4095))==(rcix&(~4095)))
		return(rcix1);
#endif
	return(BTM_BlockOffsetRcix(rcix, 0, 0, -1));
}

u64 BTM_ConvCorgToBlkPos(u64 cpos)
{
	u64 bpos;
	int cx, cy, cz;

	cx=(cpos>> 8)&65535;
	cy=(cpos>>32)&65535;
	cz=(cpos>>56)&255;

	bpos=(((u64)cz)<<32)|(((u64)cy)<<16)|(((u64)cx)<<0);
	return(bpos);
}

int BTM_HashForBlkPos(u64 bpos)
{
	int h;
	h=((bpos*(65521ULL*65521ULL*65521ULL))>>48)&255;
	return(h);
}

volatile int btm_stat_szallocsq[24];

// void *BTM_WorldAllocSq(BTM_World *wrl, int pidx)
void *BTM_WorldAllocSqLln(BTM_World *wrl, int pidx, char *file, int line)
{
	void *ptr;
	byte *pcs;
	int i, j, k, n;

	if(pidx<4)
		pidx=4;

	BTM_LockAlloc();
	ptr=wrl->mm_p2alloc[pidx];
	if(ptr)
	{
		wrl->mm_p2alloc[pidx]=*(void **)ptr;
		BTM_UnlockAlloc();
		return(ptr);
	}
	
	printf("BTM_WorldAllocSqLln: %d %s:%d\n", 1<<pidx, file, line);
	
	if(pidx>=16)
	{
		ptr=btm_malloc(1<<pidx);
		btm_stat_szallocsq[pidx]+=1<<pidx;
		BTM_UnlockAlloc();
		return(ptr);
	}

	ptr=btm_malloc(65536);
	btm_stat_szallocsq[pidx]+=65536;

	n=1<<(16-pidx);
	k=1<<pidx;
	pcs=ptr;
	for(i=0; i<n; i++)
	{
		*(void **)pcs=wrl->mm_p2alloc[pidx];
		wrl->mm_p2alloc[pidx]=pcs;
		pcs+=k;
	}
	
	ptr=wrl->mm_p2alloc[pidx];
	wrl->mm_p2alloc[pidx]=*(void **)ptr;
	BTM_UnlockAlloc();
	return(ptr);
}

int BTM_WorldFreeSq(BTM_World *wrl, void *ptr, int pidx)
{
	BTM_LockAlloc();
	*(void **)ptr=wrl->mm_p2alloc[pidx];
	wrl->mm_p2alloc[pidx]=ptr;
	BTM_UnlockAlloc();
	return(0);
}


int BTM_UpdateWorldBlockOccBlkCheck(
	BTM_World *wrl, u32 blka, u32 blkb)
{
	if(blkb&BTM_BLKDFL_NODRAW)
		return(0);

	if(blkb&BTM_BLKDFL_SEETHRU)
	{
		if(blka&BTM_BLKDFL_SEETHRU)
			return(1);
		return(0);
	}

	if(blkb&BTM_BLKDFL_NONSOLID)
	{
		if(blka&BTM_BLKDFL_NONSOLID)
			return(1);
		if(blka&BTM_BLKDFL_FLUID)
			return(1);
		return(0);
	}

	if(blkb&BTM_BLKDFL_FLUID)
	{
		if(blka&BTM_BLKDFL_FLUID)
			return(1);
		return(0);
	}
	
	if(blkb&BTM_BLKDFL_TY_MASK)
	{
		/* Partial Block */
		return(0);
	}

	return(1);
}

int BTM_UpdateWorldBlockOccCix(BTM_World *wrl, u64 rcix)
{
	BTM_Region *rgn;
//	int xmsk, ymsk, zmsk;
//	int cix1, xsh2,
	int bt, dx, dy, dz, isfluid;
//	int xsh, zsh, cix
	int cix, cix1;
	int rix, rix1;
	u64 bm, rcix1;
	u32 blk, blk0, blkd, blk2, blk2d;
	int i, j, k;

	rix=BTM_Rcix2Rix(rcix);
//	cix=BTM_Rcix2Cix(rcix);

	rgn=BTM_GetRegionForRix(wrl, rix);

//	xmsk=(1<<xsh)-1;
//	ymsk=xmsk<<xsh;

//	xsh=7;
//	zsh=7;

//	xmsk=127;
//	ymsk=xmsk<< 7;
//	zmsk=xmsk<<14;
//	msk=(1<<21)-1;

//	xsh2=xsh<<1;
//	xsh2=14;

	cix=BTM_Rcix2Cix(rcix);
//	blk=rgn->vox[cix];
	blk=BTM_GetRegionBlockCix(wrl, rgn, cix);

//	blk=BTM_GetWorldBlockCix(wrl, rcix);

	bt=(blk&0xFF);
	if(bt<4)
	{
//		wrl->vox[cix]=blk;
		return(0);
	}

	blkd=btmgl_vox_atlas_side[bt];

	if(blkd&BTM_BLKDFL_NODRAW)
	{
		blk|=(0x3FU<<24);
//		rgn->vox[cix]=blk;
		BTM_SetRegionBlockCix(wrl, rgn, cix, blk);
		return(0);
	}

	if(blkd&BTM_BLKDFL_TY_MASK)
	{
		return(0);
	}

	
	blk0=blk;
	blk&=~(0x3FU<<24);

	isfluid=0;
	if(	(blk&0xFF==BTM_BLKTY_WATER) ||
		(blk&0xFF==BTM_BLKTY_SLIME) ||
		(blk&0xFF==BTM_BLKTY_LAVA))
	{
		isfluid|=1;
	}

	for(i=0; i<6; i++)
	{
		dx=0; dy=0; dz=0;
		switch(i)
		{
#if 0
			case 0: dy=-1; break;
			case 1: dx= 1; break;
			case 2: dy= 1; break;
			case 3: dx=-1; break;
			case 4: dz= 1; break;
			case 5: dz=-1; break;
#endif

#if 1
			case 0: dx= 1; break;
			case 1: dx=-1; break;
			case 2: dy= 1; break;
			case 3: dy=-1; break;
			case 4: dz= 1; break;
			case 5: dz=-1; break;
#endif
		}
		
#if 0
		if(isfluid)
		{
			if(i!=4)
			{
				blk|=1U<<(24+i);
				continue;
			}
		}
#endif
		
		rcix1=BTM_BlockOffsetRcix(rcix, dx, dy, dz);
		rix1=BTM_Rcix2Rix(rcix1);
		if(rix1==rix)
		{
			cix1=BTM_Rcix2Cix(rcix1);
//			bm=rgn->voxbm[cix1>>6];
			bm=(rgn->voxbm+(rgn->voxbmix[cix1>>12]<<6))[(cix1>>6)&63];

			if(bm&(1ULL<<(cix1&63)))
			{
//				blk2=rgn->vox[cix1];
				blk2=BTM_GetRegionBlockCix(wrl, rgn, cix1);
				blk2d=btmgl_vox_atlas_side[blk2&255];
				if(BTM_UpdateWorldBlockOccBlkCheck(wrl, blkd, blk2d))
					blk|=1U<<(24+i);
			}
		}
	}

	if(blk!=blk0)
	{
	//	rgn->vox[cix]=blk;
		BTM_SetRegionBlockCix(wrl, rgn, cix, blk);
		return(1);
	}
	
	return(0);
//	return(blk!=blk0);
}

int BTM_SetWorldBlockCix(BTM_World *wrl, u64 rcix, u32 blk)
{
	BTM_Region *rgn;
	int cix, rix;

	rix=BTM_Rcix2Rix(rcix);
	cix=BTM_Rcix2Cix(rcix);

	rgn=BTM_GetRegionForRix(wrl, rix);

//	rgn->vox[cix]=blk;
	BTM_SetRegionBlockCix(wrl, rgn, cix, blk);
	BTM_UpdateWorldBlockOccCix2(wrl, rcix);
	BTM_UpdateRegionSetBlockLightCix(wrl, rgn, rcix);
	return(0);
}

int BTM_SetWorldBlockCixNl(BTM_World *wrl, u64 rcix, u32 blk)
{
	BTM_Region *rgn;
	int cix, rix;

	rix=BTM_Rcix2Rix(rcix);
	cix=BTM_Rcix2Cix(rcix);

	rgn=BTM_GetRegionForRix(wrl, rix);

//	rgn->vox[cix]=blk;
	BTM_SetRegionBlockCix(wrl, rgn, cix, blk);
//	BTM_UpdateRegionSetBlockLightCix(wrl, rgn, rcix);
	return(0);
}

int BTM_SetWorldBlockXYZ(BTM_World *wrl,
	int cx, int cy, int cz, u32 blk)
{
	u64 rcix;
	int cix, rix, rx, ry;

	if(cz<0)
		return(0);
	if(cz>=128)
		return(0);

//	msk=(1<<21)-1;

//	xsh=wrl->xsh;
//	xsh2=xsh<<1;
//	cix=(((cz<<7)|cy)<<7)|cx;
	
//	rcix=(((u64)rix)<<24)|cix;
	rcix=BTM_BlockCoordsToRcix(cx, cy, cz);
	
	BTM_SetWorldBlockCix(wrl, rcix, blk);
	return(0);
}

int BTM_SetWorldBlockNlXYZ(BTM_World *wrl,
	int cx, int cy, int cz, u32 blk)
{
	u64 rcix;
	int cix, rix, rx, ry;

	if(cz<0)
		return(0);
	if(cz>=128)
		return(0);

	rcix=BTM_BlockCoordsToRcix(cx, cy, cz);	
	BTM_SetWorldBlockCixNl(wrl, rcix, blk);
	return(0);
}

int BTM_UpdateWorldBlockOccCix2(BTM_World *wrl, u64 rcix)
{
	u64 rcix1, rcix2;
	int xsh, xsh2;
	
//	xsh=wrl->xsh;
//	xsh2=xsh<<1;

	BTM_UpdateWorldBlockOccCix(wrl, rcix);
//	BTM_UpdateWorldBlockOccCix(wrl, rcix+(1    ));
//	BTM_UpdateWorldBlockOccCix(wrl, rcix-(1    ));
//	BTM_UpdateWorldBlockOccCix(wrl, rcix+(1<< 7));
//	BTM_UpdateWorldBlockOccCix(wrl, rcix-(1<< 7));
//	BTM_UpdateWorldBlockOccCix(wrl, rcix+(1<<14));
//	BTM_UpdateWorldBlockOccCix(wrl, rcix-(1<<14));

	rcix1=BTM_BlockOffsetRcix(rcix,  1, 0, 0);
	rcix2=BTM_BlockOffsetRcix(rcix, -1, 0, 0);
	BTM_UpdateWorldBlockOccCix(wrl, rcix1);
	BTM_UpdateWorldBlockOccCix(wrl, rcix2);
	rcix1=BTM_BlockOffsetRcix(rcix, 0,  1, 0);
	rcix2=BTM_BlockOffsetRcix(rcix, 0, -1, 0);
	BTM_UpdateWorldBlockOccCix(wrl, rcix1);
	BTM_UpdateWorldBlockOccCix(wrl, rcix2);
	rcix1=BTM_BlockOffsetRcix(rcix, 0, 0,  1);
	rcix2=BTM_BlockOffsetRcix(rcix, 0, 0, -1);
	BTM_UpdateWorldBlockOccCix(wrl, rcix1);
	BTM_UpdateWorldBlockOccCix(wrl, rcix2);

	return(0);
}

int BTM_UpdateWorldBlockOccCix3R(BTM_World *wrl, u64 rcix, int rcnt)
{
	u64 rcix1, rcix2;
	int xsh, xsh2, rt;
	
//	xsh=wrl->xsh;
//	xsh2=xsh<<1;

	if(rcnt<=0)
		return(0);

	rt=BTM_UpdateWorldBlockOccCix(wrl, rcix);
	if(!rt)
		return(0);

	rcix1=BTM_BlockOffsetRcix(rcix,  1, 0, 0);
	rcix2=BTM_BlockOffsetRcix(rcix, -1, 0, 0);
	BTM_UpdateWorldBlockOccCix3R(wrl, rcix1, rcnt-1);
	BTM_UpdateWorldBlockOccCix3R(wrl, rcix2, rcnt-1);
	rcix1=BTM_BlockOffsetRcix(rcix, 0,  1, 0);
	rcix2=BTM_BlockOffsetRcix(rcix, 0, -1, 0);
	BTM_UpdateWorldBlockOccCix3R(wrl, rcix1, rcnt-1);
	BTM_UpdateWorldBlockOccCix3R(wrl, rcix2, rcnt-1);
	rcix1=BTM_BlockOffsetRcix(rcix, 0, 0,  1);
	rcix2=BTM_BlockOffsetRcix(rcix, 0, 0, -1);
	BTM_UpdateWorldBlockOccCix3R(wrl, rcix1, rcnt-1);
	BTM_UpdateWorldBlockOccCix3R(wrl, rcix2, rcnt-1);

	return(0);
}

u32 BTM_GetWorldBlockCix(BTM_World *wrl, u64 rcix)
{
	BTM_Region *rgn;
	u32 blk;
	int cix, rix;

	rix=BTM_Rcix2Rix(rcix);
	cix=BTM_Rcix2Cix(rcix);

	rgn=BTM_GetRegionForRix(wrl, rix);

	blk=BTM_GetRegionBlockCix(wrl, rgn, cix);
//	blk=rgn->vox[cix];
	return(blk);
}

u32 BTM_TryGetWorldBlockCix(BTM_World *wrl, u64 rcix)
{
	BTM_Region *rgn;
	u32 blk;
	int cix, rix;

	rix=BTM_Rcix2Rix(rcix);
	cix=BTM_Rcix2Cix(rcix);

	rgn=BTM_LookupRegionForRix(wrl, rix);
	if(!rgn)
		return(0);

	blk=BTM_GetRegionBlockCix(wrl, rgn, cix);
//	blk=rgn->vox[cix];
	return(blk);
}

u32 BTM_WeakGetWorldBlockCix(BTM_World *wrl, u64 rcix)
{
	BTM_Region *rgn;
	u32 blk;
	int cix, rix;

	rix=BTM_Rcix2Rix(rcix);
	cix=BTM_Rcix2Cix(rcix);

	rgn=BTM_GetRegionForRix(wrl, rix);

	blk=BTM_WeakGetRegionBlockCix(wrl, rgn, cix);
//	blk=rgn->vox[cix];
	return(blk);
}

u32 BTM_TryWeakGetWorldBlockCix(BTM_World *wrl, u64 rcix)
{
	BTM_Region *rgn;
	u32 blk;
	int cix, rix;

	rix=BTM_Rcix2Rix(rcix);
	cix=BTM_Rcix2Cix(rcix);

	rgn=BTM_LookupRegionForRix(wrl, rix);
	if(!rgn)
		return(0);

	blk=BTM_WeakGetRegionBlockCix(wrl, rgn, cix);
//	blk=rgn->vox[cix];
	return(blk);
}

u32 BTM_GetWorldBlockXYZ(BTM_World *wrl,
	int cx, int cy, int cz)
{
	u64 rcix;
	int cix, rix, rx, ry;

	if(cz<0)
		return(0);
	if(cz>=128)
		return(0);

//	msk=(1<<21)-1;

//	xsh=wrl->xsh;
//	xsh2=xsh<<1;
//	cix=(((cz<<7)|cy)<<7)|cx;
//	rcix=(((u64)rix)<<24)|cix;

	rcix=BTM_BlockCoordsToRcix(cx, cy, cz);
	
	return(BTM_GetWorldBlockCix(wrl, rcix));
}

u32 BTM_TryGetWorldBlockXYZ(BTM_World *wrl,
	int cx, int cy, int cz)
{
	u64 rcix;
	int cix, rix, rx, ry;

	if(cz<0)
		return(0);
	if(cz>=128)
		return(0);

	rcix=BTM_BlockCoordsToRcix(cx, cy, cz);	
	return(BTM_TryGetWorldBlockCix(wrl, rcix));
}

u32 BTM_GetWorldBlockCorg(BTM_World *wrl, u64 corg)
{
	int cx, cy, cz;
	u32 v;
	
	cx=(corg>> 8)&65535;
	cy=(corg>>32)&65535;
	cz=(corg>>56)&255;
	v=BTM_GetWorldBlockXYZ(wrl, cx, cy, cz);
	return(v);
}

u32 BTM_TryGetWorldBlockCorg(BTM_World *wrl, u64 corg)
{
	int cx, cy, cz;
	u32 v;
	
	cx=(corg>> 8)&65535;
	cy=(corg>>32)&65535;
	cz=(corg>>56)&255;
	v=BTM_TryGetWorldBlockXYZ(wrl, cx, cy, cz);
	return(v);
}

int BTM_UpdateWorldBlockOcc(BTM_World *wrl)
{
	int xsh, zsh, msk;
	int i;

//	xsh=wrl->xsh;
//	zsh=wrl->zsh;
//	msk=(1<<(xsh+xsh+zsh))-1;
//	for(i=0; i<(msk+1); i++)
//		BTM_UpdateWorldBlockOccCix(wrl, i);
	return(0);
}

BTM_World *BTM_AllocWorld(int xsh, int zsh)
{
	BTM_World *tmp;
	int x, y, z, ix, xs, zs;
	int i, j;
	
	tmp=btm_malloc(sizeof(BTM_World));
	memset(tmp, 0, sizeof(BTM_World));

//	tmp->xsh=xsh;
//	tmp->zsh=zsh;
//	tmp->vox=malloc((1<<(xsh+xsh+zsh))*sizeof(u32));
//	memset(tmp->vox, 0, (1<<(xsh+xsh+zsh))*sizeof(u32));

//	tmp->voxbm=malloc((1<<(xsh+xsh+zsh-6))*8);
//	memset(tmp->voxbm, 0, (1<<(xsh+xsh+zsh-6))*8);
	
#if 0
	xs=1<<xsh;
	zs=1<<zsh;
	for(x=0; x<xs; x++)
		for(y=0; y<xs; y++)
	{
//		z=0;
//		ix=(((z<<xsh)|y)<<xsh)|x;
//		BTM_SetWorldBlockCix(tmp, ix, 0x1);

		z=zs-1;
		ix=(((z<<xsh)|y)<<xsh)|x;
		BTM_SetWorldBlockCix(tmp, ix, 0x22);
	}
#endif
	
	tmp->magic1=BTM_MAGIC1;
	tmp->magic2=BTM_MAGIC1;
	tmp->magic3=BTM_MAGIC1;
	tmp->magic4=BTM_MAGIC1;
	tmp->magic5=BTM_MAGIC1;
//	tmp->magic1=BTM_MAGIC1;
	
	return(tmp);
}

int BTM_CheckWorldMagic(BTM_World *wrl)
{
	if(	(wrl->magic1!=BTM_MAGIC1) ||
		(wrl->magic2!=BTM_MAGIC1) ||
		(wrl->magic3!=BTM_MAGIC1) ||
		(wrl->magic4!=BTM_MAGIC1) ||
		(wrl->magic5!=BTM_MAGIC1)	)
	{
		debug_break
	}
	return(0);
}

int BTM_GetWorldTimeHhMm(BTM_World *wrl)
{
	int hh, mm, hm, tt;

	tt=wrl->daytimer;
	hh=tt/60000;
	mm=(tt-(hh*60000))/1000;
	
	hh+=6;
	if(hh>=24)
		hh-=24;
	
	hm=hh*100+mm;
	return(hm);
}

int BTM_SetWorldTimeHhMm(BTM_World *wrl, int hhmm)
{
	int hh, mm, hm, tt;

	hm=hhmm;

	hh=hm/100;
	mm=hm%100;

	hh=hh-6;
	if(hh<0)
		hh+=24;
	
	tt=((hh*60+mm)*1000);
	wrl->daytimer=tt;
	return(0);
}

#if 0
static const int  btmgl_cube_tris[6*6]=
{
	0, 1, 5,  0, 5, 4,	//-Y
	1, 3, 7,  1, 7, 5,	//+X
	3, 2, 6,  3, 6, 7,	//+Y
	2, 0, 4,  2, 4, 6,	//-X
	4, 5, 7,  4, 7, 6,	//+Z
	1, 0, 2,  1, 2, 3	//-Z
};

static const int  btmgl_cube_quads[6*4]=
{
	0, 1, 5, 4,	//-Y
	1, 3, 7, 5,	//+X
	3, 2, 6, 7,	//+Y
	2, 0, 4, 6,	//-X
	4, 5, 7, 6,	//+Z
	1, 0, 2, 3	//-Z
};
#endif

#if 1
static const int  btmgl_cube_tris[6*6]=
{
	1, 3, 7,  1, 7, 5,	//+X
	2, 0, 4,  2, 4, 6,	//-X
	3, 2, 6,  3, 6, 7,	//+Y
	0, 1, 5,  0, 5, 4,	//-Y
	4, 5, 7,  4, 7, 6,	//+Z
	1, 0, 2,  1, 2, 3	//-Z
};

static const int  btmgl_cube_quads[6*4]=
{
	1, 3, 7, 5,	//+X
	2, 0, 4, 6,	//-X
	3, 2, 6, 7,	//+Y
	0, 1, 5, 4,	//-Y
	4, 5, 7, 6,	//+Z
	1, 0, 2, 3	//-Z
};

static const int  btmgl_cross_quads[6*4]=
{
	1, 2, 6, 5, //Cross X/Y 1
	3, 0, 4, 7, //Cross X/Y 2

	3, 2, 6, 7,	//+Y
	0, 1, 5, 4,	//-Y
	4, 5, 7, 6,	//+Z
	1, 0, 2, 3	//-Z
};
#endif


void *btmgl_array_mutex;
void *btmgl_world_mutex;

#if 1
/*
 0-31: Visible Region, Opaque
32-47: Visible Region, Transparent (Generic)
48-63: Visible Region, Water Surfaces (opt)
64-255: Reserved
256-4352: Chunk Cache
 */

int btmgl_blkemit_raix;		//render array index
int btmgl_blkemit_raim;		//render array mask
int btmgl_blkemit_raicm;	//render array chunk-cache mask

// main (opaque) vertex array
float *btmgl_blkemitr_xyz[256+4096];
float *btmgl_blkemitr_st[256+4096];
u32 *btmgl_blkemitr_rgb[256+4096];
int btmgl_blkemitr_npts[256+4096];
int btmgl_blkemitr_mpts[256+4096];
int btmgl_blkemitr_chid[256+4096];		//chunk ID (chunk cache)
byte btmgl_blkemitr_chrc[256+4096];		//chunk res count (chunk cache)
byte btmgl_blkemitr_chfl[256+4096];		//chunk ID (chunk cache)

// main vertex array (thread render copy)
float *btmgl_blkemitr2_xyz[256];
float *btmgl_blkemitr2_st[256];
u32 *btmgl_blkemitr2_rgb[256];
int btmgl_blkemitr2_npts[256];
#endif


#if 0
// main (opaque) vertex array
float *btmgl_blkemit_xyz;
float *btmgl_blkemit_st;
u32 *btmgl_blkemit_rgb;
int btmgl_blkemit_npts;
int btmgl_blkemit_mpts;

// main vertex array (thread render copy)
float *btmgl_blkemit2_xyz;
float *btmgl_blkemit2_st;
u32 *btmgl_blkemit2_rgb;
int btmgl_blkemit2_npts;

// alpha vertex array
float *btmgl_blkemita_xyz;
float *btmgl_blkemita_st;
u32 *btmgl_blkemita_rgb;
int btmgl_blkemita_npts;
int btmgl_blkemita_mpts;

// alpha vertex array (thread render copy)
float *btmgl_blkemita2_xyz;
float *btmgl_blkemita2_st;
u32 *btmgl_blkemita2_rgb;
int btmgl_blkemita2_npts;
#endif

int BTMGL_EmitLockArrays()
{
#ifdef BTM_RAYTHREAD
	if(!btmgl_array_mutex)
		btmgl_array_mutex=thMutex();
	thLockMutex(btmgl_array_mutex);
#endif
	return(0);
}

int BTMGL_EmitUnlockArrays()
{
#ifdef BTM_RAYTHREAD
	thUnlockMutex(btmgl_array_mutex);
#endif
	return(0);
}

int BTMGL_LockWorld()
{
#ifdef BTM_RAYTHREAD
	if(!btmgl_world_mutex)
		btmgl_world_mutex=thMutex();
	thLockMutex(btmgl_world_mutex);
#endif
	return(0);
}

int BTMGL_UnlockWorld()
{
#ifdef BTM_RAYTHREAD
	thUnlockMutex(btmgl_world_mutex);
#endif
	return(0);
}


int BTMGL_EmitBlockVertex(float *xyz, float *st, u32 rgb, int fl)
{
	int ix, ixa, raix;
	int i, j, k;

#if 1
	raix=btmgl_blkemit_raix;
	
	if((raix<0) || (raix>=(4096+256)))
	{
		__debugbreak();
	}

	if(!btmgl_blkemitr_xyz[raix])
	{
#ifdef BTM_RAYTHREAD
		if(!btmgl_array_mutex)
		{
			btmgl_array_mutex=thMutex();
			btmgl_world_mutex=thMutex();
		}
#endif
	
		BTMGL_EmitLockArrays();

//		k=(128*128)*24;
		k=(128*128)*16;	/* Region Opaque */
//		if(fl&1)
		if(raix&32)
		{
			/* Region Transparent */
			k=(128*128)*8;
		}
		
		if(raix>256)
		{
//			k=512*8*4;	/* Chunk Cache */
			k=256*8*4;	/* Chunk Cache */
		}
		
		ix=4096;
		while(ix<k)
//			ix=ix<<1;
			ix=ix+(ix>>1);

		btmgl_blkemitr_xyz[raix]=btm_malloc_va(ix*3*sizeof(float));
		btmgl_blkemitr_st[raix]=btm_malloc_va(ix*2*sizeof(float));
		btmgl_blkemitr_rgb[raix]=btm_malloc_va(ix*1*sizeof(u32));

#ifdef BTM_RAYTHREAD
		if(raix<256)
		{
			btmgl_blkemitr2_xyz[raix]=btm_malloc_va(ix*3*sizeof(float));
			btmgl_blkemitr2_st[raix]=btm_malloc_va(ix*2*sizeof(float));
			btmgl_blkemitr2_rgb[raix]=btm_malloc_va(ix*1*sizeof(u32));
		}
#else
		if(raix<256)
		{
			btmgl_blkemitr2_xyz[raix]=btmgl_blkemitr_xyz[raix];
			btmgl_blkemitr2_st[raix]=btmgl_blkemitr_st[raix];
			btmgl_blkemitr2_rgb[raix]=btmgl_blkemitr_rgb[raix];
		}
#endif

		btmgl_blkemitr_mpts[raix]=ix;
		btmgl_blkemitr_npts[raix]=0;

		BTMGL_EmitUnlockArrays();
	}
	
	if((btmgl_blkemitr_npts[raix]+1)>=btmgl_blkemitr_mpts[raix])
	{
		BTMGL_EmitLockArrays();
	
		ix=btmgl_blkemitr_mpts[raix];
		ix=ix+(ix>>1);
		
		if((btmgl_blkemitr_npts[raix]+1)>=ix)
		{
			__debugbreak();
		}

		btmgl_blkemitr_xyz[raix]=
			btm_realloc_va(
				btmgl_blkemitr_xyz[raix],
				ix*3*sizeof(float));
		btmgl_blkemitr_st[raix]=
			btm_realloc_va(
				btmgl_blkemitr_st[raix],
				ix*2*sizeof(float));
		btmgl_blkemitr_rgb[raix]=
			btm_realloc_va(
				btmgl_blkemitr_rgb[raix],
				ix*1*sizeof(u32));

#ifdef BTM_RAYTHREAD
		if(raix<256)
		{
			btmgl_blkemitr2_xyz[raix]=
				btm_realloc_va(
					btmgl_blkemitr2_xyz[raix],
					ix*3*sizeof(float));
			btmgl_blkemitr2_st[raix]=
				btm_realloc_va(
					btmgl_blkemitr2_st[raix],
					ix*2*sizeof(float));
			btmgl_blkemitr2_rgb[raix]=
				btm_realloc_va(
					btmgl_blkemitr2_rgb[raix],
					ix*1*sizeof(u32));
		}
#else
		if(raix<256)
		{
			btmgl_blkemitr2_xyz[raix]=btmgl_blkemitr_xyz[raix];
			btmgl_blkemitr2_st[raix]=btmgl_blkemitr_st[raix];
			btmgl_blkemitr2_rgb[raix]=btmgl_blkemitr_rgb[raix];
		}
#endif

		btmgl_blkemitr_mpts[raix]=ix;

		BTMGL_EmitUnlockArrays();	
	}

	ix=btmgl_blkemitr_npts[raix]++;
	memcpy(btmgl_blkemitr_xyz[raix]+ix*3, xyz, 3*sizeof(float));
	memcpy(btmgl_blkemitr_st[raix]+ix*2, st, 2*sizeof(float));
	btmgl_blkemitr_rgb[raix][ix]=rgb;
#endif

#if 0	
	if(!btmgl_blkemit_xyz)
	{
#ifdef BTM_RAYTHREAD
		if(!btmgl_array_mutex)
		{
			btmgl_array_mutex=thMutex();
			btmgl_world_mutex=thMutex();
		}
#endif
	
		BTMGL_EmitLockArrays();

//		k=(btm_drawdist*btm_drawdist)*12;
//		k=(btm_drawdist*btm_drawdist)*14;
//		k=(btm_drawdist*btm_drawdist)*16;
//		k=(btm_drawdist*btm_drawdist)*18;
		k=(btm_drawdist*btm_drawdist)*24;
//		k=(btm_drawdist*btm_drawdist)*48;

		j=(btm_drawdist*btm_drawdist)*8;
		
		ix=4096;
		while(ix<k)
//			ix=ix<<1;
			ix=ix+(ix>>1);

		ixa=4096;
		while(ixa<j)
			ixa=ix+(ixa>>1);

//		ix=4096;
//		ix=1<<19;
//		ix=1<<18;
		btmgl_blkemit_xyz=btm_malloc_va(ix*3*sizeof(float));
		btmgl_blkemit_st=btm_malloc_va(ix*2*sizeof(float));
		btmgl_blkemit_rgb=btm_malloc_va(ix*1*sizeof(u32));

		btmgl_blkemita_xyz=btm_malloc_va(ixa*3*sizeof(float));
		btmgl_blkemita_st=btm_malloc_va(ixa*2*sizeof(float));
		btmgl_blkemita_rgb=btm_malloc_va(ixa*1*sizeof(u32));

#ifdef BTM_RAYTHREAD
		btmgl_blkemit2_xyz=btm_malloc_va(ix*3*sizeof(float));
		btmgl_blkemit2_st=btm_malloc_va(ix*2*sizeof(float));
		btmgl_blkemit2_rgb=btm_malloc_va(ix*1*sizeof(u32));

		btmgl_blkemita2_xyz=btm_malloc_va(ixa*3*sizeof(float));
		btmgl_blkemita2_st=btm_malloc_va(ixa*2*sizeof(float));
		btmgl_blkemita2_rgb=btm_malloc_va(ixa*1*sizeof(u32));
#else
		btmgl_blkemit2_xyz=btmgl_blkemit_xyz;
		btmgl_blkemit2_st=btmgl_blkemit_st;
		btmgl_blkemit2_rgb=btmgl_blkemit_rgb;

		btmgl_blkemita2_xyz=btmgl_blkemita_xyz;
		btmgl_blkemita2_st=btmgl_blkemita_st;
		btmgl_blkemita2_rgb=btmgl_blkemita_rgb;
#endif

		btmgl_blkemit_mpts=ix;
		btmgl_blkemita_mpts=ixa;

		BTMGL_EmitUnlockArrays();
	}
	
	if((btmgl_blkemit_npts+1)>=btmgl_blkemit_mpts)
	{
		BTMGL_EmitLockArrays();
	
		ix=btmgl_blkemit_mpts+(btmgl_blkemit_mpts>>1);

		btmgl_blkemit_xyz=btm_realloc_va(btmgl_blkemit_xyz,
			ix*3*sizeof(float));
		btmgl_blkemit_st=btm_realloc_va(btmgl_blkemit_st,
			ix*2*sizeof(float));
		btmgl_blkemit_rgb=btm_realloc_va(btmgl_blkemit_rgb,
			ix*1*sizeof(u32));

#ifdef BTM_RAYTHREAD
		btmgl_blkemit2_xyz=btm_realloc_va(btmgl_blkemit2_xyz,
			ix*3*sizeof(float));
		btmgl_blkemit2_st=btm_realloc_va(btmgl_blkemit2_st,
			ix*2*sizeof(float));
		btmgl_blkemit2_rgb=btm_realloc_va(btmgl_blkemit2_rgb,
			ix*1*sizeof(u32));
#else
		btmgl_blkemit2_xyz=btmgl_blkemit_xyz;
		btmgl_blkemit2_st=btmgl_blkemit_st;
		btmgl_blkemit2_rgb=btmgl_blkemit_rgb;
#endif

		btmgl_blkemit_mpts=ix;

		BTMGL_EmitUnlockArrays();	
	}

	if((btmgl_blkemita_npts+1)>=btmgl_blkemita_mpts)
	{
		BTMGL_EmitLockArrays();
	
		ix=btmgl_blkemita_mpts+(btmgl_blkemita_mpts>>1);

		btmgl_blkemita_xyz=btm_realloc_va(btmgl_blkemita_xyz,
			ix*3*sizeof(float));
		btmgl_blkemita_st=btm_realloc_va(btmgl_blkemita_st,
			ix*2*sizeof(float));
		btmgl_blkemita_rgb=btm_realloc_va(btmgl_blkemita_rgb,
			ix*1*sizeof(u32));

#ifdef BTM_RAYTHREAD
		btmgl_blkemita2_xyz=btm_realloc_va(btmgl_blkemita2_xyz,
			ix*3*sizeof(float));
		btmgl_blkemita2_st=btm_realloc_va(btmgl_blkemita2_st,
			ix*2*sizeof(float));
		btmgl_blkemita2_rgb=btm_realloc_va(btmgl_blkemita2_rgb,
			ix*1*sizeof(u32));
#else
		btmgl_blkemita2_xyz=btmgl_blkemita_xyz;
		btmgl_blkemita2_st=btmgl_blkemita_st;
		btmgl_blkemita2_rgb=btmgl_blkemita_rgb;
#endif

		btmgl_blkemita_mpts=ix;

		BTMGL_EmitUnlockArrays();	
	}

	if(fl&1)
	{
		ix=btmgl_blkemita_npts++;
		memcpy(btmgl_blkemita_xyz+ix*3, xyz, 3*sizeof(float));
		memcpy(btmgl_blkemita_st+ix*2, st, 2*sizeof(float));
		btmgl_blkemita_rgb[ix]=rgb;
	}else
	{
		ix=btmgl_blkemit_npts++;
		memcpy(btmgl_blkemit_xyz+ix*3, xyz, 3*sizeof(float));
		memcpy(btmgl_blkemit_st+ix*2, st, 2*sizeof(float));
		btmgl_blkemit_rgb[ix]=rgb;
	}
#endif
	
	return(0);
}

int BTMGL_CheckRaIxForChid(int chid)
{
	int ks, k0, k1, k2, k3;

	if(!btmgl_blkemit_raicm)
		return(0);

	ks=chid^(chid>>17);
	ks=(ks^(ks>>5)^(ks>>7));
	ks=(ks<<1)^(ks>>3);

	k0=(ks&btmgl_blkemit_raicm)+256;
	if(btmgl_blkemitr_chid[k0]==chid)
	{
		btmgl_blkemitr_chrc[k0]=128;
		return(k0);
	}

	ks=(ks<<1)^(ks>>3);
	k1=(ks&btmgl_blkemit_raicm)+256;
	if(btmgl_blkemitr_chid[k1]==chid)
	{
		btmgl_blkemitr_chrc[k1]=96;
		return(k1);
	}

	ks=(ks<<1)^(ks>>3);
	k2=(ks&btmgl_blkemit_raicm)+256;
	if(btmgl_blkemitr_chid[k2]==chid)
	{
		btmgl_blkemitr_chrc[k2]=64;
		return(k2);
	}

	ks=(ks<<1)^(ks>>3);
	k3=(ks&btmgl_blkemit_raicm)+256;
	if(btmgl_blkemitr_chid[k3]==chid)
	{
		btmgl_blkemitr_chrc[k3]=48;
		return(k3);
	}
	
	if(!btmgl_blkemitr_chrc[k0])
	{
		btmgl_blkemitr_chid[k0]=chid;
		btmgl_blkemitr_npts[k0]=0;
		btmgl_blkemitr_chrc[k0]=128;
		btmgl_blkemitr_chfl[k0]=0;
		return(k0);
	}
	
	if(!btmgl_blkemitr_chrc[k1])
	{
		btmgl_blkemitr_chid[k1]=chid;
		btmgl_blkemitr_npts[k1]=0;
		btmgl_blkemitr_chrc[k1]=96;
		btmgl_blkemitr_chfl[k1]=0;
		return(k1);
	}
	
	if(!btmgl_blkemitr_chrc[k2])
	{
		btmgl_blkemitr_chid[k2]=chid;
		btmgl_blkemitr_npts[k2]=0;
		btmgl_blkemitr_chrc[k2]=64;
		btmgl_blkemitr_chfl[k2]=0;
		return(k2);
	}

	if(!btmgl_blkemitr_chrc[k3])
	{
		btmgl_blkemitr_chid[k3]=chid;
		btmgl_blkemitr_npts[k3]=0;
		btmgl_blkemitr_chrc[k3]=48;
		btmgl_blkemitr_chfl[k3]=0;
		return(k3);
	}

	return(0);
}

int BTMGL_ProbeRaIxForChid(int chid)
{
	int ks, k0, k1, k2, k3;

	if(!btmgl_blkemit_raicm)
		return(0);

	ks=chid^(chid>>17);
	ks=(ks^(ks>>5)^(ks>>7));
	ks=(ks<<1)^(ks>>3);

	k0=(ks&btmgl_blkemit_raicm)+256;
	if(btmgl_blkemitr_chid[k0]==chid)
		return(k0);

	ks=(ks<<1)^(ks>>3);
	k1=(ks&btmgl_blkemit_raicm)+256;
	if(btmgl_blkemitr_chid[k1]==chid)
		return(k1);

	ks=(ks<<1)^(ks>>3);
	k2=(ks&btmgl_blkemit_raicm)+256;
	if(btmgl_blkemitr_chid[k2]==chid)
		return(k2);

	ks=(ks<<1)^(ks>>3);
	k3=(ks&btmgl_blkemit_raicm)+256;
	if(btmgl_blkemitr_chid[k3]==chid)
		return(k3);

	return(0);
}

int BTMGL_ChidForCoords(int cx, int cy, int cz)
{
	int i, j, k, chid;
	i=(cz>>4)&7;
	j=(cy>>4)&4095;
	k=(cx>>4)&4095;
	chid=(i<<24)|(j<<12)|k;
	if(cx<0)	chid|=1<<28;
	if(cy<0)	chid|=1<<29;
	return(chid);
}

int BTMGL_RaIxForCoords(int cx, int cy, int cz)
{
	int i, j, k, chid;

	j=(cy>>7)&511;
	k=(cx>>7)&511;
	k=k|(j<<3);
	k=(k^(k>>5)^(k>>7))&btmgl_blkemit_raim;
	
	return(k);
}

int BTMGL_TickRaIxChidReservations()
{
	int i, j, k;
	
	for(i=0; i<=btmgl_blkemit_raicm; i++)
	{
		k=i+256;
		j=btmgl_blkemitr_chrc[k];
		if(j>0)j--;
		btmgl_blkemitr_chrc[k]=j;
	}
	return(0);
}

int BTMGL_EmitBlockFaces(int cx, int cy, int cz, int fm, u32 blk,
	u64 lbl, u64 lsl)
{
	float xyz[8*4];
	float sta[4*4*2];
	const int *tri;
	float *st;
	float mx, my, mz, nx, ny, nz;
	int i0, i1, i2, i3;
	u32 rgb, rgb1, binf;
	int bt, tx, ty, bl, sl, ma, vfl, chid;
	int i, j, k, l;
	
	if(!(fm&0x3F))
		return(0);
	
	bt=blk&255;
	if(bt<4)
		return(0);
	if(bt==BTM_BLKTY_SKY1)
		return(0);
	
	if(!(fm&0x80))
	{
		k=(blk>>24)&0x3F;
		fm&=~k;
		if(!(fm&0x3F))
			return(0);
	}
	
	binf=btmgl_vox_atlas_side[bt];
	
	mx=cx;	nx=cx+1;
	my=cy;	ny=cy+1;
	mz=cz;	nz=cz+1;
	
	if(fm&0x80)
	{
		nz=cz+0.5;
	}
	
	for(i=0; i<8; i++)
	{
		j=i*4;
		xyz[j+0]=(i&1)?nx:mx;
		xyz[j+1]=(i&2)?ny:my;
		xyz[j+2]=(i&4)?nz:mz;
	}

//	rgb=0xFFFFFFFFU;

	
	tx=(binf>> 0)&15;	ty=15-((binf>>4)&15);
	sta[0]=(tx+0)*(1.0/16)+(1.0/2048);	sta[1]=1.0-(1.0/2048)-(ty+0)*(1.0/16);
	sta[2]=(tx+1)*(1.0/16)-(1.0/2048);	sta[3]=1.0-(1.0/2048)-(ty+0)*(1.0/16);
	sta[4]=(tx+0)*(1.0/16)+(1.0/2048);	sta[5]=1.0+(1.0/2048)-(ty+1)*(1.0/16);
	sta[6]=(tx+1)*(1.0/16)-(1.0/2048);	sta[7]=1.0+(1.0/2048)-(ty+1)*(1.0/16);

	if(((binf>>8)&0xFF) == (binf&0xFF))
	{
		memcpy(sta+ 8, sta+0, 8*sizeof(float));
		memcpy(sta+16, sta+0, 8*sizeof(float));
	}else
	{
		tx=(binf>> 8)&15;	ty=15-((binf>>12)&15);
		sta[ 8]=(tx+0)*(1.0/16)+(1.0/2048);
		sta[ 9]=(ty+0)*(1.0/16)+(1.0/2048);
		sta[10]=(tx+1)*(1.0/16)-(1.0/2048);
		sta[11]=(ty+0)*(1.0/16)+(1.0/2048);
		sta[12]=(tx+0)*(1.0/16)+(1.0/2048);
		sta[13]=(ty+1)*(1.0/16)-(1.0/2048);
		sta[14]=(tx+1)*(1.0/16)-(1.0/2048);
		sta[15]=(ty+1)*(1.0/16)-(1.0/2048);

		tx=(binf>>16)&15;	ty=15-((binf>>20)&15);
		sta[16]=(tx+0)*(1.0/16)+(1.0/2048);
		sta[17]=(ty+0)*(1.0/16)+(1.0/2048);
		sta[18]=(tx+1)*(1.0/16)-(1.0/2048);
		sta[19]=(ty+0)*(1.0/16)+(1.0/2048);
		sta[20]=(tx+0)*(1.0/16)+(1.0/2048);
		sta[21]=(ty+1)*(1.0/16)-(1.0/2048);
		sta[22]=(tx+1)*(1.0/16)-(1.0/2048);
		sta[23]=(ty+1)*(1.0/16)-(1.0/2048);

		for(i=4; i<12; i++)
		{
			sta[i*2+1]=1.0-sta[i*2+1];
		}
	}

	if(binf&BTM_BLKDFL_FLUID)
	{
		st=sta;
		
		for(i=0; i<3; i++)
		{
			i0=((btmgl_time_ms>>5)+(cx<<4)-(cy<<4))&255;
			i1=(i0+16)&255;
			i2=(i0-16)&255;
			i3=(i2+16)&255;
			st[0]+=btmra_sinang_f[i0]*(0.125/16);
			st[1]+=btmra_cosang_f[i0]*(0.125/16);
			st[2]+=btmra_sinang_f[i1]*(0.125/16);
			st[3]+=btmra_cosang_f[i1]*(0.125/16);
			st[4]+=btmra_sinang_f[i2]*(0.125/16);
			st[5]+=btmra_cosang_f[i2]*(0.125/16);
			st[6]+=btmra_sinang_f[i3]*(0.125/16);
			st[7]+=btmra_cosang_f[i3]*(0.125/16);
			if(((binf>>8)&0xFF) == (binf&0xFF))
				break;
			st+=8;
		}
		if(i<3)
		{
			memcpy(sta+ 8, sta+0, 8*sizeof(float));
			memcpy(sta+16, sta+0, 8*sizeof(float));
		}
	}

	vfl=0;
	if(	(binf&BTM_BLKDFL_FLUID) ||
		(binf&BTM_BLKDFL_SEETHRU))
	{
		vfl|=1;
	}

	j=(cy>>7)&511;
	k=(cx>>7)&511;
	k=k|(j<<3);
	k=(k^(k>>5)^(k>>7))&btmgl_blkemit_raim;
	if(vfl&1)
	{
		if(btmgl_blkemit_raim==31)
			k&=15;
		k|=32;
	}
	
	if(fm&0x1000)
	{
		if(!btmgl_blkemit_raicm)
			return(0);
	
//		i=(cz>>4)&7;
//		j=(cy>>4)&4095;
//		k=(cx>>4)&4095;
//		chid=(i<<24)|(j<<12)|k;
		chid=BTMGL_ChidForCoords(cx, cy, cz);

		k=BTMGL_CheckRaIxForChid(chid);
		if(k<256)
			return(0);
	}
	
	btmgl_blkemit_raix=k;

#if 1
	for(i=0; i<6; i++)
	{
		if(!(fm&(1<<i)))
			continue;

		bl=(lbl>>(i*8))&255;
		sl=(lsl>>(i*8))&255;
		
		l=((bl&15)>(sl&15))?bl:sl;

		if(i==4)
		{
			rgb=0xFFFFFFFFU;
			st=sta+8;
		}else if(i==5)
		{
			rgb=0xFF9F9F9FU;
			st=sta+16;
		}else
		{
			rgb=0xFFBFBFBFU;
			st=sta;
		}
		
		if(fm&64)
		{
			rgb&=0xFF7FFF7FU;
		}

		if(l!=0x0F)
		{
			rgb=BTM_ModulateColorRgbForBlockLight(rgb, l);
		}
		
//		if((blk&0xFF)==BTM_BLKTY_WATER)
//		{
//			rgb&=0x3FFFFFFF;
//		}

		tri=btmgl_cube_quads+i*4;
		BTMGL_EmitBlockVertex(xyz+tri[0]*4, st+0*2, rgb, vfl);
		BTMGL_EmitBlockVertex(xyz+tri[1]*4, st+1*2, rgb, vfl);
		BTMGL_EmitBlockVertex(xyz+tri[2]*4, st+3*2, rgb, vfl);
		BTMGL_EmitBlockVertex(xyz+tri[3]*4, st+2*2, rgb, vfl);
	}
#endif

	return(0);
}

int BTMGL_EmitBlockFacesSlab(
	int cx, int cy, int cz,
	int fm, u32 blk,
	u64 lbl, u64 lsl)
{
	BTMGL_EmitBlockFaces(cx, cy, cz, 0x80|fm, blk, lbl, lsl);
	return(0);
}



int BTMGL_EmitScaledBlockFaces(
	float cxm, float cym, float czm,
	float cxn, float cyn, float czn,
	int fm, u32 blk,
	u64 lbl, u64 lsl)
{
	float xyz[8*4];
	float sta[4*4*2];
	const int *tri;
	float *st;
	float mx, my, mz, nx, ny, nz;
	int cx, cy, cz;
	int i0, i1, i2, i3;
	u32 rgb, rgb1, binf;
	int bt, tx, ty, bl, sl, ma;
	int i, j, k, l;
	
	if(!(fm&0x3F))
		return(0);
	
	bt=blk&255;
	binf=btmgl_vox_atlas_side[bt];
	
	mx=cxm;	nx=cxn;
	my=cym;	ny=cyn;
	mz=czm;	nz=czn;
	
	cx=(cxm+cxn)*0.5;
	cy=(cym+cyn)*0.5;
	
	for(i=0; i<8; i++)
	{
		j=i*4;
		xyz[j+0]=(i&1)?nx:mx;
		xyz[j+1]=(i&2)?ny:my;
		xyz[j+2]=(i&4)?nz:mz;
	}

	tx=(binf>> 0)&15;	ty=15-((binf>>4)&15);
	sta[0]=(tx+0)*(1.0/16)+(1.0/512);	sta[1]=1.0-(1.0/512)-(ty+0)*(1.0/16);
	sta[2]=(tx+1)*(1.0/16)-(1.0/512);	sta[3]=1.0-(1.0/512)-(ty+0)*(1.0/16);
	sta[4]=(tx+0)*(1.0/16)+(1.0/512);	sta[5]=1.0+(1.0/512)-(ty+1)*(1.0/16);
	sta[6]=(tx+1)*(1.0/16)-(1.0/512);	sta[7]=1.0+(1.0/512)-(ty+1)*(1.0/16);

	if(((binf>>8)&0xFF) == (binf&0xFF))
	{
		memcpy(sta+ 8, sta+0, 8*sizeof(float));
		memcpy(sta+16, sta+0, 8*sizeof(float));
	}else
	{
		tx=(binf>> 8)&15;	ty=15-((binf>>12)&15);
		sta[ 8]=(tx+0)*(1.0/16)+(1.0/256);	sta[ 9]=(ty+0)*(1.0/16)+(1.0/256);
		sta[10]=(tx+1)*(1.0/16)-(1.0/256);	sta[11]=(ty+0)*(1.0/16)+(1.0/256);
		sta[12]=(tx+0)*(1.0/16)+(1.0/256);	sta[13]=(ty+1)*(1.0/16)-(1.0/256);
		sta[14]=(tx+1)*(1.0/16)-(1.0/256);	sta[15]=(ty+1)*(1.0/16)-(1.0/256);

		tx=(binf>>16)&15;	ty=15-((binf>>20)&15);
		sta[16]=(tx+0)*(1.0/16)+(1.0/256);	sta[17]=(ty+0)*(1.0/16)+(1.0/256);
		sta[18]=(tx+1)*(1.0/16)-(1.0/256);	sta[19]=(ty+0)*(1.0/16)+(1.0/256);
		sta[20]=(tx+0)*(1.0/16)+(1.0/256);	sta[21]=(ty+1)*(1.0/16)-(1.0/256);
		sta[22]=(tx+1)*(1.0/16)-(1.0/256);	sta[23]=(ty+1)*(1.0/16)-(1.0/256);

		for(i=4; i<12; i++)
		{
			sta[i*2+1]=1.0-sta[i*2+1];
		}
	}

#if 0
	if(binf&BTM_BLKDFL_FLUID)
	{
		st=sta;
		
		for(i=0; i<3; i++)
		{
			i0=((btmgl_time_ms>>5)+(cxm<<4)-(cym<<4))&255;
			i1=(i0+16)&255;
			i2=(i0-16)&255;
			i3=(i2+16)&255;
			st[0]+=btmra_sinang_f[i0]*(0.125/16);
			st[1]+=btmra_cosang_f[i0]*(0.125/16);
			st[2]+=btmra_sinang_f[i1]*(0.125/16);
			st[3]+=btmra_cosang_f[i1]*(0.125/16);
			st[4]+=btmra_sinang_f[i2]*(0.125/16);
			st[5]+=btmra_cosang_f[i2]*(0.125/16);
			st[6]+=btmra_sinang_f[i3]*(0.125/16);
			st[7]+=btmra_cosang_f[i3]*(0.125/16);
			st+=8;
		}
	}
#endif

	j=(cy>>7)&511;
	k=(cx>>7)&511;
	k=k|(j<<3);
	k=(k^(k>>5)^(k>>7))&btmgl_blkemit_raim;
//	if(vfl&1)
	if(btmgl_blkemit_raim==31)
		k&=15;
	k|=32;
	btmgl_blkemit_raix=k;

#if 1
	for(i=0; i<6; i++)
	{
		if(!(fm&(1<<i)))
			continue;

		bl=(lbl>>(i*8))&255;
		sl=(lsl>>(i*8))&255;
		
		l=((bl&15)>(sl&15))?bl:sl;

		if(i==4)
		{
			rgb=0xFFFFFFFFU;
			st=sta+8;
		}else if(i==5)
		{
			rgb=0xFF9F9F9FU;
			st=sta+16;
		}else
		{
			rgb=0xFFBFBFBFU;
			st=sta;
		}
		
		if(fm&64)
		{
			rgb&=0xFF7FFF7FU;
		}

		if(l!=0x0F)
		{
			rgb=BTM_ModulateColorRgbForBlockLight(rgb, l);
		}

		if(fm&0x100)
		{
			tri=btmgl_cross_quads+i*4;
		}else
		{
			tri=btmgl_cube_quads+i*4;
		}
		BTMGL_EmitBlockVertex(xyz+tri[0]*4, st+0*2, rgb, 1);
		BTMGL_EmitBlockVertex(xyz+tri[1]*4, st+1*2, rgb, 1);
		BTMGL_EmitBlockVertex(xyz+tri[2]*4, st+3*2, rgb, 1);
		BTMGL_EmitBlockVertex(xyz+tri[3]*4, st+2*2, rgb, 1);
	}
#endif

	return(0);
}

int BTMGL_EmitBlockFacesStair(
	int cx, int cy, int cz,
	int fm, u32 blk,
	u64 lbl, u64 lsl)
{
	BTMGL_EmitScaledBlockFaces(
		cx+0, cy+0, cz+0,
		cx+1, cy+1, cz+0.33,
		fm, blk, lbl, lsl);

	switch((blk>>8)&3)
	{
	case 0:
		BTMGL_EmitScaledBlockFaces(
			cx+0.00, cy+0.00, cz+0.00,
			cx+1.00, cy+0.33, cz+1.00,
			fm, blk, lbl, lsl);
		BTMGL_EmitScaledBlockFaces(
			cx+0.00, cy+0.33, cz+0.00,
			cx+1.00, cy+0.66, cz+0.66,
			fm, blk, lbl, lsl);
		break;
	case 2:
		BTMGL_EmitScaledBlockFaces(
			cx+0.00, cy+0.66, cz+0.00,
			cx+1.00, cy+1.00, cz+1.00,
			fm, blk, lbl, lsl);
		BTMGL_EmitScaledBlockFaces(
			cx+0.00, cy+0.33, cz+0.00,
			cx+1.00, cy+0.66, cz+0.66,
			fm, blk, lbl, lsl);
		break;

	case 3:
		BTMGL_EmitScaledBlockFaces(
			cx+0.00, cy+0.00, cz+0.00,
			cx+0.33, cy+1.00, cz+1.00,
			fm, blk, lbl, lsl);
		BTMGL_EmitScaledBlockFaces(
			cx+0.33, cy+0.00, cz+0.00,
			cx+0.66, cy+1.00, cz+0.66,
			fm, blk, lbl, lsl);
		break;
	case 1:
		BTMGL_EmitScaledBlockFaces(
			cx+0.66, cy+0.00, cz+0.00,
			cx+1.00, cy+1.00, cz+1.00,
			fm, blk, lbl, lsl);
		BTMGL_EmitScaledBlockFaces(
			cx+0.33, cy+0.00, cz+0.00,
			cx+0.66, cy+1.00, cz+0.66,
			fm, blk, lbl, lsl);
		break;
	}

//	BTMGL_EmitBlockFaces(cx, cy, cz, 0x80|fm, blk, lbl, lsl);
	return(0);
}

int BTMGL_EmitBlockFacesPlant(
	int cx, int cy, int cz,
	int fm, u32 blk,
	u64 lbl, u64 lsl)
{
	u32 blkd;
	float zofsb, zofst;

	blkd=btmgl_vox_atlas_side[blk&255];

	zofsb=0;
	zofst=0;
	
	if((blkd&BTM_BLKDFL_TY_MASK)==BTM_BLKDFL_TY_CROP)
	{
		zofsb=(-0.5)+((((blk>>8)&15)+1)/16.0)*0.5;
		zofst=(-0.9)+((((blk>>8)&15)+1)/16.0)*0.9;
	}
	
	if((blk&255)==BTM_BLKTY_SCONCE)
		lbl=0x0F0F0F0F0F0FULL;

	BTMGL_EmitScaledBlockFaces(
		cx+0, cy+0, cz+0+zofsb,
		cx+1, cy+1, cz+1+zofst,
		0x103, blk, lbl, lsl);
	return(0);
}

int BTM_DrawRegionFarBlocks(BTM_World *wrl, BTM_Region *rgn)
{
	s16 wascache[512];
	s16 docache[512];
	u32 *ros_cel;
	u16 *ros_map, *ros_map_z;
	byte *fhit;
	u32 blk, blk1;
	u64 rpos, rcix, rcix1;
	u64 ebl, esl;
	int px, py, pz, drawfar, drawnear;
	int rix, rix1, isfull;
	int cx, cy, cz, vx, vy, vz, ch, dx, dy, d, pl, chid, n, cix, cix1;
	int bx, by, bz, vi, vj, vcx, vcy, rcx, rcy, fm, vi1, vz1;
	int i, j, k;

	vcx=(wrl->cam_org>> 8)&0xFFFF;
	vcy=(wrl->cam_org>>32)&0xFFFF;
	
	rcx=((rgn->rgnix>>0)&511)*128;
	rcy=((rgn->rgnix>>9)&511)*128;
	rix=rgn->rgnix;
	
#if 0
	j=(rcy>>7)&511;
	k=(rcx>>7)&511;
	k=k|(j<<3);
	k=(k^(k>>5)^(k>>7))&btmgl_blkemit_raim;
	btmgl_blkemit_raix=k;
#endif

	if((rcx>=(256*128)) && (vcx<(128*128)))
		rcx-=512*128;
	if((rcy>=(256*128)) && (vcy<(128*128)))
		rcy-=512*128;

	if((rcx<=(256*128)) && (vcx>(384*128)))
		rcx+=512*128;
	if((rcy<=(256*128)) && (vcy>(384*128)))
		rcy+=512*128;

	ros_cel=rgn->ros_cels;
	ros_map=rgn->ros_maps[4];
	if(!ros_map)
		return(0);

	ros_map_z=ros_map;
	
//	drawfar=btm_drawdist>>1;
//	drawfar=(3*btm_drawdist)>>2;
	drawfar=(5*btm_drawdist)>>3;
	drawnear=64;
	if(btm_drawdist<drawnear)
		drawnear=btm_drawdist;
	if(drawnear>drawfar)
		drawfar=drawnear;
	
	wrl->drawnear=drawnear;
	wrl->drawfar=drawfar;

	/* draw caches... */
	for(cz=0; cz<8; cz++)
		for(cy=0; cy<8; cy++)
			for(cx=0; cx<8; cx++)
	{
		ch=cz*64+cy*8+cx;

		docache[ch]=0;
		chid=BTMGL_ChidForCoords(rcx+cx*16, rcy+cy*16, cz*16);
		k=BTMGL_ProbeRaIxForChid(chid);
		if(k<=0)
		{
			wascache[ch]=0;
			rgn->chkraix[ch]=0;
			continue;
		}

		wascache[ch]=k;
		rgn->chkraix[ch]=k;
		
		if(!rgn->chkhit[ch])
			continue;

		vx=rcx+cx*16;
		vy=rcy+cy*16;
		vz=cz*16;

		d=BTM_CameraDistanceToCoords(wrl, vx, vy, vz);
//		if(d<(btm_drawdist>>2))
		if(d<drawnear)
		{
			/* force raycast */
			btmgl_blkemitr_npts[k]=0;
			btmgl_blkemitr_chid[k]=0;
			btmgl_blkemitr_chfl[k]=0;

			wascache[ch]=0;
			rgn->chkraix[ch]=0;
			continue;
		}

		if(d<drawfar)
		{
//			if(!rgn->voxa[ch])
			if(!(btmgl_blkemitr_chfl[k]&1))
			{
				/* needs to be converted to proper chunk. */
				btmgl_blkemitr_npts[k]=0;
				wascache[ch]=0;
				rgn->chkraix[ch]=0;
				continue;
			}
		}

		k=BTMGL_CheckRaIxForChid(chid);
		wascache[ch]=k;
		rgn->chkraix[ch]=k;

		btmgl_blkemit_raix=BTMGL_RaIxForCoords(vx, vy, vz);
		
//		n=btmgl_blkemitr_npts[k]/4;
		n=btmgl_blkemitr_npts[k];
		for(i=0; i<n; i++)
		{
			BTMGL_EmitBlockVertex(
				btmgl_blkemitr_xyz[k]+i*3,
				btmgl_blkemitr_st[k]+i*2,
				btmgl_blkemitr_rgb[k][i], 0);
		}
	}

	for(cz=0; cz<8; cz++)
		for(cy=0; cy<8; cy++)
			for(cx=0; cx<8; cx++)
	{
		ch=cz*64+cy*8+cx;
		if(!rgn->chkhit[ch])
			continue;

		if(wascache[ch])
			continue;
		if(rgn->chkraix[ch])
			continue;

		chid=BTMGL_ChidForCoords(rcx+cx*16, rcy+cy*16, cz*16);
		
		vx=rcx+cx*16;
		vy=rcy+cy*16;
		vz=cz*16;

		d=BTM_CameraDistanceToCoords(wrl, vx, vy, vz);
		if(d<drawnear)
			continue;
//		if(d>=drawfar)
//			continue;

		k=docache[ch];
		if(!k)
		{
			k=BTMGL_CheckRaIxForChid(chid);
			rgn->chkraix[ch]=k;
			if(!k)k=1;
			docache[ch]=k;
		}

		isfull=0;

		if(k>=256)
		{
			rcix=BTM_BlockCoordsToRcix(vx, vy, vz+15);
			cix=BTM_Rcix2Cix(rcix);
			blk=BTM_WeakGetRegionBlockCix(wrl, rgn, cix);

			if(rgn->voxa[ch])
			{
				isfull=1;
				btmgl_blkemitr_chfl[k]|=1;
			}
		}

		for(bz=0; bz<16; bz++)
			for(by=0; by<16; by++)
				for(bx=0; bx<16; bx++)
		{
			vx=cx*16+bx;
			vy=cy*16+by;
			vz=cz*16+bz;

			rcix=BTM_BlockCoordsToRcix(
				rcx+vx, rcy+vy, vz);
			cix=BTM_Rcix2Cix(rcix);
			blk=BTM_WeakGetRegionBlockCix(wrl, rgn, cix);

			if((blk&255)<4)
				continue;
//			if(((blk>>24)&0x3F)==0x3F)
//				continue;

			fm=((blk>>24)&0x3F)^0x3F;
			fm|=0x1000;

			if((blk&0xFF)==BTM_BLKTY_WATER)
				fm&=~0x0F;
			
			if(fm==0x1000)
				continue;
			
			ebl=0;
			esl=0;
			
			if(isfull)
			{
				for(j=0; j<6; j++)
				{
					if(!(fm&(1<<j)))
						continue;

					px=0; py=0; pz=0;
					switch(j)
					{
						case 0: px= 1; break;
						case 1: px=-1; break;
						case 2: py= 1; break;
						case 3: py=-1; break;
						case 4: pz= 1; break;
						case 5: pz=-1; break;
					}
					rcix1=BTM_BlockOffsetRcix(rcix, px, py, pz);
					rix1=BTM_Rcix2Cix(rcix1);

					if(rix1==rix)
					{
						cix1=BTM_Rcix2Cix(rcix1);
						blk1=BTM_WeakGetRegionBlockCix(wrl, rgn, cix1);
					}else
					{
						blk1=BTM_TryWeakGetWorldBlockCix(wrl, rcix1);
					}

					k=(blk1>>12)&255LL;

					if((blk1&0xFF)==BTM_BLKTY_WATER)
						{ k=0x80|(k&15); }
					ebl|=((u64)k)<<(j*8);

					k=(blk1>>20)&15;
					k=(k*wrl->daylight)>>4;
					if((blk1&0xFF)==BTM_BLKTY_WATER)
						{ k|=0x80; }
					esl|=((u64)k)<<(j*8);
				}
			}else
			{
				ebl=(blk>>12)&0xFF;
				esl=(blk>>20)&0xF;
				ebl|=ebl<<8;	esl|=esl<<8;
				ebl|=ebl<<16;	esl|=esl<<16;
				ebl|=ebl<<32;	esl|=esl<<32;
			}

			BTMGL_EmitBlockFaces(vx+rcx, vy+rcy, vz, fm, blk, ebl, esl);
		}
	}

#if 0
	for(cy=0; cy<8; cy++)
		for(cx=0; cx<8; cx++)
	{
		if(!rgn->facehit[(cy<<3)+cx])
			continue;

#if 0
		vx=rcx+cx*16;
		vy=rcy+cy*16;
		dx=vcx-vx;			dy=vcy-vy;
		dx=dx^(dx>>31);		dy=dy^(dy>>31);
		d=dx+(dy>>1);
		if(dy>dx)	d=dy+(dx>>1);
		if(d>=btm_drawdist)
			continue;
#endif

		for(by=0; by<16; by++)
			for(bx=0; bx<16; bx++)
		{
			vx=cx*16+bx;
			vy=cy*16+by;
			vj=vy*128+vx;
			vi=ros_map[vj];
			vz=127-((vi>>9)&127);
			if(!vz)
				continue;
			blk=ros_cel[vi&511];
			
			if((blk&255)<4)
				continue;
			
			ch=(vz>>4)*64+cy*8+cx;
			if(wascache[ch])
				continue;

			if(rgn->voxa[ch])
			{
				d=BTM_CameraDistanceToCoords(wrl,
					rcx+(vx&(~15)),
					rcy+(vy&(~15)),
					vz&(~15));
				if(d<drawfar)
					continue;
			}
			
			k=docache[ch];
			if(!k)
			{
				chid=BTMGL_ChidForCoords(rcx+vx, rcy+vy, vz);
				k=BTMGL_CheckRaIxForChid(chid);
				rgn->chkraix[ch]=k;
				if(!k)k=1;
				docache[ch]=k;
			}

			ebl=(blk>>12)&0xFF;
			esl=(blk>>20)&0xF;
			ebl|=ebl<<8;	esl|=esl<<8;
			ebl|=ebl<<16;	esl|=esl<<16;
			ebl|=ebl<<32;	esl|=esl<<32;
			
			fm=0x3F;
			
			fm&=~(1<<5);
			
			vi1=ros_map[(vj+1)&16383];
			if((vi1>>9)<=(vi>>9))fm&=~1;
			vi1=ros_map[(vj-1)&16383];
			if((vi1>>9)<=(vi>>9))fm&=~2;
			vi1=ros_map[(vj+128)&16383];
			if((vi1>>9)<=(vi>>9))fm&=~4;
			vi1=ros_map[(vj-128)&16383];
			if((vi1>>9)<=(vi>>9))fm&=~8;
			
			if(k>=256)
			{
				fm|=0x1000;
			}else
			{
//				continue;
			}
			
			BTMGL_EmitBlockFaces(vx+rcx, vy+rcy, vz, fm, blk, ebl, esl);
		}
	}
	
	for(pl=0; pl<4; pl++)
	{
		if(pl==0)
			if(vcx<(rcx+  0))
				continue;
		if(pl==1)
			if(vcx>(rcx+128))
				continue;
		if(pl==2)
			if(vcy<(rcy+  0))
				continue;
		if(pl==3)
			if(vcy>(rcy+128))
				continue;

		ros_map=rgn->ros_maps[pl];
		fhit=rgn->facehit+(((pl>>1)+1)<<6);
		
		if(!ros_map)
			continue;


		for(cz=0; cz<8; cz++)
			for(cx=0; cx<8; cx++)
		{
			if(!fhit[(cz<<3)+cx])
				continue;

			if(cz<4)
				continue;

			if(!(pl&2))
			{
				vx=rcx;
				if(!(pl&1))
					vx+=128;
				vy=rcy+cx*16;
			}else
			{
				vx=rcx+cx*16;
				vy=rcy;
				if(!(pl&1))
					vy+=128;
			}

#if 0
			dx=vcx-vx;			dy=vcy-vy;
			dx=dx^(dx>>31);		dy=dy^(dy>>31);
			d=dx+(dy>>1);
			if(dy>dx)	d=dy+(dx>>1);
			if(d>=btm_drawdist)
				continue;
#endif

			for(bz=0; bz<16; bz++)
				for(bx=0; bx<16; bx++)
			{
				if(!(pl&2))
				{
					//+X or -X (YZ plane)
					vy=cx*16+bx;
					vz=cz*16+bz;

					vj=vz*128+vy;
					vi=ros_map[vj];
					vx=127-((vi>>9)&127);
					if(!vx)
						continue;
					if(pl&1)
						vx=127-vx;
					blk=ros_cel[vi&511];
				}else
				{
					//+Y or -Y (XZ plane)
					vx=cx*16+bx;
					vz=cz*16+bz;

					vj=vz*128+vx;
					vi=ros_map[vj];
					vy=127-((vi>>9)&127);
					if(!vy)
						continue;
					if(pl&1)
						vy=127-vy;
					blk=ros_cel[vi&511];
				}
				
				if((pl==0) && (vx+rcx)>vcx)		continue;
				if((pl==1) && (vx+rcx)<vcx)		continue;
				if((pl==2) && (vy+rcy)>vcy)		continue;
				if((pl==3) && (vy+rcy)<vcy)		continue;
				
				if((blk&255)<4)
					continue;
				
				ch=(vz>>4)*64+(vy>>4)*8+(vx>>4);

				if(wascache[ch])
					continue;

				if(rgn->voxa[ch])
				{
					d=BTM_CameraDistanceToCoords(wrl, rcx+vx, rcy+vy, vz);
					if(d<drawfar)
						continue;
				}

				k=docache[ch];
				if(!k)
				{
					chid=BTMGL_ChidForCoords(rcx+vx, rcy+vy, vz);
					k=BTMGL_CheckRaIxForChid(chid);
					rgn->chkraix[ch]=k;
					if(!k)k=1;
					docache[ch]=k;
				}

				vj=vy*128+vx;
				vi1=ros_map_z[vj];
				vz1=127-(vi1>>9);
				if(vz==vz1)
				{
					/* skip block, already seen in +Z map */
					continue;
				}

				fm=0;
				fm|=1<<pl;

				if(k>=256)
				{
					fm|=0x1000;
				}else
				{
//					continue;
				}

				ebl=(blk>>12)&0xFF;
				esl=(blk>>20)&0xF;
				ebl|=ebl<<8;	esl|=esl<<8;
				ebl|=ebl<<16;	esl|=esl<<16;
				ebl|=ebl<<32;	esl|=esl<<32;

				BTMGL_EmitBlockFaces(vx+rcx, vy+rcy, vz, fm, blk, ebl, esl);
			}
		}
	}
#endif

#if 1
	for(cz=0; cz<8; cz++)
		for(cy=0; cy<8; cy++)
			for(cx=0; cx<8; cx++)
	{
		ch=cz*64+cy*8+cx;

		if(wascache[ch])
			continue;
		if(docache[ch]<256)
			continue;

		vx=rcx+cx*16;
		vy=rcy+cy*16;
		vz=cz*16;

		k=docache[ch];

		btmgl_blkemit_raix=BTMGL_RaIxForCoords(vx, vy, vz);
		
		n=btmgl_blkemitr_npts[k];
		for(i=0; i<n; i++)
		{
			BTMGL_EmitBlockVertex(
				btmgl_blkemitr_xyz[k]+i*3,
				btmgl_blkemitr_st[k]+i*2,
				btmgl_blkemitr_rgb[k][i], 0);
		}
	}
#endif

	return(0);
}

int BTM_DrawWorldFarBlocks(BTM_World *wrl)
{
	BTM_Region *rgn, *rnxt;

	rgn=wrl->region;
	while(rgn)
	{
		BTM_DrawRegionFarBlocks(wrl, rgn);
		rgn=rgn->next;
	}

	return(0);
}


int BTMGL_DrawSceneBlocks(BTM_World *wrl)
{
	BTM_Region *rgn;
	u32 *dbd;
	u64 *dbp;
	int *dbl;
//	u32 *vox;

	u64 cpos, lbl, lsl;
	u32 blk, blk1, blkd;
	u64 rcix, rcix1;

	int pos, cix, cix1, rix, rix1;
	int rgix, cx, cy, cz, vx, vy, vz;
	int px, py, pz, pd;
	int cxm, czm, xsh, zsh, xzshr;
	int cxfull, cxhalf, cxlqtr, cxhqtr;
	int ra, sina, cosa;
	int np, fm;
	int i, j, k;
	
//	xsh=wrl->xsh;
//	zsh=wrl->zsh;
//	cxm=(1<<xsh)-1;
//	czm=(1<<zsh)-1;
//	vox=wrl->vox;
	
//	xzshr=xsh+xsh;

	BTMGL_LockWorld();

	cxfull=1<<(16-0);
	cxhalf=1<<(16-1);
	cxlqtr=1<<(16-2);
	cxhqtr=cxhalf+cxlqtr;
	
	ra=wrl->cam_yaw&255;
	cosa=btmra_cosang[ra];
	sina=btmra_sinang[ra];

	vx=(wrl->cam_org>> 0)&0xFFFFFF;
	vy=(wrl->cam_org>>24)&0xFFFFFF;
	vz=(wrl->cam_org>>48)&0x00FFFF;
//	vx=(vx<< 8)>> 8;
//	vy=(vy<< 8)>> 8;
//	vz=(vz<<16)>>16;
	
	vx>>=8;
	vy>>=8;
	vz>>=8;

//	btmgl_blkemit_npts = 0;
//	btmgl_blkemita_npts = 0;

	for(i=0; i<64; i++)
	{
		btmgl_blkemitr_npts[i]=0;
	}
	
	if(btm_drawdist<=64)
	{
		btmgl_blkemit_raim=1;
		btmgl_blkemit_raicm=0;
	}
	else if(btm_drawdist<=128)	
	{
		btmgl_blkemit_raim=3;
//		btmgl_blkemit_raicm=255;
		btmgl_blkemit_raicm=511;
	}
	else if(btm_drawdist<=192)
	{
		btmgl_blkemit_raim=7;
//		btmgl_blkemit_raicm=511;
		btmgl_blkemit_raicm=1023;
	}
	else if(btm_drawdist<=256)
	{
		btmgl_blkemit_raim=15;
//		btmgl_blkemit_raicm=1023;
		btmgl_blkemit_raicm=2047;
	}
	else
	{
		btmgl_blkemit_raim=31;
		btmgl_blkemit_raicm=2047;
	}

	BTM_DrawWorldFarBlocks(wrl);
	
	rgn=NULL; rgix=-1;
//	vox=NULL;

	np=wrl->scr_npts;
	for(i=0; i<np; i++)
	{
		rcix=wrl->scr_pts_list[i];
		rix=BTM_Rcix2Rix(rcix);
		cix=BTM_Rcix2Cix(rcix);
		
		cpos=BTM_ConvRcixToBlkPos(rcix);
		cx=(cpos>> 0)&65535;
		cy=(cpos>>16)&65535;
		cz=(cpos>>32)&255;
		
//		cx=(cix>> 0)&127;
//		cy=(cix>> 7)&127;
//		cz=(cix>>14)&127;	
//		cx+=((rix>>0)&255)<<7;
//		cy+=((rix>>8)&255)<<7;

		if(rix!=rgix)
		{
//			rgn=BTM_GetRegionForRix(wrl, rix);
			rgn=BTM_LookupRegionForRix(wrl, rix);
			if(!rgn)
				continue;
			rgix=rix;
//			vox=rgn->vox;
		}

#if 1
		if(vx>cxhalf)
		{	if(cx<cxlqtr)
				cx+=cxfull;		}
		else
		{	if(cx>cxhqtr)
				cx-=cxfull;		}
		if(vy>cxhalf)
		{	if(cy<cxlqtr)
				cy+=cxfull;		}
		else
		{	if(cy>cxhqtr)
				cy-=cxfull;		}
#endif

//		blk=vox[cix];
//		blk=BTM_GetRegionBlockCix(wrl, rgn, cix);
		blk=BTM_WeakGetRegionBlockCix(wrl, rgn, cix);
		fm=0x3F;
//		lsl=0xFFFFFFFFFFFFULL;
		blkd=btmgl_vox_atlas_side[blk&255];

//		if(!(blkd&(BTM_BLKDFL_SEETHRU|BTM_BLKDFL_FLUID)))
//		if(!(blkd&BTM_BLKDFL_FLUID))
		if(!(blkd&BTM_BLKDFL_FLUID) && !(blkd&BTM_BLKDFL_TY_MASK))
		{
			if(vy>cy)
	//			fm&=~1;
				fm&=~8;
			else
				fm&=~4;

			if(vx>cx)
	//			fm&=~8;
				fm&=~2;
			else
	//			fm&=~2;
				fm&=~1;

			if(vz>=cz)
				fm&=~32;
			else
				fm&=~16;
		}

		if(blkd&BTM_BLKDFL_FLUID)
		{
			fm&=~15;
		}

		if(rcix==wrl->scr_lhit)
			fm|=64;

		lbl=0;
		lsl=0;
		
		for(j=0; j<6; j++)
		{
			if(!(fm&(1<<j)))
				continue;

			px=0; py=0; pz=0;
			switch(j)
			{
				case 0: px= 1; break;
				case 1: px=-1; break;
				case 2: py= 1; break;
				case 3: py=-1; break;
				case 4: pz= 1; break;
				case 5: pz=-1; break;
			}
			rcix1=BTM_BlockOffsetRcix(rcix, px, py, pz);
			rix1=BTM_Rcix2Cix(rcix1);

//			if(j==4)cix1=cix+(1<<14);
//			else if(j==5)cix1=cix-(1<<14);
//			else if(j==0)cix1=cix+1;
//			else if(j==1)cix1=cix-1;
//			else if(j==2)cix1=cix+(1<<7);
//			else if(j==3)cix1=cix-(1<<7);

			if(rix1==rix)
			{
				cix1=BTM_Rcix2Cix(rcix1);
//				blk1=BTM_GetRegionBlockCix(wrl, rgn, cix1);
				blk1=BTM_WeakGetRegionBlockCix(wrl, rgn, cix1);
			}else
			{
//				blk1=BTM_GetWorldBlockCix(wrl, rcix1);
//				blk1=BTM_TryGetWorldBlockCix(wrl, rcix1);
				blk1=BTM_TryWeakGetWorldBlockCix(wrl, rcix1);
			}

			k=(blk1>>12)&255LL;

#if 1
			if((blk1&0xFF)==BTM_BLKTY_WATER)
			{
				k=0x80|(k&15);

//				if((k&15)<8)
//					k=0x10|(k&15);
//				else
//					k=0x90|(k&15);
			}
#endif

//			lbl|=((blk1>>12)&63LL)<<(j*8);
			lbl|=((u64)k)<<(j*8);
//			lsl|=((blk1>>18)&63LL)<<(j*8);

//			k=(blk1>>18)&63;
			k=(blk1>>20)&15;
			k=(k*wrl->daylight)>>4;
			
			if((blk1&0xFF)==BTM_BLKTY_WATER)
			{
				k|=0x80;
//				if(k<8)
//					k|=0x10;
//				else
//					k|=0x90;
			}
			
			lsl|=((u64)k)<<(j*8);

		}
		
		if((blkd&BTM_BLKDFL_TY_MASK)==BTM_BLKDFL_TY_FULL)
		{
			BTMGL_EmitBlockFaces(cx, cy, cz, fm, blk, lbl, lsl);
		}else if((blkd&BTM_BLKDFL_TY_MASK)==BTM_BLKDFL_TY_SLAB)
		{
			BTMGL_EmitBlockFacesSlab(cx, cy, cz, fm, blk, lbl, lsl);
		}else if((blkd&BTM_BLKDFL_TY_MASK)==BTM_BLKDFL_TY_STAIR)
		{
			BTMGL_EmitBlockFacesStair(cx, cy, cz, fm, blk, lbl, lsl);
		}else if((blkd&BTM_BLKDFL_TY_MASK)==BTM_BLKDFL_TY_PLANT)
		{
			BTMGL_EmitBlockFacesPlant(cx, cy, cz, fm, blk, lbl, lsl);
		}else if((blkd&BTM_BLKDFL_TY_MASK)==BTM_BLKDFL_TY_CROP)
		{
			BTMGL_EmitBlockFacesPlant(cx, cy, cz, fm, blk, lbl, lsl);
		}
	}

	BTMGL_UnlockWorld();

	BTMGL_EmitLockArrays();
	
#ifdef BTM_RAYTHREAD

#if 1
	for(i=0; i<64; i++)
	{
		memcpy(
			btmgl_blkemitr2_xyz[i],
			btmgl_blkemitr_xyz[i],
			btmgl_blkemitr_npts[i]*3*4);
		memcpy(
			btmgl_blkemitr2_st[i],
			btmgl_blkemitr_st[i],
			btmgl_blkemitr_npts[i]*2*4);
		memcpy(
			btmgl_blkemitr2_rgb[i],
			btmgl_blkemitr_rgb[i],
			btmgl_blkemitr_npts[i]*1*4);
	}
#endif

#if 0
	memcpy(btmgl_blkemit2_xyz, btmgl_blkemit_xyz, btmgl_blkemit_npts*3*4);
	memcpy(btmgl_blkemit2_st, btmgl_blkemit_st, btmgl_blkemit_npts*2*4);
	memcpy(btmgl_blkemit2_rgb, btmgl_blkemit_rgb, btmgl_blkemit_npts*1*4);

	memcpy(btmgl_blkemita2_xyz, btmgl_blkemita_xyz, btmgl_blkemita_npts*3*4);
	memcpy(btmgl_blkemita2_st, btmgl_blkemita_st, btmgl_blkemita_npts*2*4);
	memcpy(btmgl_blkemita2_rgb, btmgl_blkemita_rgb, btmgl_blkemita_npts*1*4);
#endif

#endif

//	btmgl_blkemit2_npts=btmgl_blkemit_npts;
//	btmgl_blkemita2_npts=btmgl_blkemita_npts;

	for(i=0; i<64; i++)
	{
		btmgl_blkemitr2_npts[i]=btmgl_blkemitr_npts[i];
	}

	BTMGL_EmitUnlockArrays();

	return(0);
}

int BTMGL_DrawSceneArrays(BTM_World *wrl)
{
	int i, j, k;

//	if(btmgl_blkemit2_npts<=0)
//		return(0);

	for(i=0; i<64; i++)
	{
		if(btmgl_blkemitr2_npts[i]>0)
			break;
	}
	if(i>=32)
		return(0);

	BTMGL_EmitLockArrays();

#if 1
	for(i=0; i<64; i++)
	{
		if(btmgl_blkemitr2_npts[i]<=0)
			continue;

		k=2;
		if(i>=32)
			k=3;

		pglBindTexture(TKRA_TEXTURE_2D, k);

		pglEnableClientState(GL_VERTEX_ARRAY);
		pglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//	pglEnableClientState(GL_NORMAL_ARRAY);
		pglEnableClientState(GL_COLOR_ARRAY);

		pglVertexPointer(3, TKRA_GL_FLOAT, 3*4, btmgl_blkemitr2_xyz[i]+0);
		pglTexCoordPointer(2, TKRA_GL_FLOAT, 2*4, btmgl_blkemitr2_st[i]+0);
		pglColorPointer(4, TKRA_GL_UNSIGNED_BYTE, 4, btmgl_blkemitr2_rgb[i]+0);
		pglDrawArrays(TKRA_GL_QUADS, 0, btmgl_blkemitr2_npts[i]);

		pglDisableClientState(GL_VERTEX_ARRAY);
		pglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	//	pglDisableClientState(GL_NORMAL_ARRAY);
		pglDisableClientState(GL_COLOR_ARRAY);
	}
#endif

#if 0
	pglBindTexture(TKRA_TEXTURE_2D, 2);

	pglEnableClientState(GL_VERTEX_ARRAY);
	pglEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	pglEnableClientState(GL_NORMAL_ARRAY);
	pglEnableClientState(GL_COLOR_ARRAY);

	pglVertexPointer(3, TKRA_GL_FLOAT, 3*4, btmgl_blkemit2_xyz+0);
	pglTexCoordPointer(2, TKRA_GL_FLOAT, 2*4, btmgl_blkemit2_st+0);
	pglColorPointer(4, TKRA_GL_UNSIGNED_BYTE, 4, btmgl_blkemit2_rgb+0);
//	pglDrawArrays(TKRA_GL_TRIANGLES, 0, btmgl_blkemit_npts);
	pglDrawArrays(TKRA_GL_QUADS, 0, btmgl_blkemit2_npts);

	pglDisableClientState(GL_VERTEX_ARRAY);
	pglDisableClientState(GL_TEXTURE_COORD_ARRAY);
//	pglDisableClientState(GL_NORMAL_ARRAY);
	pglDisableClientState(GL_COLOR_ARRAY);


	pglBindTexture(TKRA_TEXTURE_2D, 3);

	pglEnableClientState(GL_VERTEX_ARRAY);
	pglEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	pglEnableClientState(GL_NORMAL_ARRAY);
	pglEnableClientState(GL_COLOR_ARRAY);

	pglVertexPointer(3, TKRA_GL_FLOAT, 3*4, btmgl_blkemita2_xyz+0);
	pglTexCoordPointer(2, TKRA_GL_FLOAT, 2*4, btmgl_blkemita2_st+0);
	pglColorPointer(4, TKRA_GL_UNSIGNED_BYTE, 4, btmgl_blkemita2_rgb+0);
//	pglDrawArrays(TKRA_GL_TRIANGLES, 0, btmgl_blkemit_npts);
	pglDrawArrays(TKRA_GL_QUADS, 0, btmgl_blkemita2_npts);

	pglDisableClientState(GL_VERTEX_ARRAY);
	pglDisableClientState(GL_TEXTURE_COORD_ARRAY);
//	pglDisableClientState(GL_NORMAL_ARRAY);
	pglDisableClientState(GL_COLOR_ARRAY);
#endif

	BTMGL_EmitUnlockArrays();

	return(0);
}

int btmgl_filter_min;
int btmgl_filter_max;

void BTMGL_UploadCompressed (
	byte *data, byte mipmap, byte alpha)
{
	BTMGL_DDS_HEADER *tdds;
	byte *cs, *css;
	int xs, ys;
	int fmin, fmax;
	int xshl, xshl1, isz, mip, txc;
	int i, j, k;
	
	if(!memcmp(data, "DDS ", 4))
//	if(1)
	{
		tdds = (void *)(data+4);
		css=data+4+(tdds->dwSize);
//		xshl=data[4];

		xs=tdds->dwWidth;
		i=xs; xshl=0;
		while(i>1)
			{ i>>=1; xshl++; }
		
		if(!(tdds->dwFlags&0x20000))
			mipmap=0;
	}else
	{
		printf("BTMGL_UploadCompressed: Not DDS\n");
		return;
	}

//	txc=GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	txc=TKRA_GL_CMPR_RGBA_S3TC_DXT1;
	if(!(alpha&1))
//		txc=GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		txc=TKRA_GL_CMPR_RGB_S3TC_DXT1;

	fmin=btmgl_filter_min;
	fmax=btmgl_filter_max;
	
	if(alpha&2)
	{
		fmax=GL_NEAREST;
	}
	

	cs=css; mip=0;
	while(xshl>=0)
	{
		if(mip && !mipmap)
			break;
	
		xshl1=xshl-2;
		if(xshl1<0)
			xshl1=0;
	
		isz=1<<(xshl1+xshl1+3);
		pglCompressedTexImage2D(
			GL_TEXTURE_2D, mip, 
			txc,
			1<<xshl, 1<<xshl, 0, isz, cs);
		cs+=isz;
		xshl--; mip++;
	}

	if (mipmap)
	{
		pglTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, fmin);
		pglTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER, fmax);

		pglTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MAX_LEVEL, 5);
//			GL_TEXTURE_MAX_LOD, 4);
	}
	else
	{
		pglTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, fmax);
		pglTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER, fmax);
	}
}

