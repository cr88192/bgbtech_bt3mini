float BTM_BmdLoad_DecodeF16A(u16 val)
{
	float f;
	u32 v0;
	int fr, ex, sg;
	
	sg=val>>15;
	ex=(val>>10)&31;
	fr=val&1023;
	ex=(ex-15)+127;
	fr=fr<<13;
	v0=(sg<<31)|(ex<<23)|fr;
	f=*(float *)(&v0);
	return(f);
}

float BTM_BmdLoad_DecodeF8A(byte val)
{
	float f;
	u32 v0;
	int sg, ex, fr;
	
	if(!(val&0x7F))
		return(0);
	
	sg=(val>>7);
	ex=(val>>4)&7;
	fr=val&15;
	ex=(ex-7)+15;
	fr=(fr<<6);

	ex=(ex-15)+127;
	fr=fr<<13;
	v0=(sg<<31)|(ex<<23)|fr;
	f=*(float *)(&v0);
	return(f);
}

int BTM_BmdLoad_DecodePackSe32(u32 val, float *xyz)
{
	float sc;
	int vx, vy, vz, ex, v;

	vx=((s32)(val<<23))>>23;
	vy=((s32)(val<<14))>>23;
	vz=((s32)(val<< 5))>>23;
	ex=(val>>27)&31;
	v=ex<<10;
	sc=BTM_BmdLoad_DecodeF16A(v)*(1.0/128);
	xyz[0]=vx*sc;
	xyz[1]=vy*sc;
	xyz[2]=vz*sc;
	return(0);
}

int BTM_BmdLoad_DecodePackQuat32(u32 val, float *xyzw)
{
	float tv[4];
	tv[0]=BTM_BmdLoad_DecodeF8A((val>> 0)&255);
	tv[1]=BTM_BmdLoad_DecodeF8A((val>> 8)&255);
	tv[2]=BTM_BmdLoad_DecodeF8A((val>>16)&255);
	tv[3]=BTM_BmdLoad_DecodeF8A((val>>24)&255);
	TKRA_Vec4F_Normalize(tv, xyzw);
	return(0);
}

byte BTM_ConvFloatToFP8A(float val)
{
	int fr, e, v;
	if(val<0)
		return(BTM_ConvFloatToFP8A(-val)|0x80);
	
	fr=val*(16*128); e=0;
	if(!fr)
		return(0);
	while(fr>=32)
		{ fr=(fr+1)>>1; e++; }
	if(e>=8)
		return(0x7F);
	v=(e<<4)|(fr&15);
	return(v);
}

u16 BTM_ConvFloatToFP16A(float val)
{
	u32 v0;
	int sg, fr, e, v;
	
	v0=*(u32 *)(&val);

	sg=(v0>>31)&1;
	fr=(v0>>13)&0x3FF;
	e=((v0>>23)&255)-127+15;
	if(e<=0)
		return(0);
	if(e>=31)
		return((sg<<15)|0x7C00);
		
	v=(sg<<15)|(e<<10)|(fr&1023);
	return(v);
}

u32 BTM_MeshEncodePackSe32(float *xyz)
{
	u32 v;
	s64 fr_x, fr_y, fr_z;
	int fr_e;
	
	fr_x=xyz[0]*(32768*128);
	fr_y=xyz[1]*(32768*128);
	fr_z=xyz[2]*(32768*128);
	
	fr_e=0;
	while(
		((fr_x^(fr_x>>63))>=254) ||
		((fr_y^(fr_y>>63))>=254) ||
		((fr_z^(fr_z>>63))>=254) )
	{
		if(fr_e>=30)
			break;
		fr_x=(fr_x+1)>>1;
		fr_y=(fr_y+1)>>1;
		fr_z=(fr_z+1)>>1;
		fr_e++;
	}
	
	if((fr_x^(fr_x>>63))>=255)
		fr_x=(fr_x>0)?255:(-255);
	if((fr_y^(fr_y>>63))>=255)
		fr_y=(fr_y>0)?255:(-255);
	if((fr_z^(fr_z>>63))>=255)
		fr_z=(fr_z>0)?255:(-255);

	v=	((fr_x&511)<< 0) |
		((fr_y&511)<< 9) |
		((fr_z&511)<<18) |
		((fr_e& 31)<<27);
	return(v);
}

u32 BTM_MeshEncodePackQuat32(float *xyzw)
{
	u32 v0, v1, v2, v3, v;
	
	v0=BTM_ConvFloatToFP8A(xyzw[0]);
	v1=BTM_ConvFloatToFP8A(xyzw[1]);
	v2=BTM_ConvFloatToFP8A(xyzw[2]);
	v3=BTM_ConvFloatToFP8A(xyzw[3]);
	v=v0|(v1<<8)|(v2<<16)|(v3<<24);
	return(v);
}

u32 BTM_BmdLoad_InterpolatePackQuat32(
	u32 val0, u32 val1, int ix0, int ix1, int ixd, int ixn)
{
	float tv0[4], tv1[4], tv2[4], tv3[4];
	float f;
	
	if(ix0==ix1)
		return(val0);

	if(ix0>ix1)
	{
		if(ixd>ix0)
		{
			return(BTM_BmdLoad_InterpolatePackQuat32(
				val0, val1, ix0, ix1+ixn, ixd, ixn));
		}else
		{
			return(BTM_BmdLoad_InterpolatePackQuat32(
				val0, val1, ix0-ixn, ix1, ixd, ixn));
		}
	}
	
	BTM_BmdLoad_DecodePackQuat32(val0, tv0);
	BTM_BmdLoad_DecodePackQuat32(val1, tv1);
	f=(1.0*(ixd-ix0))/(ix1-ix0);
	tv2[0]=(tv0[0]*(1.0-f))+(tv1[0]*f);
	tv2[1]=(tv0[1]*(1.0-f))+(tv1[1]*f);
	tv2[2]=(tv0[2]*(1.0-f))+(tv1[2]*f);
	tv2[3]=(tv0[3]*(1.0-f))+(tv1[3]*f);
	TKRA_Vec4F_Normalize(tv2, tv3);
	return(BTM_MeshEncodePackQuat32(tv3));
}

