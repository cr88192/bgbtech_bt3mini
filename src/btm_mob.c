
int BTMGL_DrawEntityBasic(BTM_World *wrl,
	BTM_MobEntity *mob)
{
	float sorg[3], vorg[3], v_fw[3], v_rt[3];
	float so_lf[3], so_rt[3];
	float so_tlf[3], so_trt[3];

	float to_lf[2], to_rt[2];
	float to_tlf[2], to_trt[2];

	int cxfull, cxhalf, cxlqtr, cxhqtr;
	int cx, cy, cz, vx, vy, vz, tex, ll;
	int va, vda, vdr;
	float ox, oy, oz, os, ot;
	u32 rgb;

	cxfull=1<<(24-0);
	cxhalf=1<<(24-1);
	cxlqtr=1<<(24-2);
	cxhqtr=cxhalf+cxlqtr;

	vx=(wrl->cam_org>> 0)&0x00FFFFFF;
	vy=(wrl->cam_org>>24)&0x00FFFFFF;
	vz=(wrl->cam_org>>48)&0x0000FFFF;
	
	vorg[0]=vx*(1.0/256);
	vorg[1]=vy*(1.0/256);
	vorg[2]=vz*(1.0/256);
	
	cx=mob->org_x;
	cy=mob->org_y;
	cz=mob->org_z;

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

	ll=BTM_GetLightForXYZ(wrl, cx>>8, cy>>8, cz>>8);
	rgb=BTM_GetColorRgbForBlockLight(ll);

//	tex=mob->spr_id;
	tex=BTMGL_LoadSpriteForName(
		mob->spr_base, mob->spr_seq, mob->spr_frame/4);
	
	sorg[0]=cx*(1.0/256);
	sorg[1]=cy*(1.0/256);
	sorg[2]=cz*(1.0/256);
	
	v_fw[0]=sorg[0]-vorg[0];
	v_fw[1]=sorg[1]-vorg[1];
	v_fw[2]=0;

	BTM_V3F_Normalize(v_fw, v_fw);

	v_rt[0]=-v_fw[1];
	v_rt[1]= v_fw[0];
	v_rt[2]=0;
	
	v_rt[0]=(v_rt[0]+wrl->cam_rt[0])/2;
	v_rt[1]=(v_rt[1]+wrl->cam_rt[1])/2;
	
	va=atan2(v_fw[1], v_fw[0])*(128.0/M_PI);
//	va=atan2(v_fw[0], v_fw[1])*(128.0/M_PI);
//	vda=mob->yaw-va;
	vda=64-(va-mob->yaw);
//	vda=va-mob->yaw;
	
	switch((vda>>5)&7)
	{
		case 0: vdr=0; break;
		case 7: vdr=0; break;

//		case 1: vdr=2; break;
//		case 2: vdr=2; break;

		case 1: vdr=3; break;
		case 2: vdr=3; break;

		case 3: vdr=1; break;
		case 4: vdr=1; break;

//		case 5: vdr=3; break;
//		case 6: vdr=3; break;

		case 5: vdr=2; break;
		case 6: vdr=2; break;
	}
	
//	so_lf[0]=sorg[0]-(mob->spr_dxs*wrl->cam_rt[0]*0.5);
//	so_lf[1]=sorg[1]-(mob->spr_dxs*wrl->cam_rt[1]*0.5);
//	so_lf[2]=sorg[2]-(mob->spr_dxs*wrl->cam_rt[2]*0.5);

//	so_rt[0]=sorg[0]+(mob->spr_dxs*wrl->cam_rt[0]*0.5);
//	so_rt[1]=sorg[1]+(mob->spr_dxs*wrl->cam_rt[1]*0.5);
//	so_rt[2]=sorg[2]+(mob->spr_dxs*wrl->cam_rt[2]*0.5);

	so_lf[0]=sorg[0]-(mob->spr_dxs*v_rt[0]*0.5);
	so_lf[1]=sorg[1]-(mob->spr_dxs*v_rt[1]*0.5);
	so_lf[2]=sorg[2]-(mob->spr_dxs*v_rt[2]*0.5);

	so_rt[0]=sorg[0]+(mob->spr_dxs*v_rt[0]*0.5);
	so_rt[1]=sorg[1]+(mob->spr_dxs*v_rt[1]*0.5);
	so_rt[2]=sorg[2]+(mob->spr_dxs*v_rt[2]*0.5);

	so_tlf[0]=so_lf[0];
	so_tlf[1]=so_lf[1];
	so_tlf[2]=so_lf[2]+mob->spr_dzs;

	so_trt[0]=so_rt[0];
	so_trt[1]=so_rt[1];
	so_trt[2]=so_rt[2]+mob->spr_dzs;

	os=(mob->spr_frame&3)*0.25;
//	ot=0.0;
	ot=vdr*0.25;

	to_lf[0] =os+0.00;	to_lf[1] =ot+0.25;
	to_rt[0] =os+0.25;	to_rt[1] =ot+0.25;
	to_tlf[0]=os+0.00;	to_tlf[1]=ot+0.00;
	to_trt[0]=os+0.25;	to_trt[1]=ot+0.00;

	tkra_glBindTexture(TKRA_TEXTURE_2D, tex);
//	tkra_glColor4f(1.0, 1.0, 1.0, 1.0);
//	tkra_glColor4ubv((void *)&rgb);
	tkra_glColor4f(
//		((rgb    )&255)*(1.0/255),
		((rgb>>16)&255)*(1.0/255),
		((rgb>> 8)&255)*(1.0/255),
//		((rgb>>16)&255)*(1.0/255),
		((rgb>> 0)&255)*(1.0/255),
		1.0);
	
	tkra_glBegin(GL_QUADS);

	tkra_glTexCoord2fv(to_tlf);
	tkra_glVertex3fv(so_tlf);

	tkra_glTexCoord2fv(to_lf);
	tkra_glVertex3fv(so_lf);

	tkra_glTexCoord2fv(to_rt);
	tkra_glVertex3fv(so_rt);

	tkra_glTexCoord2fv(to_trt);
	tkra_glVertex3fv(so_trt);

	tkra_glEnd();
	
	return(0);
}

