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

// byte *wrl_zmap;
// byte *wrl_nmap;

BCCX_Node *BTM_LookupMenuNode(char *name, char *subname);

int btm_tgrand(BTM_World *wrl)
{
	u16 v;
	u64 seed1;
	
	seed1=(wrl->tg_curseed*0x0000FFD302A2F2D1ULL)+4093;
	wrl->tg_curseed=seed1;
	v=seed1>>48;
	return(v);
}

int BTM_SetupWorldSeed(BTM_World *wrl, u64 seed)
{
	int i, j, k;

	wrl->tg_baseseed=seed;
	wrl->tg_curseed=seed;

	for(i=0; i<4096; i++)
		btm_tgrand(wrl);

	for(j=0; j<16; j++)
	{
		wrl->tg_nmap[j]=malloc(256);
		for(i=0; i<256; i++)
		{
			wrl->tg_nmap[j][i]=btm_tgrand(wrl);
//			wrl->tg_nmap[j][i]=rand();
		}
	}
	return(0);
}

int BTM_SetupLocalSeedXY(BTM_World *wrl, int cx, int cy)
{
	u64 seed1;
	
	seed1=wrl->tg_curseed;
	seed1=(seed1*0x0000FFD302A2F2D1ULL)+cx;
	seed1=(seed1*0x0000FFD302A2F2D1ULL)+cy;
	seed1=(seed1*0x0000FFD302A2F2D1ULL)+cx;
	seed1=(seed1*0x0000FFD302A2F2D1ULL)+cy;
	seed1=(seed1*0x0000FFD302A2F2D1ULL)+0;
	seed1=(seed1*0x0000FFD302A2F2D1ULL)+0;
	wrl->tg_curseed=seed1;
	return(0);
}

float BTM_NoisePt8F(BTM_World *wrl,
	int ix, int iy, int mskx, int msky, int mn)
{
	byte *nmap;
	int ix1, iy1, p;
	float f;

	ix&=mskx;
	iy&=msky;
	nmap=wrl->tg_nmap[mn];

	ix1=(2*ix-iy);
	iy1=(2*iy-ix);
	p=iy1*15+ix1;
	f=nmap[p&255]*(1.0/128)-1.0;
	return(f);
}

float BTM_NoisePtXyz8F(BTM_World *wrl,
	int ix, int iy, int iz, int mskx, int msky, int mskz, int mn)
{
	byte *nmap;
	int ix1, iy1, iz1, p;
	float f;

	ix&=mskx;
	iy&=msky;
	iz&=mskz;
	nmap=wrl->tg_nmap[mn];

	ix1=(2*ix-iy);
	iy1=(2*iy-ix);
	iz1=(2*iz-(2*iy1-ix1));
	p=iz1*7+iy1*3+ix1;
	p=(p*31)>>5;
	f=nmap[p&255]*(1.0/128)-1.0;
	return(f);
}

float BTM_Noise8F(BTM_World *wrl,
	float x, float y, int mskx, int msky, int mn)
{
	float f0, f1, f2, f3;
	float f4, f5, f6;
	float fx, fy;
	int ix, iy;
	int p0, p1, p2, p3;

	ix=x;		iy=y;
	fx=x-ix;	fy=y-iy;

	f0=BTM_NoisePt8F(wrl, ix+0, iy+0, mskx, msky, mn);
	f1=BTM_NoisePt8F(wrl, ix+1, iy+0, mskx, msky, mn);
	f2=BTM_NoisePt8F(wrl, ix+0, iy+1, mskx, msky, mn);
	f3=BTM_NoisePt8F(wrl, ix+1, iy+1, mskx, msky, mn);
	f4=(1.0-fx)*f0+(fx*f1);
	f5=(1.0-fx)*f2+(fx*f3);
	f6=(1.0-fy)*f4+(fy*f5);
	return(f6);
}