u32 BTM_BmdLoad_InterpolatePackSe32(
	u32 val0, u32 val1, int ix0, int ix1, int ixd, int ixn)
{
	float tv0[4], tv1[4], tv2[4], tv3[4];
	float f;
	
	if(ix0==ix1)
		return(val0);
	
	if(ix0>ix1)
	{
		if(ixd>ix0)
		{
			return(BTM_BmdLoad_InterpolatePackSe32(
				val0, val1, ix0, ix1+ixn, ixd, ixn));
		}else
		{
			return(BTM_BmdLoad_InterpolatePackSe32(
				val0, val1, ix0-ixn, ix1, ixd, ixn));
		}
	}
	
	BTM_BmdLoad_DecodePackSe32(val0, tv0);
	BTM_BmdLoad_DecodePackSe32(val1, tv1);
	f=(1.0*(ixd-ix0))/(ix1-ix0);
	tv2[0]=(tv0[0]*(1.0-f))+(tv1[0]*f);
	tv2[1]=(tv0[1]*(1.0-f))+(tv1[1]*f);
	tv2[2]=(tv0[2]*(1.0-f))+(tv1[2]*f);
//	TKRA_Vec3F_Normalize(tv2, tv3);
	return(BTM_MeshEncodePackSe32(tv2));
}

int BTM_BmdLoad_DecodeVertex(byte *buf, int vsz, u32 baseclr,
	float *xy, float *st, float *nv, u32 *cl, byte *bn)
{
	u64 v0, v1;
	float stsc;
	int tag, ste;

	v0=((u64 *)buf)[0];
	v1=((u64 *)buf)[1];
	tag=(v0>>56);

	if(tag==0)
	{
		ste=0;
	}

	if(tag<0xF0)
	{
		BTM_BmdLoad_DecodePackSe32(v0, xy);
//		st[0]=((v0>>32)&4095)*(1.0/2048)-0.5;
//		st[1]=((v0>>44)&4095)*(1.0/2048)-0.5;
		bn[0]=(v0>>56)&255;

//		if(tag&1)
		if(1)
		{
			if(baseclr&(1<<24))
			{
				st[0]=0;
				st[1]=0;
				nv[0]=BTM_BmdLoad_DecodeF8A(v0>>32);
				nv[1]=BTM_BmdLoad_DecodeF8A(v0>>40);
				nv[2]=BTM_BmdLoad_DecodeF8A(v0>>48);
				cl[0]=0xFF000000U|baseclr;
			}else
			{
				st[0]=((v0>>32)&2047)*(1.0/1024)-0.5;
				st[1]=((v0>>43)&2047)*(1.0/1024)-0.5;
				ste=(v0>>54)&3;

				stsc=1.0;
				if(ste==1)		stsc=4;
				if(ste==2)		stsc=16;
				if(ste==3)		stsc=64;
				st[0]*=stsc;
				st[1]*=stsc;

				cl[0]=0xFFFFFFFFU;
				TKRA_Vec3F_Normalize(xy, nv);
			}
		}

		cl[0]=
			( cl[0]     &0xFF00FF00) |
			((cl[0]>>16)&0x000000FF) |
			((cl[0]<<16)&0x00FF0000) ;
		return(2);
	}

	if(tag<0xFC)
	{
		BTM_BmdLoad_DecodePackSe32(v0, xy);

		st[0]=((v0>>32)&4095)*(1.0/2048)-0.5;
		st[1]=((v0>>44)&4095)*(1.0/2048)-0.5;

		if(tag&1)
		{
			st[0]=((v0>>32)&2047)*(1.0/1024)-0.5;
			st[1]=((v0>>43)&2047)*(1.0/1024)-0.5;
			ste=(v0>>54)&3;

			stsc=1.0;
			if(ste==1)		stsc=4;
			if(ste==2)		stsc=16;
			if(ste==3)		stsc=64;
			st[0]*=stsc;
			st[1]*=stsc;
		}

//		if(tag&1)
//		{
//			st[0]=((v0>>32)&4095)*(1.0/128)-16.0;
//			st[1]=((v0>>44)&4095)*(1.0/128)-16.0;
//		}

		nv[0]=BTM_BmdLoad_DecodeF8A(v1>> 0);
		nv[1]=BTM_BmdLoad_DecodeF8A(v1>> 8);
		nv[2]=BTM_BmdLoad_DecodeF8A(v1>>16);
		cl[0]=BTM_GetColorRgbForBlockLight((v1>>24)&255);
		bn[0]=0;
//		if(tag==0xFA)
		if(tag&2)
		{
			bn[0]=(v1>>24)&255;
			cl[0]=0xFFFFFFFFU;
		}

		cl[0]=
			( cl[0]     &0xFF00FF00) |
			((cl[0]>>16)&0x000000FF) |
			((cl[0]<<16)&0x00FF0000) ;
		return(3);
	}

	xy[0]=BTM_BmdLoad_DecodeF16A(v0>> 0);
	xy[1]=BTM_BmdLoad_DecodeF16A(v0>>16);
	xy[2]=BTM_BmdLoad_DecodeF16A(v0>>32);
	cl[0]=BTM_GetColorRgbForBlockLight((v0>>48)&255);
//	st[0]=((s16)(v1>> 0))*(1.0/4096);
//	st[1]=((s16)(v1>>16))*(1.0/4096);

	st[0]=((v1>> 0)&16383)*(1.0/4096)-2.0;
	st[1]=((v1>>14)&16383)*(1.0/4096)-2.0;
	ste=(v1>>28)&15;
	stsc=1<<ste;
	st[0]*=stsc;
	st[1]*=stsc;
	
	nv[0]=BTM_BmdLoad_DecodeF8A(v1>>32);
	nv[1]=BTM_BmdLoad_DecodeF8A(v1>>40);
	nv[2]=BTM_BmdLoad_DecodeF8A(v1>>48);
	bn[0]=(v1>>56)&255;

	cl[0]=
		( cl[0]     &0xFF00FF00) |
		((cl[0]>>16)&0x000000FF) |
		((cl[0]<<16)&0x00FF0000) ;
	return(4);
}