int btmgl_tex_rov=64;

char *btmgl_tex_names[1024];
int btmgl_tex_txn[1024];
int btmgl_tex_cnt=0;

int BTMGL_LoadTextureForName(char *name)
{
	byte *tbuf;
	int tex, sz;
	int i, j, k;
	
	for(i=0; i<btmgl_tex_cnt; i++)
	{
		if(!strcmp(btmgl_tex_names[i], name))
			return(btmgl_tex_txn[i]);
	}
	
	tbuf=BTM_LoadFile(name, &sz);
	if(tbuf)
	{
		tex=btmgl_tex_rov++;
		tkra_glBindTexture(TKRA_TEXTURE_2D, tex);
		BTMGL_UploadCompressed(tbuf, 1, 1);
		free(tbuf);
	}else
	{
		tex=0;
	}
	
	i=btmgl_tex_cnt++;
	btmgl_tex_names[i]=bccx_strdup(name);
	btmgl_tex_txn[i]=tex;
	return(tex);
}

int BTMGL_LoadSpriteForName(char *base, int seq, int frm)
{
	char tb[256];
	
	if(frm)
	{
		sprintf(tb, "sprites/%s_m%uf%u.dds", base, seq, frm);
	}else
	{
		sprintf(tb, "sprites/%s_m%u.dds", base, seq);
	}
	return(BTMGL_LoadTextureForName(tb));
}


bccx_cxstate	bccx_mobj;