float BTM_NoiseXyz8F(BTM_World *wrl,
	float x, float y, float z, int mskx, int msky, int mskz, int mn)
{
	float f0, f1, f2, f3;
	float f4, f5, f6, f7, f8;
	float fx, fy, fz;
	int ix, iy, iz;
	int p0, p1, p2, p3;

	ix=x;		iy=y;		iz=z;
	fx=x-ix;	fy=y-iy;	fz=z-iz;

	f0=BTM_NoisePtXyz8F(wrl, ix+0, iy+0, iz+0, mskx, msky, mskz, mn);
	f1=BTM_NoisePtXyz8F(wrl, ix+1, iy+0, iz+0, mskx, msky, mskz, mn);
	f2=BTM_NoisePtXyz8F(wrl, ix+0, iy+1, iz+0, mskx, msky, mskz, mn);
	f3=BTM_NoisePtXyz8F(wrl, ix+1, iy+1, iz+0, mskx, msky, mskz, mn);
	f4=(1.0-fx)*f0+(fx*f1);
	f5=(1.0-fx)*f2+(fx*f3);
	f6=(1.0-fy)*f4+(fy*f5);

	f0=BTM_NoisePtXyz8F(wrl, ix+0, iy+0, iz+1, mskx, msky, mskz, mn);
	f1=BTM_NoisePtXyz8F(wrl, ix+1, iy+0, iz+1, mskx, msky, mskz, mn);
	f2=BTM_NoisePtXyz8F(wrl, ix+0, iy+1, iz+1, mskx, msky, mskz, mn);
	f3=BTM_NoisePtXyz8F(wrl, ix+1, iy+1, iz+1, mskx, msky, mskz, mn);
	f4=(1.0-fx)*f0+(fx*f1);
	f5=(1.0-fx)*f2+(fx*f3);
	f7=(1.0-fy)*f4+(fy*f5);

	f8=(1.0-fz)*f6+(fz*f7);
	return(f8);
}

int btm_abs(int x)
{
	return((x<0)?(-x):x);
}

int BTM_GenTree(BTM_World *wrl, int cx, int cy, int cz)
{
	int i, j, k, d, h;

	h=3+(rand()&3);

	for(i=0; i<5; i++)
		for(j=0; j<5; j++)
			for(k=0; k<5; k++)
	{
		d=btm_abs(i-2)+btm_abs(j-2)+btm_abs(k-2);
		if(d>4)
			continue;
		BTM_SetWorldBlockNlXYZ(wrl,
			cx+(i-2), cy+(j-2), cz+(h-2)+k,
			BTM_BLKTY_LEAVES);
	}

	for(k=0; k<4; k++)
	{
		BTM_SetWorldBlockNlXYZ(wrl, cx, cy, cz+k,
			BTM_BLKTY_LOG);
	}
	
	return(0);
}

#define BTM_TGEN_BASEHEIGHT		64

float btm_abspow(float base, float exp)
{
	if(base<0)
		return(-pow(-base, exp));
	return(pow(base, exp));
}

int BTM_GenerateBaseHeightXY(BTM_World *wrl, int cx, int cy)
{
	int i, j, k, l;
	float f0, f1;
	
	i=cx;
	j=cy;

//	l=	BTM_Noise8F(i/32.0, j/32.0, 7, 7)*4 +
//		BTM_Noise8F(i/8.0, j/8.0, 31, 31)*2 +
//		BTM_Noise8F(i/2.0, j/2.0, 127, 127)*1 +
//		26;

	f0=
		BTM_Noise8F(wrl, i/128.0, j/128.0,   511,   511, 0)*12 +
		BTM_Noise8F(wrl, i/ 32.0, j/ 32.0,  2047,  2047, 0)* 6 +
		BTM_Noise8F(wrl, i/  8.0, j/  8.0,  8191,  8191, 0)* 3 +
		BTM_Noise8F(wrl, i/  2.0, j/  2.0, 32767, 32767, 0)* 1 ;

	f1=
		BTM_Noise8F(wrl, i/128.0, j/128.0,   511,   511, 1)* 0.75 +
		BTM_Noise8F(wrl, i/ 32.0, j/ 32.0,  2047,  2047, 1)* 0.25 +
		1.25;

//	f0=btm_abspow(f0, 1.5);
//	f0=btm_abspow(f0, 1.25);
	f0=btm_abspow(f0, f1);

	if(f0>16)
	{
		f0=sqrt(f0-16)+16;
	}

	if(f0<(-16))
	{
		f0=(-16)-sqrt((-16)-f0);
	}


	l=	f0 + (BTM_TGEN_BASEHEIGHT+4);
	
	if(l<24)
		l=24;
	if(l>112)
		l=112;
	
	return(l);
}