int BTM_BmdLoad_DecodeTriQuad(byte *buf, int tsz,
	u16 *vidx)
{
	u64 v;

	v=*(u64 *)buf;
	if((v&3)==0)
	{
		v=*(u32 *)buf;
		vidx[0]=(v>> 2)&1023;
		vidx[1]=(v>>12)&1023;
		vidx[2]=(v>>22)&1023;
		vidx[3]=vidx[2];
		return(1);
	}
	if((v&3)==1)
	{
		v=*(u32 *)buf;
		vidx[0]=(v>> 2)&127;
		vidx[1]=(v>> 9)&127;
		vidx[2]=(v>>16)&127;
		vidx[3]=(v>>23)&127;
		return(1);
	}
	if((v&3)==2)
	{
		vidx[0]=(v>> 2)&65535;
		vidx[1]=(v>>22)&65535;
		vidx[2]=(v>>42)&65535;
		vidx[3]=vidx[2];
	}
	if((v&3)==3)
	{
		vidx[0]=(v>> 2)&65535;
		vidx[1]=(v>>17)&65535;
		vidx[2]=(v>>32)&65535;
		vidx[2]=(v>>47)&65535;
	}
	return(2);
}

int BTM_BmdLoad_DecodeAnimFrames(BTM_BtModelAnim *anim, byte *buf)
{
	u32 *refrot, *reforg, *refscl;
	byte *cs;
	u32 dflscl;
	int bn, fr, mbn, mfr, tg;
	int i0r, i1r, i0b, i1b, i0s, i1s;
	int i0, i1, i2;
	
	mbn=anim->n_bones;
	mfr=anim->n_frames;

	anim->frm_rot=btm_malloc(mfr*mbn*sizeof(u32));
	anim->frm_org=btm_malloc(mfr*mbn*sizeof(u32));
	anim->frm_scl=btm_malloc(mfr*mbn*sizeof(u32));
	memset(anim->frm_rot, 0xFF, mfr*mbn*sizeof(u32));
	memset(anim->frm_org, 0xFF, mfr*mbn*sizeof(u32));
	memset(anim->frm_scl, 0xFF, mfr*mbn*sizeof(u32));

	refrot=btm_malloc(mfr*mbn*sizeof(u32));
	reforg=btm_malloc(mfr*mbn*sizeof(u32));
	refscl=btm_malloc(mfr*mbn*sizeof(u32));
	memcpy(refrot, anim->frm_rot, mfr*mbn*sizeof(u32));
	memcpy(reforg, anim->frm_org, mfr*mbn*sizeof(u32));
	memcpy(refscl, anim->frm_scl, mfr*mbn*sizeof(u32));

	dflscl=(15<<27)|(128<<0)|(128<<9)|(128<<18);

	fr=0; bn=0; cs=buf;
//	for(fr=0; fr<mfr; fr++)
	for(bn=0; bn<mbn; bn++)
	{
		if(!*cs)
		{
			cs++;
			refrot[0*mbn+bn]=0x70000000U;
			reforg[0*mbn+bn]=0x00000000U;
			refscl[0*mbn+bn]=dflscl;
			continue;
		}
	
		fr=0;
//		while(bn<mbn)
		while(fr<mfr)
		{
			tg=*cs++;
			if(!tg)
				break;
			fr+=tg&31;
			if(fr>=mfr)
				break;
			if(tg&0x80)
			{
				memcpy(anim->frm_rot+fr*mbn+bn, cs, 4);
				memcpy(refrot+fr*mbn+bn, cs, 4);
				cs+=4;
			}
			if(tg&0x40)
			{
				memcpy(anim->frm_org+fr*mbn+bn, cs, 4);
				memcpy(reforg+fr*mbn+bn, cs, 4);
				cs+=4;
			}
			if(tg&0x20)
			{
				memcpy(anim->frm_scl+fr*mbn+bn, cs, 4);
				memcpy(refscl+fr*mbn+bn, cs, 4);
				cs+=4;
			}
			fr++;
		}
	}

#if 0
	/* if nothing is set for a bone parm, set first frame to default value. */
	for(bn=0; bn<mbn; bn++)
	{
		for(fr=0; fr<mfr; fr++)
		{
			if(refrot[fr*mbn+bn]!=0xFFFFFFFFU)
				break;
		}
		if(fr>=mfr)
			{ refrot[0*mbn+bn]=0x70000000U; }
		for(fr=0; fr<mfr; fr++)
		{
			if(reforg[fr*mbn+bn]!=0xFFFFFFFFU)
				break;
		}
		if(fr>=mfr)
			{ reforg[0*mbn+bn]=0x00000000U; }

		for(fr=0; fr<mfr; fr++)
		{
			if(refscl[fr*mbn+bn]!=0xFFFFFFFFU)
				break;
		}
		if(fr>=mfr)
			{ refscl[0*mbn+bn]=dflscl; }
	}

	/* Interpolate values for each bone.
	   Scan backwards and forwards to find values to interpolate.
	   No interpolation if keyframe or only a single value exists. */
	for(bn=0; bn<mbn; bn++)
	{
		for(fr=0; fr<mfr; fr++)
		{
			for(i0=0; i0<mfr; i0++)
			{
				i1=fr-i0;
				if(i1<0)i1+=mfr;
				if(refrot[i1*mbn+bn]!=0xFFFFFFFFU)
					{ i0r=i1; break; }
			}
			for(i0=0; i0<mfr; i0++)
			{
				i1=fr+i0;
				if(i1>=mfr)i1-=mfr;
				if(refrot[i1*mbn+bn]!=0xFFFFFFFFU)
					{ i1r=i1; break; }
			}

			for(i0=0; i0<mfr; i0++)
			{
				i1=fr-i0;
				if(i1<0)i1+=mfr;
				if(reforg[i1*mbn+bn]!=0xFFFFFFFFU)
					{ i0b=i1; break; }
			}
			for(i0=0; i0<mfr; i0++)
			{
				i1=fr+i0;
				if(i1>=mfr)i1-=mfr;
				if(reforg[i1*mbn+bn]!=0xFFFFFFFFU)
					{ i1b=i1; break; }
			}

			for(i0=0; i0<mfr; i0++)
			{
				i1=fr-i0;
				if(i1<0)i1+=mfr;
				if(refscl[i1*mbn+bn]!=0xFFFFFFFFU)
					{ i0s=i1; break; }
			}
			for(i0=0; i0<mfr; i0++)
			{
				i1=fr+i0;
				if(i1>=mfr)i1-=mfr;
				if(refscl[i1*mbn+bn]!=0xFFFFFFFFU)
					{ i1s=i1; break; }
			}
			
			if(i0r==i1r)
			{
				anim->frm_rot[fr*mbn+bn]=refrot[i0r*mbn+bn];
			}else
			{
				anim->frm_rot[fr*mbn+bn]=
					BTM_BmdLoad_InterpolatePackQuat32(
						refrot[i0r*mbn+bn],
						refrot[i1r*mbn+bn],
						i0r, i1r, fr, mfr);
			}

			if(i0b==i1b)
			{
				anim->frm_org[fr*mbn+bn]=reforg[i0b*mbn+bn];
			}else
			{
				anim->frm_org[fr*mbn+bn]=
					BTM_BmdLoad_InterpolatePackSe32(
						reforg[i0b*mbn+bn],
						reforg[i1b*mbn+bn],
						i0b, i1b, fr, mfr);
			}

			if(i0s==i1s)
			{
				anim->frm_scl[fr*mbn+bn]=refscl[i0s*mbn+bn];
			}else
			{
				anim->frm_scl[fr*mbn+bn]=
					BTM_BmdLoad_InterpolatePackSe32(
						refscl[i0s*mbn+bn],
						refscl[i1s*mbn+bn],
						i0s, i1s, fr, mfr);
			}
		}
	}
#endif
	
	return(0);
}