BTM_MobEntity *BTM_AllocWorldMob(BTM_World *wrl)
{
	BTM_MobEntity *tmp;
	
	tmp=wrl->free_mobent;
	if(tmp)
	{
		wrl->free_mobent=tmp;
		memset(tmp, 0, sizeof(BTM_MobEntity));
		return(tmp);
	}
	
	tmp=malloc(sizeof(BTM_MobEntity));
	memset(tmp, 0, sizeof(BTM_MobEntity));
	return(tmp);
}

int BTM_RunTickMobNone(BTM_World *wrl, BTM_MobEntity *self)
{
	return(0);
}

int BTM_RunMobTickMove(BTM_World *wrl, BTM_MobEntity *self)
{
	float org[3], vel[3], ivel[3];
	float dtf, frc;
	float f0, f1, f2;
	int mvflag;
	
	dtf=0.1;

	org[0]=self->org_x*(1.0/256);
	org[1]=self->org_y*(1.0/256);
	org[2]=self->org_z*(1.0/256);

	vel[0]=self->vel_x*(1.0/256);
	vel[1]=self->vel_y*(1.0/256);
	vel[2]=self->vel_z*(1.0/256);

	ivel[0]=self->ivel_x*(1.0/256);
	ivel[1]=self->ivel_y*(1.0/256);
	ivel[2]=self->ivel_z*(1.0/256);

	mvflag=self->mvflag;

	vel[2]-=16*dtf;
	
	if(mvflag&4)
	{
		frc=1.0-4*dtf;
		vel[2]=vel[2]*frc+26*dtf;
	}else
		if(mvflag&2)
	{
		frc=1.0-2*dtf;
		vel[2]=vel[2]*frc+20*dtf;
	}

	frc=1.0-8*dtf;

	if(mvflag&2)
		frc=1.0-1*dtf;

	if(frc<0)
		frc=0;

	if(TKRA_Vec3F_DotProduct(vel, vel)<0.2)
		frc=0;

	if(mvflag&1)
	{
		f0=TKRA_Vec3F_DotProduct(vel, ivel);
		f1=TKRA_Vec3F_DotProduct(vel, vel)+0.01;
		f2=f0/f1;
	
		if(f2<=0.9)
		{
			vel[0]*=frc;
			vel[1]*=frc;
//					TKRA_Vec3F_Scale(cam_vel, frc, cam_vel);
		}
	}else
		if(mvflag&2)
	{
		f0=TKRA_Vec3F_DotProduct(vel, ivel);
		f1=TKRA_Vec3F_DotProduct(vel, vel)+0.01;
		f2=f0/f1;
	
		if(f2<=0.9)
		{
			vel[0]*=frc;
			vel[1]*=frc;
//					TKRA_Vec3F_Scale(cam_vel, frc, cam_vel);
		}
	}
	
	if(TKRA_Vec3F_DotProduct(vel, ivel)<49)
	{
		if(mvflag&1)
		{
			vel[0]+=ivel[0]*dtf*4.0;
			vel[1]+=ivel[1]*dtf*4.0;
			vel[2]+=ivel[2]*dtf*4.0;
		}else
			if(mvflag&2)
		{
			vel[0]+=ivel[0]*dtf*1.0;
			vel[1]+=ivel[1]*dtf*1.0;
			vel[2]+=ivel[2]*dtf*1.0;
		}else
		{
			vel[0]+=ivel[0]*dtf*0.1;
			vel[1]+=ivel[1]*dtf*0.1;
			vel[2]+=ivel[2]*dtf*0.1;
		}
	}

	BTM_CheckWorldMoveVelSz(wrl, dtf,
		org, vel,
		self->rad_x*(1.0/16),
		self->rad_z*(1.0/16),
		self->rad_ofs_z*(1.0/16),
		org, vel, &mvflag);
	
	self->org_x=org[0]*256;
	self->org_y=org[1]*256;
	self->org_z=org[2]*256;

	self->vel_x=vel[0]*256;
	self->vel_y=vel[1]*256;
	self->vel_z=vel[2]*256;

	self->mvflag=mvflag;


	if(	(self->orgl_x!=self->org_x) ||
		(self->orgl_y!=self->org_y) ||
		(self->orgl_z!=self->org_z) )
	{
		self->rgn->dirty|=32;
	}

	self->orgl_x=self->org_x;
	self->orgl_y=self->org_y;
	self->orgl_z=self->org_z;

	return(0);
}