int BTM_GenerateBaseXY(BTM_World *wrl, int cx, int cy)
{
	float f0, f1;
	u32 blk;
	int i, j, k, l, lc, ld;
	
	i=cx;
	j=cy;

	l=BTM_GenerateBaseHeightXY(wrl, cx, cy);

	for(k=0; k<128; k++)
		{ BTM_SetWorldBlockNlXYZ(wrl, i, j, k, BTM_BLKTY_AIR2); }

//	for(k=l; k<24; k++)
//	for(k=l; k<48; k++)
	for(k=l; k<BTM_TGEN_BASEHEIGHT; k++)
		{ BTM_SetWorldBlockNlXYZ(wrl, i, j, k, BTM_BLKTY_WATER); }

	for(k=l-4; k<l; k++)
	{
//		if(l>(48+1))
		if(l>(BTM_TGEN_BASEHEIGHT+1))
			BTM_SetWorldBlockNlXYZ(wrl, i, j, k, BTM_BLKTY_DIRT);
		else
			BTM_SetWorldBlockNlXYZ(wrl, i, j, k, BTM_BLKTY_SAND);
	}

	for(k=0; k<l-3; k++)
		{ BTM_SetWorldBlockNlXYZ(wrl, i, j, k, BTM_BLKTY_STONE); }

	if(l>(BTM_TGEN_BASEHEIGHT+1))
		{ BTM_SetWorldBlockNlXYZ(wrl, i, j, l-1, BTM_BLKTY_GRASS); }
	else
		{ BTM_SetWorldBlockNlXYZ(wrl, i, j, l-1, BTM_BLKTY_SAND); }

	for(k=0; k<l; k++)
	{
	
		if(k<(l-3))
		{
			lc=	BTM_NoiseXyz8F(wrl,
					i/8.0, j/8.0, k/8.0, 8191, 8191, 15, 1)*16 +
				BTM_NoiseXyz8F(wrl,
					i/2.0, j/2.0, k/2.0, 32767, 32767, 63, 1)* 4 ;
			ld=	BTM_NoiseXyz8F(wrl,
					i/8.0, j/8.0, k/8.0, 8191, 8191, 15, 2)*16 +
				BTM_NoiseXyz8F(wrl,
					i/2.0, j/2.0, k/2.0, 32767, 32767, 63, 2)* 4 ;

			if(lc>=10)
			{
				if(ld>=10)
				{
					BTM_SetWorldBlockNlXYZ(wrl, i, j, k, BTM_BLKTY_STONE3);
				}else if(ld>=4)
				{
					BTM_SetWorldBlockNlXYZ(wrl, i, j, k, BTM_BLKTY_STONE2);
				}else if((ld<-10))
				{
					BTM_SetWorldBlockNlXYZ(wrl, i, j, k, BTM_BLKTY_DIRT);
				}
			}
		}

		f0=	
			BTM_Noise8F(wrl, i/32.0, j/32.0, 2047, 2047, 2)*10 +
			BTM_NoiseXyz8F(wrl, i/8.0, j/8.0, k/8.0, 8191, 8191, 255, 0)*7 +
			BTM_NoiseXyz8F(wrl, i/2.0, j/2.0, k/2.0, 32767, 32767, 255, 0)*3 ;

//		if(lc>=14)
//		if(lc>=8)
//		if(lc>=(10-((l-64)>>3)))
//		if(lc>=(9-((l-64)>>4)))
		if(f0>=(11-((l-64)>>4)))
		{
			BTM_SetWorldBlockNlXYZ(wrl, i, j, k, BTM_BLKTY_AIR1);
		}
	}

	BTM_SetWorldBlockNlXYZ(wrl, i, j, 0, BTM_BLKTY_HARDSTONE);
	
	if(btm_tgrand(wrl)&1)
	{
		BTM_SetWorldBlockNlXYZ(wrl, i, j, 1, BTM_BLKTY_HARDSTONE);
	}


#if 0
//		if(!(rand()&63))
//	if(!(rand()&255) && (l>25))
//	if(!(rand()&255) && (l>(48+1)))
//	{
//		BTM_GenTree(wrl, i, j, l);
//	}

	for(k=127; k>0; k--)
	{
		blk=BTM_GetWorldBlockXYZ(wrl, i, j, k);
		if(blk!=BTM_BLKTY_AIR2)
			break;
		
//		BTM_SetWorldBlockNlXYZ(wrl, i, j, k, BTM_BLKTY_AIR2);
	}
#endif

	return(0);
}