BTM_BtModel *BTM_BmdLoadModelBuffer(byte *buf, int sz)
{
	char tb[256];
	char *s0, *s1;
	BTM_BtModel *mdl;
	int meshofs, meshsz, lodofs, lodsz, nlod;
	int vtxofs, vtxsz, trisofs, trissz, matofs;
	int boneofs, bonesz, animofs, animsz;
	int nmesh, nvtx, ntri, szvtx, sztri;
	int nbone, nanim, mabone;
	u32 prot, porg;
	float f, g;
	int i, j, k;
	
	meshofs	=*(u32 *)(buf+0x10);
	meshsz	=*(u32 *)(buf+0x14);
	lodofs	=*(u32 *)(buf+0x18);
	lodsz	=*(u32 *)(buf+0x1C);

	boneofs	=*(u32 *)(buf+0x20);
	bonesz	=*(u32 *)(buf+0x24);
	animofs	=*(u32 *)(buf+0x28);
	animsz	=*(u32 *)(buf+0x2C);

	nmesh=meshsz>>5;
	nlod=lodsz>>2;

	nbone=bonesz>>4;
	nanim=animsz>>4;

	mdl=btm_malloc(sizeof(BTM_BtModel));
	
	mdl->n_lod=nlod;
	mdl->n_mesh=nmesh;
	mdl->n_bone=nbone;
	mdl->n_anim=nanim;
	
	for(i=0; i<nlod; i++)
	{
		mdl->lod_base[i]=buf[lodofs+i*4+0];
		mdl->lod_cnt[i]=buf[lodofs+i*4+1];
		f=BTM_BmdLoad_DecodeF8A(buf[lodofs+i*4+2]);
		if(f!=0)	f=1.0/f;
		else		f=256.0;
		mdl->lod_dist[i]=f;
	}
	
	if(nlod<2)
	{
		if(!mdl->lod_base[0] && !mdl->lod_cnt[0])
		{
			mdl->lod_cnt[0]=nmesh;
		}

		nlod=1;
		mdl->n_lod=nlod;
	}

	for(i=0; i<nmesh; i++)
	{
		mdl->mesh[i]=btm_malloc(sizeof(BTM_BtModelMesh));
		vtxofs	=*(u32 *)(buf+(meshofs+i*32+0x00));
		trisofs	=*(u32 *)(buf+(meshofs+i*32+0x04));
		matofs	=*(u32 *)(buf+(meshofs+i*32+0x08));
		nvtx	=*(u16 *)(buf+(meshofs+i*32+0x0C));
		ntri	=*(u16 *)(buf+(meshofs+i*32+0x0E));
		vtxsz	=*(u32 *)(buf+(meshofs+i*32+0x10));
		trissz	=*(u32 *)(buf+(meshofs+i*32+0x14));
		if(nvtx)
			szvtx	=vtxsz/nvtx;
		if(ntri)
			sztri	=trissz/ntri;

		mdl->mesh[i]->matname=NULL;
		if(matofs)
		{
			strcpy(tb, (char *)(buf+matofs));
			s0=tb;
			if(!strncmp(s0, "resource/", 9))
				s0+=9;
			s1=s0+strlen(s0);
			if(*(s1-4)=='.')
				*(s1-4)=0;
			
			mdl->mesh[i]->matname=bccx_strdup(s0);
			
			mdl->mesh[i]->baseclr=0xFFFFFF;
			if(s0[0]=='#')
				{
					mdl->mesh[i]->baseclr=
						0x1000000|strtol(s0+1, NULL, 16);
				}
		}

		mdl->mesh[i]->v_xy=btm_malloc(nvtx*3*sizeof(float));
		mdl->mesh[i]->v_st=btm_malloc(nvtx*2*sizeof(float));
		mdl->mesh[i]->v_nv=btm_malloc(nvtx*3*sizeof(float));
		mdl->mesh[i]->v_cl=btm_malloc(nvtx*1*sizeof(u32));
		mdl->mesh[i]->v_bn=btm_malloc(nvtx*1*sizeof(byte));
		mdl->mesh[i]->tris=btm_malloc(nvtx*4*sizeof(u16));
		mdl->mesh[i]->n_vtx=nvtx;
		mdl->mesh[i]->n_tri=ntri;
		
		k=0;
		for(j=0; j<nvtx; j++)
		{
			k+=BTM_BmdLoad_DecodeVertex(
				buf+vtxofs+k*4, szvtx,
				mdl->mesh[i]->baseclr,
				mdl->mesh[i]->v_xy+j*3,
				mdl->mesh[i]->v_st+j*2,
				mdl->mesh[i]->v_nv+j*3,
				mdl->mesh[i]->v_cl+j*1,
				mdl->mesh[i]->v_bn+j*1);
		}
		
//		mdl->mesh[i]->v_bn[j-1]=mdl->mesh[i]->v_bn[j-2];

		k=0;
		for(j=0; j<ntri; j++)
		{
			k+=BTM_BmdLoad_DecodeTriQuad(
				buf+trisofs+k*4, sztri,
				mdl->mesh[i]->tris+j*4);
		}
	}
	
	if(!boneofs)
	{
		/* No bones, so done. */
		return(mdl);
	}

	mabone=0;
	for(i=0; i<nbone; i++)
	{
		mdl->bone[i]=btm_malloc(sizeof(BTM_BtModelBone));
		
		porg	=*(u32 *)(buf+(boneofs+i*16+0x00));
		prot	=*(u32 *)(buf+(boneofs+i*16+0x04));
		matofs	=*(u32 *)(buf+(boneofs+i*16+0x08));

		if(	(prot==0xFFFFFFFFU) ||
			(prot==0x00000000U))
				prot=0x70000000U;

		mdl->bone[i]->id_parent=buf[boneofs+i*16+0x0C];
		mdl->bone[i]->id_solid=buf[boneofs+i*16+0x0D];
		BTM_BmdLoad_DecodePackSe32(porg, mdl->bone[i]->baseorg);
		BTM_BmdLoad_DecodePackQuat32(prot, mdl->bone[i]->baserot);

		mdl->bone[i]->name=NULL;
		if(matofs)
			mdl->bone[i]->name=bccx_strdup((char *)(buf+matofs));
		
		if(mdl->bone[i]->id_parent<0xF0)
			mabone=i+1;
	}
	

	for(i=0; i<nanim; i++)
	{
		mdl->anim[i]=btm_malloc(sizeof(BTM_BtModelAnim));

		matofs	=*(u32 *)(buf+(animofs+i*16+0x00));
		vtxofs	=*(u32 *)(buf+(animofs+i*16+0x08));
		vtxsz	=*(u32 *)(buf+(animofs+i*16+0x0C));
		nvtx	=buf[animofs+i*16+0x04];
		
		mdl->anim[i]->name=NULL;
		if(matofs)
			mdl->anim[i]->name=bccx_strdup((char *)(buf+matofs));

		mdl->anim[i]->n_frames=nvtx;
		mdl->anim[i]->n_bones=mabone;
		BTM_BmdLoad_DecodeAnimFrames(mdl->anim[i], buf+vtxofs);
	}

	return(mdl);
}