u64 BTM_MobGetOriginPos(BTM_World *wrl, BTM_MobEntity *self)
{
	u64 cpos;
	int cx, cy, cz;

	cx=self->org_x;
	cy=self->org_y;
	cz=self->org_z;

	cpos=
		((cx&0x00FFFFFFULL)    ) |
		((cy&0x00FFFFFFULL)<<24) |
		((cz&0x0000FFFFULL)<<48) ;
	return(cpos);
}

u64 BTM_MobGetVelPos(BTM_World *wrl, BTM_MobEntity *self)
{
	u64 cpos;
	int cx, cy, cz;

	cx=self->vel_x;
	cy=self->vel_y;
	cz=self->vel_z;

	cpos=
		((cx&0x00FFFFFFULL)    ) |
		((cy&0x00FFFFFFULL)<<24) |
		((cz&0x0000FFFFULL)<<48) ;
	return(cpos);
}

u64 BTM_MobGetImpulsePos(BTM_World *wrl, BTM_MobEntity *self)
{
	u64 cpos;
	int cx, cy, cz;

	cx=self->ivel_x;
	cy=self->ivel_y;
	cz=self->ivel_z;

	cpos=
		((cx&0x00FFFFFFULL)    ) |
		((cy&0x00FFFFFFULL)<<24) |
		((cz&0x0000FFFFULL)<<48) ;
	return(cpos);
}

int BTM_CheckMobCanMoveSpot(BTM_World *wrl, BTM_MobEntity *self, u64 spot)
{
	u64 cpos;
	u32 blk0, blk1, blk2, blkc0, blkc1;
	
	cpos=BTM_MobGetOriginPos(wrl, self);

	blk0=BTM_GetWorldBlockCorg(wrl, spot);
	blk1=BTM_GetWorldBlockCorg(wrl, spot-(1LL<<56));
	blk2=BTM_GetWorldBlockCorg(wrl, spot+(1LL<<56));

	blkc0=BTM_GetWorldBlockCorg(wrl, cpos);
	blkc1=BTM_GetWorldBlockCorg(wrl, cpos-(1LL<<56));

	if((blk0&255)>=4)
	{
		if((blk2&255)<4)
		{
			return(2);
		}
		return(0);
	}

	if((blk1&255)<4)
	{
		return(0);
	}
	
	if(BTM_BlockIsFluidP(wrl, blk1))
	{
		if(BTM_BlockIsFluidP(wrl, blkc1))
		{
			return(1);
		}
		return(0);
	}
	
	return(1);
}

int BTM_RunTickMobBasic(BTM_World *wrl, BTM_MobEntity *self)
{
	u64 cpos, mvpos;
	int mvfl;

	BTM_RunMobTickMove(wrl, self);

	cpos=BTM_MobGetOriginPos(wrl, self);
	mvpos=cpos+BTM_MobGetImpulsePos(wrl, self);

	mvfl=BTM_CheckMobCanMoveSpot(wrl, self, mvpos);
	if(!mvfl)
	{
		self->mob_rtick=0;
	}

	if(self->mob_rtick>0)
	{
		self->mob_rtick--;
	}else
	{
		self->yaw=rand()&255;
		self->mob_rtick=128+(rand()&63);
		self->mob_mvtick=rand()&63;
	}

	if((self->vel_x+self->vel_y)!=0)
	{
		self->spr_frame=(self->spr_frame+1)&3;
	}

	if(self->mob_mvtick>0)
	{
		self->mob_mvtick--;
		self->ivel_x= 2*sin(self->yaw*(M_PI/128))*256;
		self->ivel_y=-2*cos(self->yaw*(M_PI/128))*256;
		if(mvfl&2)
		{
//			self->ivel_z=12;
			self->org_z+=384;
		}
	}else
	{
		self->ivel_x=0;
		self->ivel_y=0;
	}

	return(0);
}