int BTM_GenerateBaseRegion(BTM_World *wrl, BTM_Region *rgn)
{
	u64 rpos, rcix;
	u32 blk;
	int cx, cy, x, y, z, cix;
	int l;
	
	blk=BTM_GetRegionBlockCix(wrl, rgn, 0);

//	if(rgn->vox[0])
//	if(blk)
	if(blk&255)
		return(0);
	
	rgn->dirty|=4;
	
	rpos=BTM_ConvRixToBlkPos(rgn->rgnix);
	cx=(rpos>> 0)&65535;
	cy=(rpos>>16)&65535;
	
	BTM_SetupLocalSeedXY(wrl, cx, cy);
	
	for(y=0; y<128; y++)
		for(x=0; x<128; x++)
	{
		BTM_GenerateBaseXY(wrl, cx+x, cy+y);
	}

	for(y=0; y<128; y++)
		for(x=0; x<128; x++)
	{
//		BTM_GenerateBaseXY(wrl, cx+x, cy+y);

		if(!(btm_tgrand(wrl)&255) &&
			(x>2) && (x<126) &&
			(y>2) && (y<126))
		{
			l=BTM_GenerateBaseHeightXY(wrl, cx+x, cy+y);
			if(l>(BTM_TGEN_BASEHEIGHT+1))
				BTM_GenTree(wrl, cx+x, cy+y, l);
		}
	}

	for(y=0; y<128; y++)
		for(x=0; x<128; x++)
	{
		for(z=127; z>8; z--)
		{
			blk=BTM_GetWorldBlockXYZ(wrl, cx+x, cy+y, z);
			if(blk!=BTM_BLKTY_AIR2)
				break;
//			blk|=0x0F<<18;
			blk|=0x0F<<20;
			BTM_SetWorldBlockNlXYZ(wrl, cx+x, cy+y, z, blk);
		}

		for(z=1; z<16; z++)
		{
			blk=BTM_GetWorldBlockXYZ(wrl, cx+x, cy+y, z);
			if(blk!=BTM_BLKTY_AIR1)
				continue;
			blk=BTM_BLKTY_LAVA;
			BTM_SetWorldBlockNlXYZ(wrl, cx+x, cy+y, z, blk);
		}
	}

	for(z=0; z<128; z++)
		for(y=0; y<128; y++)
			for(x=0; x<128; x++)
	{
		cix=(z<<14)|(y<<7)|x;
		rcix=(((u64)rgn->rgnix)<<21)|cix;
		BTM_UpdateWorldBlockOccCix(wrl, rcix);
	}

	for(z=12; z<112; z++)
		for(y=8; y<120; y++)
			for(x=8; x<120; x++)
	{
		BTM_UpdateBlockLightForRCix(wrl, rcix);
	}

	return(0);
}