int BTM_BmdGetTexture(char *matname)
{
	char tb[256];
	int tex;
	
	if(matname[0]=='#')
		return(0);
	
	sprintf(tb, "%s.dds", matname);
	tex=BTMGL_LoadTextureForName(tb);
	return(tex);
}

int BTM_BmdDrawModelBasic(BTM_BtModel *mdl)
{
	BTM_BtModelMesh *mesh;
	int mb, mn, mi, ti;
	
	mb=mdl->lod_base[0];
	mn=mb+mdl->lod_cnt[0];
	
	for(mi=mb; mi<mn; mi++)
	{
		mesh=mdl->mesh[mi];
	
		ti=BTM_BmdGetTexture(mesh->matname);
		if(ti>0)
		{
			pglEnable(GL_TEXTURE_2D);
			pglBindTexture(GL_TEXTURE_2D, ti);
		}else
		{
			pglDisable(GL_TEXTURE_2D);
		}

		pglEnableClientState(GL_VERTEX_ARRAY);
		pglEnableClientState(GL_TEXTURE_COORD_ARRAY);
//		pglEnableClientState(GL_NORMAL_ARRAY);
		pglEnableClientState(GL_COLOR_ARRAY);

		pglVertexPointer(3, TKRA_GL_FLOAT, 3*4, mesh->v_xy);
		pglTexCoordPointer(2, TKRA_GL_FLOAT, 2*4, mesh->v_st);
		pglNormalPointer(TKRA_GL_FLOAT, 3*4, mesh->v_nv);
		pglColorPointer(4, TKRA_GL_UNSIGNED_BYTE, 4, mesh->v_cl);
		pglDrawElements(TKRA_GL_QUADS, mesh->n_tri*4,
			TKRA_GL_UNSIGNED_SHORT, mesh->tris);

		pglDisableClientState(GL_VERTEX_ARRAY);
		pglDisableClientState(GL_TEXTURE_COORD_ARRAY);
//		pglDisableClientState(GL_NORMAL_ARRAY);
		pglDisableClientState(GL_COLOR_ARRAY);

		pglEnable(GL_TEXTURE_2D);
	}
	
	return(0);
}

BTM_BtModel *btm_bmdskelcache_mdl[256];
int btm_bmdskelcache_sqfr[256];
float *btm_bmdskelcache_trans[256];

float btm_bmdanim_boneorg[256*3];
float btm_bmdanim_bonerot[256*4];