int BTM_MobCalcDistanceXY(BTM_World *wrl,
	BTM_MobEntity *self, BTM_MobEntity *other)
{
	int dx, dy, d;
	
	dx=self->org_x-other->org_x;
	dy=self->org_y-other->org_y;
	if(dx<0)	dx^=dx>>31;
	if(dy<0)	dy^=dy>>31;
	if(dx>dy)
		d=dx+(dy>>1);
	else
		d=dy+(dx>>1);
	return(d);
}

int BTM_MobCalcDistanceZ(BTM_World *wrl,
	BTM_MobEntity *self, BTM_MobEntity *other)
{
	int d;
	d=self->org_z-other->org_z;
	if(d<0)	d^=d>>31;
	return(d);
}

int BTM_MobCheckHandleCollide(BTM_World *wrl,
	BTM_MobEntity *self, BTM_MobEntity *other)
{
	int dx, dy, dz, rx, rz, ix, iz;

	if(self==other)
		return(0);
		
	dx=BTM_MobCalcDistanceXY(wrl, self, other);
	rx=(self->rad_x+other->rad_x)<<4;
	if(dx>=rx)
		return(0);

	dz=BTM_MobCalcDistanceZ(wrl, self, other);
	rz=(self->rad_z+other->rad_z)<<4;
	if(dz>=rz)
		return(0);
		
	ix=rx-dx;
	iz=rz-dz;

	dx=self->org_x-other->org_x;
	dy=self->org_y-other->org_y;
	dz=self->org_z-other->org_z;

	self->vel_x+=(dx*ix)>>8;
	self->vel_y+=(dy*ix)>>8;
	self->vel_z+=(dz*iz)>>8;

	return(0);
}

int BTMGL_DrawEntityBasic(BTM_World *wrl,
	BTM_MobEntity *mob);

int BTM_RunSpawnerForEntity(BTM_World *wrl,
	BTM_MobEntity *mob)
{
	mob->Tick=BTM_RunTickMobNone;
	mob->Draw=BTMGL_DrawEntityBasic;
	mob->spr_dxs=1;
	mob->spr_dzs=1;

	if(!strcmp(mob->cname, "chicken"))
	{
		mob->Tick=BTM_RunTickMobBasic;
		mob->spr_base="chicken1";
	}

	if(!strcmp(mob->cname, "pig"))
	{
		mob->Tick=BTM_RunTickMobBasic;
		mob->spr_base="pig1";
		mob->spr_dxs=2;
		mob->spr_dzs=2;
	}

	if(!strcmp(mob->cname, "sheep"))
	{
		mob->Tick=BTM_RunTickMobBasic;
		mob->spr_base="cow1";
		mob->spr_dxs=2;
		mob->spr_dzs=2;
	}

	if(!strcmp(mob->cname, "cow"))
	{
		mob->Tick=BTM_RunTickMobBasic;
		mob->spr_base="cow1";
		mob->spr_dxs=3;
		mob->spr_dzs=3;
	}

	return(0);
}

int BTM_CheckForEntityClass(BTM_World *wrl,
	BTM_MobEntity *mob, int cls)
{
	if(!cls)
		return(1);
		
	if(cls==1)
	{
		if(!strcmp(mob->cname, "chicken"))
			return(1);
		if(!strcmp(mob->cname, "pig"))
			return(1);
		if(!strcmp(mob->cname, "cow"))
			return(1);
		if(!strcmp(mob->cname, "sheep"))
			return(1);
		return(0);
	}
	
	return(0);
}