int BTM_InstFill(BTM_World *wrl,
	int mcx, int mcy, int mcz,
	int ncx, int ncy, int ncz,
	u32 blk)
{
	int x, y, z;

	for(z=mcz; z<=ncz; z++)
		for(y=mcy; y<=ncy; y++)
			for(x=mcx; x<=ncx; x++)
	{
		BTM_SetWorldBlockNlXYZ(wrl, +x, y, z, blk);
	}

	return(0);
}

char *btm_curtopname;

int BTM_InstanceStructureNodeAt(BTM_World *wrl,
	int bcx, int bcy, int bcz, BCCX_Node *node)
{
	BCCX_Node *c;
	char *bty, *s0, *s1, *s2;
	char *otop;
	u32 blk;
	int na, ci;
	int mx, my, mz, nx, ny, nz;

	if(!node)
		return(0);

	if(BCCX_TagIsP(node, "structure") ||
		BCCX_TagIsP(node, "list") ||
		BCCX_TagIsP(node, "object"))
	{
		na=BCCX_GetNodeChildCount(node);
		for(ci=0; ci<na; ci++)
		{
			c=BCCX_GetNodeIndex(node, ci);
			BTM_InstanceStructureNodeAt(wrl, bcx, bcy, bcz, c);
		}
		return(0);
	}

	if(BCCX_TagIsP(node, "random"))
	{
		na=BCCX_GetNodeChildCount(node);
		if(na<=0)
			return(0);

		ci=rand()%na;
		c=BCCX_GetNodeIndex(node, ci);
		BTM_InstanceStructureNodeAt(wrl, bcx, bcy, bcz, c);
		return(0);
	}

	if(BCCX_TagIsP(node, "fill"))
	{
		mx=BCCX_GetInt(node, "min_x");
		my=BCCX_GetInt(node, "min_y");
		mz=BCCX_GetInt(node, "min_z");
		nx=BCCX_GetInt(node, "max_x");
		ny=BCCX_GetInt(node, "max_y");
		nz=BCCX_GetInt(node, "max_z");

		bty=BCCX_Get(node, "block");
		blk=BTM_BlockForName(wrl, bty);
		if(!blk)
			return(0);
		
		BTM_InstFill(wrl,
			bcx+mx, bcy+my, bcz+mz,
			bcx+nx, bcy+ny, bcz+nz,
			blk);
		return(0);
	}

	if(BCCX_TagIsP(node, "instance"))
	{
		mx=BCCX_GetInt(node, "rel_x");
		my=BCCX_GetInt(node, "rel_y");
		mz=BCCX_GetInt(node, "rel_z");

		s1=BCCX_Get(node, "name");
		if(!s1)
			s1=btm_curtopname;
		
		if(!s1)
			return(0);
		
		s2=BCCX_Get(node, "subname");
		
		otop=btm_curtopname;
		btm_curtopname=s1;
		
		c=BTM_LookupMenuNode(s1, s2);
		BTM_InstanceStructureNodeAt(wrl, bcx+mx, bcy+my, bcz+mz, c);
		
		btm_curtopname=otop;
		return(0);
	}

	return(0);
}

int BTM_InstanceStructureAt(BTM_World *wrl,
	int bcx, int bcy, int bcz, char *name, char *subname)
{
	BCCX_Node *c;
	char *otop;

	otop=btm_curtopname;
	btm_curtopname=name;
	
	c=BTM_LookupMenuNode(name, subname);
	BTM_InstanceStructureNodeAt(wrl, bcx, bcy, bcz, c);
	
	btm_curtopname=otop;
	return(0);
}