float *BTM_BmdGetModelCachedSkelTrans(BTM_BtModel *mdl, int seq, int frm)
{
	float tmat[16], trot[4];
	BTM_BtModelAnim *acur;
	float *bonetrans;
	int h, sqfr;
	u32 pv_r, pv_b;
	int i0r, i1r, i0b, i1b;
	int i0, i1, fr, bn, mbn, mdlb;
	int i, j, k;

	sqfr=(seq<<8)|frm;
	mdlb=((long long)mdl);

	h=1;
	h=h*65521+sqfr+1;
	h=h*65521+mdlb+1;
	h=h*65521+sqfr+1;
	h=h*65521+mdlb+1;
	h=h*65521+1;
	h=(h>>16)&63;
	
	if(	(btm_bmdskelcache_mdl[h]==mdl) &&
		(btm_bmdskelcache_sqfr[h]==sqfr) &&
		btm_bmdskelcache_trans[h])
			return(btm_bmdskelcache_trans[h]);
	
	if(!btm_bmdskelcache_trans[h])
	{
		btm_bmdskelcache_trans[h]=btm_malloc(64*16*sizeof(float));
		memset(btm_bmdskelcache_trans[h], 0, 64*16*sizeof(float));
	}
	
	acur=mdl->anim[seq];
	bonetrans=btm_bmdskelcache_trans[h];

	btm_bmdskelcache_mdl[h]=mdl;
	btm_bmdskelcache_sqfr[h]=sqfr;

	if(!acur)
	{
		TKRA_Mat4F_Identity(bonetrans+0*16);
		for(i=1; i<mdl->n_bone; i++)
		{
			j=mdl->bone[i]->id_parent;
			QuatF_Multiply(
				btm_bmdanim_bonerot+i*4,
				mdl->bone[i]->baserot,
				trot);
			QuatF_ToMatrix(
				trot, tmat);
			tmat[12]+=mdl->bone[i]->baseorg[0];
			tmat[13]+=mdl->bone[i]->baseorg[1];
			tmat[14]+=mdl->bone[i]->baseorg[2];			
			TKRA_Mat4F_MatMult(
				tmat, bonetrans+j*16,
				bonetrans+i*16);
		}
	}

	fr=frm;
	mbn=acur->n_bones;
	
	if(acur->n_frames>0)
	{
		while(fr>=acur->n_frames)
			fr-=acur->n_frames;
	}

	for(bn=0; bn<mbn; bn++)
	{
		i0r=fr;	i1r=fr;
		i0b=fr;	i1b=fr;
		for(j=0; j<acur->n_frames; j++)
		{
			i0=fr-j;
			if(i0<0)
				i0+=acur->n_frames;
			if(acur->frm_rot[i0*mbn+bn]!=0xFFFFFFFFU)
				{ i0r=i0; break; }
		}
		for(j=0; j<acur->n_frames; j++)
		{
			i0=fr+j;
			if(i0>=acur->n_frames)
				i0-=acur->n_frames;
			if(acur->frm_rot[i0*mbn+bn]!=0xFFFFFFFFU)
				{ i1r=i0; break; }
		}

		for(j=0; j<acur->n_frames; j++)
		{
			i0=fr-j;
			if(i0<0)
				i0+=acur->n_frames;
			if(acur->frm_org[i0*mbn+bn]!=0xFFFFFFFFU)
				{ i0b=i0; break; }
		}
		for(j=0; j<acur->n_frames; j++)
		{
			i0=fr+j;
			if(i0>=acur->n_frames)
				i0-=acur->n_frames;
			if(acur->frm_org[i0*mbn+bn]!=0xFFFFFFFFU)
				{ i1b=i0; break; }
		}
		
		if(i0r==i1r)
		{
			pv_r=acur->frm_rot[i0r*mbn+bn];
			if(pv_r==0xFFFFFFFFU)
				pv_r=0x70000000U;
		}else
		{
			pv_r=BTM_BmdLoad_InterpolatePackQuat32(
				acur->frm_rot[i0r*mbn+bn],
				acur->frm_rot[i1r*mbn+bn],
				i0r, i1r, fr, acur->n_frames);
		}

		if(i0b==i1b)
		{
			pv_b=acur->frm_org[i0b*mbn+bn];
			if(pv_b==0xFFFFFFFFU)
				pv_b=0x00000000U;
		}else
		{
			pv_b=BTM_BmdLoad_InterpolatePackSe32(
				acur->frm_org[i0b*mbn+bn],
				acur->frm_org[i1b*mbn+bn],
				i0b, i1b, fr, acur->n_frames);
		}
		
		BTM_BmdLoad_DecodePackQuat32(pv_r, btm_bmdanim_bonerot+bn*4);
		BTM_BmdLoad_DecodePackSe32(pv_b, btm_bmdanim_boneorg+bn*3);
	}

	TKRA_Mat4F_Identity(bonetrans+0*16);
	for(i=1; i<mdl->n_bone; i++)
	{
		j=mdl->bone[i]->id_parent;
		
		QuatF_Multiply(
			btm_bmdanim_bonerot+i*4,
			mdl->bone[i]->baserot,
			trot);
		QuatF_ToMatrix(
//				skel->bone_baserot+i*4, tmat);
			trot, tmat);
		tmat[12]+=mdl->bone[i]->baseorg[0];
		tmat[13]+=mdl->bone[i]->baseorg[1];
		tmat[14]+=mdl->bone[i]->baseorg[2];
		tmat[12]+=btm_bmdanim_boneorg[i*3+0];
		tmat[13]+=btm_bmdanim_boneorg[i*3+1];
		tmat[14]+=btm_bmdanim_boneorg[i*3+2];
		
		TKRA_Mat4F_MatMult(
			tmat, bonetrans+j*16,
			bonetrans+i*16);
	}

	return(bonetrans);
}

BTM_BtModel *btm_bmdmeshcache_mdl[256];
int btm_bmdmeshcache_sqfr[256];
int btm_bmdmeshcache_nvtx[256];
float *btm_bmdmeshcache_xyz[256];
float *btm_bmdmeshcache_nv[256];
byte *btm_bmdmeshcache_cl[256];