int BTM_CountRegionEntityClass(BTM_World *wrl,
	BTM_Region *rgn, int cls)
{
	BTM_MobEntity *mob;
	int cnt;
	
	cnt=0;
	mob=rgn->live_entity;
	while(mob)
	{
		if(BTM_CheckForEntityClass(wrl, mob, cls))
			cnt++;
		mob=mob->next;
	}
	return(cnt);
}

bccx_cxstate	bccx_classname;
bccx_cxstate	bccx_org_x;
bccx_cxstate	bccx_org_y;
bccx_cxstate	bccx_org_z;
bccx_cxstate	bccx_yaw;
bccx_cxstate	bccx_pitch;

int BTM_SpawnRegionEntity(BTM_World *wrl,
	BTM_Region *rgn, BCCX_Node *ent)
{
	double org[3];
	BTM_MobEntity *mob;
	char *cname;
	int yaw, pitch;

	if(BCCX_TagIsCstP(ent, &bccx_mobj, "mobj"))
	{
		cname=BCCX_GetCst(ent, &bccx_classname, "classname");
		org[0]=BCCX_GetFloatCst(ent, &bccx_org_x, "org_x");
		org[1]=BCCX_GetFloatCst(ent, &bccx_org_y, "org_y");
		org[2]=BCCX_GetFloatCst(ent, &bccx_org_z, "org_z");
		yaw=BCCX_GetIntCst(ent, &bccx_yaw, "yaw");
		pitch=BCCX_GetIntCst(ent, &bccx_pitch, "pitch");
		
		mob=BTM_AllocWorldMob(wrl);
		
		mob->cname=cname;
		mob->org_x=org[0]*256;
		mob->org_y=org[1]*256;
		mob->org_z=org[2]*256;
		mob->yaw=yaw;
		mob->yaw=pitch;
		
		mob->orgl_x=mob->org_x;
		mob->orgl_y=mob->org_y;
		mob->orgl_z=mob->org_z;
		
		mob->wrl=wrl;
		mob->rgn=rgn;
		
		mob->next=rgn->live_entity;
		rgn->live_entity=mob;
		
		BTM_RunSpawnerForEntity(wrl, mob);
		return(0);
	}

	return(0);
}

int BTM_SpawnRegionEntities(BTM_World *wrl,
	BTM_Region *rgn, BCCX_Node *ents)
{
	BCCX_Node *ntmp;
	int na, ci;

	if(!ents)
		return(0);

	na=BCCX_GetNodeChildCount(ents);
	for(ci=0; ci<na; ci++)
	{
		ntmp=BCCX_GetNodeIndex(ents, ci);
		BTM_SpawnRegionEntity(wrl, rgn, ntmp);
	}

	return(0);
}

BCCX_Node *BTM_FlattenRegionLiveEntities(
	BTM_World *wrl, BTM_Region *rgn)
{
	BCCX_Node *ntmp, *elst;
	BTM_MobEntity *mob;

	mob=rgn->live_entity;
	if(!mob)
	{
		return(NULL);
	}

	elst=BCCX_New("entities");
	while(mob)
	{
		ntmp=BCCX_NewCst(&bccx_mobj, "mobj");
		BCCX_SetCst(ntmp, &bccx_classname, "classname", mob->cname);
		BCCX_SetFloatCst(ntmp, &bccx_org_x, "org_x", mob->org_x*(1.0/256));
		BCCX_SetFloatCst(ntmp, &bccx_org_y, "org_y", mob->org_y*(1.0/256));
		BCCX_SetFloatCst(ntmp, &bccx_org_z, "org_z", mob->org_z*(1.0/256));

		if(mob->yaw)
			BCCX_SetIntCst(ntmp, &bccx_yaw, "yaw", mob->yaw);
		if(mob->pitch)
			BCCX_SetIntCst(ntmp, &bccx_pitch, "pitch", mob->pitch);
		
		BCCX_Add(elst, ntmp);

		mob=mob->next;
	}

	return(elst);
}