byte BTM_BmdScaleByteFp(byte src, float f)
{
	int i;
	i=src*f;
	if(i<  0)	i=  0;
	if(i>255)	i=255;
	return(i);
}

int BTM_BmdGetModelCacheVertexTransIdx(BTM_BtModel *mdl,
	int midx, int seq, int frm)
{
	float tmat[16];
	float tv0[4], tv1[4], tv2[4];
	BTM_BtModelMesh *mesh;
	float *trans, *xyz, *nv;
	byte *cl, *mcl;
	float f;
	int h, sqfr, mdlbits, nvtx;
	int i, j, k;

	sqfr=(midx<<16)|(seq<<8)|frm;
	mdlbits=((long long)mdl);
	
	h=1;
	h=h*65521+sqfr+31;
	h=h*65521+mdlbits+31;
	h=h*65521+sqfr+31;
	h=h*65521+mdlbits+31;
	h=h*65521+31;
//	h=(h>>16)&63;
	h=(h>>16)&255;
	
//	if(	(btm_bmdmeshcache_mdl[h]==mdl) &&
//		(btm_bmdmeshcache_sqfr[h]==sqfr) &&
//		btm_bmdmeshcache_xyz[h])
//			return(h);

	mesh=mdl->mesh[midx];
	nvtx=mesh->n_vtx;
	
	if(
		btm_bmdmeshcache_xyz[h] &&
		(nvtx>btm_bmdmeshcache_nvtx[h]))
	{
		btm_free(btm_bmdmeshcache_xyz[h]);
		btm_free(btm_bmdmeshcache_nv[h]);
		btm_free(btm_bmdmeshcache_cl[h]);
	
		btm_bmdmeshcache_xyz[h]=NULL;
		btm_bmdmeshcache_nv[h]=NULL;
		btm_bmdmeshcache_cl[h]=NULL;
	}
	
	if(!btm_bmdmeshcache_xyz[h])
	{
		k=128;
		while(nvtx>k)
			k=k+(k>>1);

		btm_bmdmeshcache_xyz[h]=btm_malloc(k*3*sizeof(float));
		btm_bmdmeshcache_nv[h]=btm_malloc(k*3*sizeof(float));
		btm_bmdmeshcache_cl[h]=btm_malloc(k*4*sizeof(byte));
		btm_bmdmeshcache_nvtx[h]=k;
	}
	
	btm_bmdmeshcache_mdl[h]=mdl;
	btm_bmdmeshcache_sqfr[h]=sqfr;
		
	trans=BTM_BmdGetModelCachedSkelTrans(mdl, seq, frm);
	
	xyz=btm_bmdmeshcache_xyz[h];
	nv=btm_bmdmeshcache_nv[h];
	cl=btm_bmdmeshcache_cl[h];
	
	mcl=(byte *)(mesh->v_cl);
	
	for(i=0; i<nvtx; i++)
	{
		j=mesh->v_bn[i];

		if((j<0) || (j>=mdl->n_bone))
		{
			k=-1;
		}
		
		TKRA_Mat4F_Copy(trans+j*16, tmat);
		
		f=	tmat[0]+tmat[1]+tmat[2] +
			tmat[4]+tmat[5]+tmat[6] ;
		
		if(f==0)
		{
			k=-1;
		}
		
		TKRA_Vec3F_Copy(mesh->v_xy+i*3, tv0);
		QuatF_TransformPoint(tv0, tv1, tmat);
		TKRA_Vec3F_Copy(tv1, xyz+i*3);

		TKRA_Vec3F_Copy(mesh->v_nv+i*3, tv0);
		QuatF_TransformNormal(tv0, tv2, tmat);
		TKRA_Vec3F_Copy(tv2, nv+i*3);
		
		f=0.8+tv2[2]*0.4;
		
		cl[i*4+0]=BTM_BmdScaleByteFp(mcl[i*4+0], f);
		cl[i*4+1]=BTM_BmdScaleByteFp(mcl[i*4+1], f);
		cl[i*4+2]=BTM_BmdScaleByteFp(mcl[i*4+2], f);
		cl[i*4+3]=mcl[i*4+3];
	}

	return(h);

}

int BTM_BmdDrawModelSkel(BTM_BtModel *mdl, int seq, int frm)
{
	BTM_BtModelMesh *mesh;
	float *xyz, *nv;
	byte *cl;
	int mb, mn, mi, ti, h;
	
	mb=mdl->lod_base[0];
	mn=mb+mdl->lod_cnt[0];
	
	for(mi=mb; mi<mn; mi++)
	{
		mesh=mdl->mesh[mi];
	
		ti=BTM_BmdGetTexture(mesh->matname);
		if(ti>0)
		{
			pglEnable(GL_TEXTURE_2D);
			pglBindTexture(GL_TEXTURE_2D, ti);
		}else
		{
			pglDisable(GL_TEXTURE_2D);
		}

		h=BTM_BmdGetModelCacheVertexTransIdx(mdl, mi, seq, frm);
		xyz=btm_bmdmeshcache_xyz[h];
		nv=btm_bmdmeshcache_nv[h];
		cl=btm_bmdmeshcache_cl[h];

		pglEnableClientState(GL_VERTEX_ARRAY);
		pglEnableClientState(GL_TEXTURE_COORD_ARRAY);
//		pglEnableClientState(GL_NORMAL_ARRAY);
		pglEnableClientState(GL_COLOR_ARRAY);

		pglVertexPointer(3, TKRA_GL_FLOAT, 3*4, xyz);
		pglTexCoordPointer(2, TKRA_GL_FLOAT, 2*4, mesh->v_st);
		pglNormalPointer(TKRA_GL_FLOAT, 3*4, nv);
		pglColorPointer(4, TKRA_GL_UNSIGNED_BYTE, 4, cl);
		pglDrawElements(TKRA_GL_QUADS, mesh->n_tri*4,
			TKRA_GL_UNSIGNED_SHORT, mesh->tris);

		pglDisableClientState(GL_VERTEX_ARRAY);
		pglDisableClientState(GL_TEXTURE_COORD_ARRAY);
//		pglDisableClientState(GL_NORMAL_ARRAY);
		pglDisableClientState(GL_COLOR_ARRAY);

		pglEnable(GL_TEXTURE_2D);
	}
	
	return(0);
}

int BTM_BmdDrawModel(BTM_BtModel *mdl, int seq, int frm)
{
	if(mdl->n_bone<=1)
	{
		BTM_BmdDrawModelBasic(mdl);
		return(0);
	}
	
	BTM_BmdDrawModelSkel(mdl, seq, frm);
	return(0);
}

int BTM_ConvFloatToLB8S(float f)
{
	int i;
	i=f*127+128;
	if(i<  1)i=  1;
	if(i>255)i=255;
	return(i);
}

u32 BTM_MeshEncodePackQuat32_LB(float *xyzw)
{
	u32 v0, v1, v2, v3, v;
	
	v0=BTM_ConvFloatToLB8S(xyzw[0]);
	v1=BTM_ConvFloatToLB8S(xyzw[1]);
	v2=BTM_ConvFloatToLB8S(xyzw[2]);
	v3=BTM_ConvFloatToLB8S(xyzw[3]);
	v=v0|(v1<<8)|(v2<<16)|(v3<<24);
	return(v);
}

int BTM_BmdLoad_DecodePackQuat32_LB(u32 val, float *xyzw)
{
	float tv[4];
	tv[0]=(((val>> 0)&255)-128)*(1.0/127);
	tv[1]=(((val>> 8)&255)-128)*(1.0/127);
	tv[2]=(((val>>16)&255)-128)*(1.0/127);
	tv[3]=(((val>>24)&255)-128)*(1.0/127);
	TKRA_Vec4F_Normalize(tv, xyzw);
	return(0);
}

u32 BTM_MeshEncodePackQuat32_Trel(float *xyzw)
{
	static u32 bitab[128];
	float tv[4];
	float d, d1;
	u32 pv, pv1;
	int i;

	if(!bitab[0])
	{
		for(i=0; i<81; i++)
		{
//			pv =((((i/ 1)%3)-1)&255)<< 0;
//			pv+=((((i/ 3)%3)-1)&255)<< 8;
//			pv+=((((i/ 9)%3)-1)&255)<<16;
//			pv+=((((i/27)%3)-1)&255)<<24;

			pv =(((i/ 1)%3)-1)<< 0;
			pv+=(((i/ 3)%3)-1)<< 8;
			pv+=(((i/ 9)%3)-1)<<16;
			pv+=(((i/27)%3)-1)<<24;
			bitab[i]=pv;
		}
	}

	pv=BTM_MeshEncodePackQuat32(xyzw);
	BTM_BmdLoad_DecodePackQuat32(pv, tv);
	d=TKRA_Vec4F_Distance(xyzw, tv);

//	for(i=0; i<256; i++)
	for(i=0; i<81; i++)
	{
		pv1=pv+bitab[i];
//		pv1=pv+
//			((((i>>0)&3)-1)<< 0) +
//			((((i>>2)&3)-1)<< 8) +
//			((((i>>4)&3)-1)<<16) +
//			((((i>>6)&3)-1)<<24) ;
		BTM_BmdLoad_DecodePackQuat32(pv1, tv);
		d1=TKRA_Vec4F_Distance(xyzw, tv);
		if(d1<d)
		{
			pv=pv1;
			d=d1;
		}
	}
	
	return(pv);
}

int BTM_BmdDrawInit()
{
	static int init=0;
	float tv0[4], tv1[4], tv2[4];
	double eq8, eql, eq8t;
	float f, g, h;
	u32 pv;
	int i, j, k;
	
	if(init)
		return(0);
	init=1;
	
	tv0[0]=1.375;
	tv0[1]=0.295;
	tv0[2]=0.0625;
	pv=BTM_MeshEncodePackSe32(tv0);
	BTM_BmdLoad_DecodePackSe32(pv, tv1);
	
	pv=BTM_ConvFloatToFP16A(tv0[0]);
	tv1[0]=BTM_BmdLoad_DecodeF16A(pv);

	pv=BTM_ConvFloatToFP8A(tv0[1]);
	tv1[1]=BTM_BmdLoad_DecodeF8A(pv);

	tv2[0]=BTM_BmdLoad_DecodeF8A(0x7F);
	tv2[1]=BTM_BmdLoad_DecodeF8A(0xFF);

#if 0
	eq8=0; eql=0;
	eq8t=0;
	for(i=0; i<512; i++)
	{
		printf("%d/%d EqF8=%f EqLB=%f EqF8T=%f\r", i, 512, eq8, eql, eq8t);
		fflush(stdout);
		for(j=0; j<256; j++)
			for(k=0; k<64; k++)
		{
			f=i*(M_PI/256.0);
			g=j*(M_PI/256.0);
			h=k*(M_PI/32.0);
			tv0[0]=sin(f)*cos(g);
			tv0[1]=cos(f)*cos(g);
			tv0[2]=sin(g)*cos(h);
			tv0[3]=sin(h);
			TKRA_Vec4F_Normalize(tv0, tv0);
			
			pv=BTM_MeshEncodePackQuat32(tv0);
			BTM_BmdLoad_DecodePackQuat32(pv, tv1);		
			eq8+=TKRA_Vec4F_Distance(tv0, tv1);
			
			pv=BTM_MeshEncodePackQuat32_LB(tv0);
			BTM_BmdLoad_DecodePackQuat32_LB(pv, tv1);		
			eql+=TKRA_Vec4F_Distance(tv0, tv1);

			pv=BTM_MeshEncodePackQuat32_Trel(tv0);
			BTM_BmdLoad_DecodePackQuat32(pv, tv1);		
			eq8t+=TKRA_Vec4F_Distance(tv0, tv1);
			
		}
	}
	
	printf("EqF8=%f EqLB=%f EqF8T=%f\n", eq8, eql, eq8t);
#endif

	return(1);
}