int BTMGL_SpawnWorldMobClass(BTM_World *wrl, u64 cpos, int cls)
{
	float org[3];
	BTM_Region *rgn;
	BCCX_Node *ntmp;
	char *cname;
	int rix, j;
	
	rix=BTM_WorldCorgToRix(cpos);
	rgn=BTM_GetRegionForRix(wrl, rix);

	org[0]=((cpos>> 0)&0x00FFFFFF)*(1.0/256);
	org[1]=((cpos>>24)&0x00FFFFFF)*(1.0/256);
	org[2]=((cpos>>48)&0x0000FFFF)*(1.0/256);

	cname=NULL;
	
	if(cls==1)
	{
		j=rand()&3;
		switch(j)
		{
			case 0: cname="chicken"; break;
			case 1: cname="pig"; break;
			case 2: cname="cow"; break;
			case 3: cname="sheep"; break;
		}
	}

	if(cname)
	{
		ntmp=BCCX_NewCst(&bccx_mobj, "mobj");
		BCCX_SetCst(ntmp,
			&bccx_classname, "classname", cname);
		BCCX_SetFloatCst(ntmp,
			&bccx_org_x, "org_x", org[0]);
		BCCX_SetFloatCst(ntmp,
			&bccx_org_y, "org_y", org[1]);
		BCCX_SetFloatCst(ntmp,
			&bccx_org_z, "org_z", org[2]);

		BTM_SpawnRegionEntity(wrl, rgn, ntmp);
	}
	return(0);
}

int BTMGL_DrawWorldEntities(BTM_World *wrl)
{
	BTM_Region *rgn;
	BTM_MobEntity *mob;
	BCCX_Node	*ntmp;
	u64 rpos;
	int cx, cy, vx, vy, dx, dy, d;

	vx=(wrl->cam_org>> 8)&0xFFFF;
	vy=(wrl->cam_org>>32)&0xFFFF;

	rgn=wrl->region;
	while(rgn)
	{
		if(rgn->rgnix==0)
		{
			if(!rgn->live_entity)
			{
				ntmp=BCCX_NewCst(&bccx_mobj, "mobj");
				BCCX_SetCst(ntmp,
					&bccx_classname, "classname", "chicken");
				BCCX_SetFloatCst(ntmp,
					&bccx_org_x, "org_x", 127);
				BCCX_SetFloatCst(ntmp,
					&bccx_org_y, "org_y", 127);
				BCCX_SetFloatCst(ntmp,
					&bccx_org_z, "org_z", 96);

				BTM_SpawnRegionEntity(wrl, rgn, ntmp);
			}
		}
	
//		rpos=BTM_ConvRixToBlkPos(rgn->rgnix);
		rpos=BTM_ConvRixToBlkPosCenter(rgn->rgnix);
		cx=(rpos>> 0)&65535;
		cy=(rpos>>16)&65535;

		dx=(s16)(vx-cx);
		dy=(s16)(vy-cy);
		dx=dx^(dx>>31);
		dy=dy^(dy>>31);
//		d=dx+dy;
		if(dx>dy)
			d=dx+(dy>>1);
		else
			d=dy+(dx>>1);
		
		if(d>=256)
//		if(d>=384)
		{
			rgn=rgn->next;
			continue;
		}

		mob=rgn->live_entity;
		while(mob)
		{
			cx=(mob->org_x>>8)&65535;
			cy=(mob->org_y>>8)&65535;

			dx=(s16)(vx-cx);
			dy=(s16)(vy-cy);
			dx=dx^(dx>>31);
			dy=dy^(dy>>31);
			d=dx+dy;
			
			if(d>80)
			{
				mob=mob->next;
				continue;
			}

//			BTMGL_DrawEntity(wrl, mob);
			if(mob->Draw)
				mob->Draw(wrl, mob);
			mob=mob->next;
		}


//		BTM_BlockTickRegion(wrl, rgn);
		rgn=rgn->next;
	}

	return(0);
}
