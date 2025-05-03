#define BTM_NOMAIN
#include "btm_multi.c"
#include "src_mesh/btm_meshmulti.c"

u16 btm_cam_yaw;
s16 btm_cam_pitch;

char *btm_modelname;

char *btm_texnames[4096];
int btm_texnums[4096];
int btm_texalloc=64;

BTM_SolidMesh *btm_meshbase;

int BTM_GetTextureIdForName(char *str)
{
	char *cs;
	byte *tex0;
	int i, k, h, txs, tys;
	
	cs=str; h=1;
	while(*cs)
		{ h=h*251+(*cs++); }
	h=(((h+1)*251)>>8)&4095;

	if(btm_texnames[h] && !strcmp(btm_texnames[h], str))
		return(btm_texnums[h]);

	k=btm_texalloc++;
	tex0=BTIC1H_Img_LoadTGA5551(str, &txs, &tys);
	pglEnable(GL_TEXTURE_2D);
	pglBindTexture(TKRA_TEXTURE_2D, k);
	pglTexImage2D(TKRA_TEXTURE_2D, 0, 4, txs, tys, 0,
		TKRA_RGBA, TKRA_GL_UNSIGNED_SHORT_5_5_5_1, tex0);

	pglTexParameterf(GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	pglTexParameterf(GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	btm_texnames[h]=bccx_strdup(str);
	btm_texnums[h]=k;
	return(k);
}

static float btm_bonetrans[256*16];

float btm_doanim_boneorg[256*3];
float btm_doanim_bonerot[256*4];
int btm_doanim_bonezeroed;


BTM_SolidSkel *btm_doanim_skel;
char *btm_doanim_name;
int btm_doanim_frame;
int btm_doanim_dtfrac;

int BTM_DoAnimUpdate_ZeroBones()
{
	int i;

	if(!btm_doanim_bonezeroed)
	{
		for(i=0; i<256; i++)
		{
			TKRA_Vec3F_Zero(btm_doanim_boneorg+i*3);
			QuatF_Identity(btm_doanim_bonerot+i*4);
		}
		btm_doanim_bonezeroed=1;
	}
	return(1);
}

int BTM_DoAnimUpdate(int dt)
{
	BTM_BtModelAnim *acur;
	u32 pv_r, pv_b;
	int i0r, i1r, i0b, i1b;
	int i0, i1, fr, bn, mbn;
	int i, j, k;

	if(!btm_doanim_skel || !btm_doanim_name)
	{
		BTM_DoAnimUpdate_ZeroBones();
		return(0);
	}

	if(btm_doanim_dtfrac>0)
	{
		btm_doanim_dtfrac-=dt;
		return(0);
	}

	acur=btm_doanim_skel->anim;
	while(acur)
	{
		if(!strcmp(acur->name, btm_doanim_name))
			break;
		acur=acur->next;
	}
	
	if(!acur)
	{
		BTM_DoAnimUpdate_ZeroBones();
		return(0);
	}

	btm_doanim_frame++;
	if(btm_doanim_frame>=acur->n_frames)
		btm_doanim_frame-=acur->n_frames;

	btm_doanim_dtfrac+=100;

	fr=btm_doanim_frame;
	mbn=acur->n_bones;

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
		
		BTM_BmdLoad_DecodePackQuat32(pv_r, btm_doanim_bonerot+bn*4);
		BTM_BmdLoad_DecodePackSe32(pv_b, btm_doanim_boneorg+bn*3);
	}

	return(0);
}

int BTM_DrawMesh(BTM_SolidMesh *mesh)
{
	float xyz0[3], st0[2], nv0[3], rgb0[3];
	float xyz1[3], st1[2], nv1[3], rgb1[3];
	float xyz2[3], st2[2], nv2[3], rgb2[3];
	float nv[3], tsv[4], ttv[4];
	u32 cl0, cl1, cl2;
	int i0, i1, i2;
	int j0, j1, j2;
	int i, j, k;

	k=-1;
	if(mesh->n_texvec>0)
	{
		k=BTM_GetTextureIdForName(mesh->usetex[0]);
	}

	if(k>0)
	{
		pglEnable(GL_TEXTURE_2D);
		pglBindTexture(TKRA_TEXTURE_2D, k);
	}else
	{
		pglDisable(GL_TEXTURE_2D);
	}
	
	BTM_RebuildMeshProjectTexture(mesh);
	
	tsv[0]=mesh->texvec_s[0];
	tsv[1]=mesh->texvec_s[1];
	tsv[2]=mesh->texvec_s[2];
	tsv[3]=mesh->texvec_s[3];
	
	ttv[0]=mesh->texvec_t[0];
	ttv[1]=mesh->texvec_t[1];
	ttv[2]=mesh->texvec_t[2];
	ttv[3]=mesh->texvec_t[3];
	
	pglBegin(GL_TRIANGLES);
//	for(i=0; i<mesh->nTris; i++)
	for(i=0; i<mesh->n_tris; i++)
	{
		i0=mesh->t_vidx[i*3+0];
		i1=mesh->t_vidx[i*3+1];
		i2=mesh->t_vidx[i*3+2];
		
		btm_vec3fv_copy(mesh->v_xyz+i0*3, xyz0);
		btm_vec3fv_copy(mesh->v_xyz+i1*3, xyz1);
		btm_vec3fv_copy(mesh->v_xyz+i2*3, xyz2);

		btm_vec2fv_copy(mesh->v_st+i0*2, st0);
		btm_vec2fv_copy(mesh->v_st+i1*2, st1);
		btm_vec2fv_copy(mesh->v_st+i2*2, st2);

		btm_vec3fv_copy(mesh->v_nv+i0*3, nv0);
		btm_vec3fv_copy(mesh->v_nv+i1*3, nv1);
		btm_vec3fv_copy(mesh->v_nv+i2*3, nv2);

		cl0=mesh->v_cl[i0];
		cl1=mesh->v_cl[i1];
		cl2=mesh->v_cl[i2];

		if(mesh->skel)
		{
			j0=mesh->v_bn[i0];
			j1=mesh->v_bn[i1];
			j2=mesh->v_bn[i2];
			
			HullF_TransformPoint(xyz0, xyz0, btm_bonetrans+j0*16);
			HullF_TransformPoint(xyz1, xyz1, btm_bonetrans+j1*16);
			HullF_TransformPoint(xyz2, xyz2, btm_bonetrans+j2*16);

//			btm_vec3fv_add(xyz0, mesh->skel->bone_baseorg+(j0*3), xyz0);
//			btm_vec3fv_add(xyz1, mesh->skel->bone_baseorg+(j1*3), xyz1);
//			btm_vec3fv_add(xyz2, mesh->skel->bone_baseorg+(j2*3), xyz2);
		}

		rgb0[0]=((cl0>>16)&255)*(1.0/255.0);
		rgb0[1]=((cl0>> 8)&255)*(1.0/255.0);
		rgb0[2]=((cl0>> 0)&255)*(1.0/255.0);

		rgb1[0]=((cl1>>16)&255)*(1.0/255.0);
		rgb1[1]=((cl1>> 8)&255)*(1.0/255.0);
		rgb1[2]=((cl1>> 0)&255)*(1.0/255.0);

		rgb2[0]=((cl2>>16)&255)*(1.0/255.0);
		rgb2[1]=((cl2>> 8)&255)*(1.0/255.0);
		rgb2[2]=((cl2>> 0)&255)*(1.0/255.0);

#if 0
		xyz0[0]=mesh->tris[(i*3+0)*3+0]*mesh->scale[0];
		xyz0[1]=mesh->tris[(i*3+0)*3+1]*mesh->scale[1];
		xyz0[2]=mesh->tris[(i*3+0)*3+2]*mesh->scale[2];

		xyz1[0]=mesh->tris[(i*3+1)*3+0]*mesh->scale[0];
		xyz1[1]=mesh->tris[(i*3+1)*3+1]*mesh->scale[1];
		xyz1[2]=mesh->tris[(i*3+1)*3+2]*mesh->scale[2];

		xyz2[0]=mesh->tris[(i*3+2)*3+0]*mesh->scale[0];
		xyz2[1]=mesh->tris[(i*3+2)*3+1]*mesh->scale[1];
		xyz2[2]=mesh->tris[(i*3+2)*3+2]*mesh->scale[2];

		nv[0]=mesh->norm[i*4+0];
		nv[1]=mesh->norm[i*4+1];
		nv[2]=mesh->norm[i*4+2];
		nv[3]=mesh->norm[i*4+3];

		st0[0]=(xyz0[0]*tsv[0])+(xyz0[1]*tsv[1])+(xyz0[2]*tsv[2])-tsv[3];
		st0[1]=(xyz0[0]*ttv[0])+(xyz0[1]*ttv[1])+(xyz0[2]*ttv[2])-ttv[3];
		st1[0]=(xyz1[0]*tsv[0])+(xyz1[1]*tsv[1])+(xyz1[2]*tsv[2])-tsv[3];
		st1[1]=(xyz1[0]*ttv[0])+(xyz1[1]*ttv[1])+(xyz1[2]*ttv[2])-ttv[3];
		st2[0]=(xyz2[0]*tsv[0])+(xyz2[1]*tsv[1])+(xyz2[2]*tsv[2])-tsv[3];
		st2[1]=(xyz2[0]*ttv[0])+(xyz2[1]*ttv[1])+(xyz2[2]*ttv[2])-ttv[3];
#endif

		glColor4f(
			rgb0[0]*(0.8+nv0[0]*0.1)*(1.0+nv0[2]*0.2),
			rgb0[1]*(0.8+nv0[1]*0.1)*(1.0+nv0[2]*0.2),
			rgb0[2]*(0.8+nv0[2]*0.1)*(1.0+nv0[2]*0.2),
			1.0);
		glTexCoord2fv(st0);
		glVertex3fv(xyz0);

		glColor4f(
			rgb1[0]*(0.8+nv1[0]*0.1)*(1.0+nv1[2]*0.2),
			rgb1[1]*(0.8+nv1[1]*0.1)*(1.0+nv1[2]*0.2),
			rgb1[2]*(0.8+nv1[2]*0.1)*(1.0+nv1[2]*0.2),
			1.0);
		glTexCoord2fv(st1);
		glVertex3fv(xyz1);

		glColor4f(
			rgb2[0]*(0.8+nv2[0]*0.1)*(1.0+nv2[2]*0.2),
			rgb2[1]*(0.8+nv2[1]*0.1)*(1.0+nv2[2]*0.2),
			rgb2[2]*(0.8+nv2[2]*0.1)*(1.0+nv2[2]*0.2),
			1.0);
		glTexCoord2fv(st2);
		glVertex3fv(xyz2);
	}
	pglEnd();

	pglEnable(GL_TEXTURE_2D);
	return(0);
}

int BTM_DrawMeshList(BTM_SolidMesh *lst)
{
	float tmat[16], trot[4];
	BTM_SolidMesh *cur;
	BTM_SolidSkel *skel;
	int i, j, k;

	BTM_BmdDrawInit();

	if(!lst)
		return(0);

	skel=lst->skel;
	if(skel)
	{
		btm_doanim_skel=skel;
	
		TKRA_Mat4F_Identity(btm_bonetrans+0*16);
		for(i=1; i<skel->n_bones; i++)
		{
			j=skel->bone_parent[i];
			QuatF_Multiply(
				btm_doanim_bonerot+i*4,
				skel->bone_baserot+i*4,
				trot);
			QuatF_ToMatrix(
//				skel->bone_baserot+i*4, tmat);
				trot, tmat);
			tmat[12]+=skel->bone_relorg[i*3+0];
			tmat[13]+=skel->bone_relorg[i*3+1];
			tmat[14]+=skel->bone_relorg[i*3+2];
			tmat[12]+=btm_doanim_boneorg[i*3+0];
			tmat[13]+=btm_doanim_boneorg[i*3+1];
			tmat[14]+=btm_doanim_boneorg[i*3+2];
			
			TKRA_Mat4F_MatMult(
//				btm_bonetrans+j*16,	tmat,
				tmat, btm_bonetrans+j*16,
				btm_bonetrans+i*16);
		}
	}

	cur=lst;
	while(cur)
	{
		BTM_DrawMesh(cur);
		cur=cur->next;
	}
	return(0);
}

int BTM_ConCmd_Load(BTM_ConCmd *cmd, char **args)
{
	btm_modelname=bccx_strdup(args[1]);
	btm_meshbase=BTM_LoadMeshListStl(btm_modelname);
	return(0);
}

int BTM_ConCmd_Export(BTM_ConCmd *cmd, char **args)
{
	BTM_ExportMeshListBmd(args[1], btm_meshbase);
	return(0);
}

int BTM_ConCmd_DoAnim(BTM_ConCmd *cmd, char **args)
{
	if(args[1])
	{
		btm_doanim_name=bccx_strdup(args[1]);
	}else
	{
		btm_doanim_name=NULL;
	}
}

int main(int argc, char *argv[])
{
	char tb[256];
	float tv0[3], tv1[3];
	TKRA_Context *ractx;
	BTM_World *wrl;
	BTM_MobEntity *mob;
//	BTM_Screen *scr;
//	BTM_TexImg *tex;
//	BTM_TexImg *skytex;
	
	int tt, ltt, dt, wrldt;
	int t0, t1, t2;
//	u32 *screen, *ct;
//	u16 *fbuf, *cs, *act;
	u16 *tex0;
	u16 *fbuf;
	byte *tbuf;
	u64 wpos;
	u32 tblk;
	int x, y, z, xs, ys, txs, tys;
	int mx, my, mb, lmb;
	int afrac, astep, mdl_ntris;
	int i, j, k, l;
	float f0, f1, f2, f3;
	float ang, dtf, frc;


	FS_Init();

	BTM_ConAddCommand("runpgm", BTM_ConCmd_RunPgm);
	BTM_ConAddCommand("eval", BTM_ConCmd_Eval);
	BTM_ConAddCvar("mlook", &btm_mlook, 0x3F);

	BTM_ConAddCommand("load", BTM_ConCmd_Load);
	BTM_ConAddCommand("export", BTM_ConCmd_Export);
	BTM_ConAddCommand("doanim", BTM_ConCmd_DoAnim);

	I_SystemInit();
	BTM_PGL_InitOpenGlFuncs();

	xs=320;
	ys=200;

	printf("Init TKRA\n");

	ractx=TKRA_AllocContext();
	TKRA_SetupScreen(ractx, 320, 200);
	TKRA_SetupForState(ractx);
	TKRA_SetCurrentContext(ractx);

	fbuf=ractx->screen_rgb;

	pglViewport(0, 0, 320, 200);

//	btmgl_filter_min=GL_NEAREST_MIPMAP_NEAREST;
	btmgl_filter_min=GL_NEAREST_MIPMAP_LINEAR;
//	btmgl_filter_min=GL_LINEAR_MIPMAP_LINEAR;
//	btmgl_filter_min=GL_LINEAR_MIPMAP_NEAREST;
	btmgl_filter_max=GL_NEAREST;
//	btmgl_filter_max=GL_LINEAR;

	pglDepthRange(0.0, 0.75);
	pglClearDepth(0.999);

	pglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	pglBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	pglAlphaFunc(GL_GEQUAL, 0.5);
	
	pglClearColor(0.3, 0.6, 0.9, 1.0);

	mx=0; my=0; mb=0;

	btm_wrl=BTM_AllocWorld(7, 7);


	cam_org[0]=8;
	cam_org[1]=8;
	cam_org[2]=4;

	cam_ang_yaw=-45;
	cam_ang_pitch=-23;

	tt=I_TimeMS();
	ltt=tt;
	while(!gfxdrv_kill)
	{
		tt=I_TimeMS();
		dt=tt-ltt;

//		if(!dt)
		if(dt<8)
			{ continue; }

		ltt=tt;

		btmgl_time_ms=tt;
		
		if(dt>500)
			dt=500;

		tt_drawframe++;

		I_BeginFrame(fbuf, xs, ys);

		I_UpdateKeys();

		BTM_MenuDoInput(dt);
		BTM_ConDoInput(dt);
		BTM_InvenDoInput(dt);
		BTM_DoMixSamples(dt);

		BTM_DoAnimUpdate(dt);

		lmb=mb;
		GfxDrv_MouseGetPos(&mx, &my, &mb);

		if(!BTM_MenuDownP() && !BTM_ConDownP() && !BTM_InvenOpenP())
		{
//			if(mb&2)
			if(btm_mlook)
			{
//				if(lmb&2)
				if(btm_lmlook)
				{
					cam_ang_yaw+=mx*0.5;
					cam_ang_pitch-=my*0.5;
				}
				GfxDrv_MouseSetPos(0, 0);
			}

			if((mb&2) && !(lmb&2))
			{
				btm_mlook=!btm_mlook;
			}
			
			btm_lmlook=btm_mlook;
		
			if(!I_KeyDown(K_SHIFT) && !btm_mlook)
			{
				if(I_KeyDown(K_LEFTARROW))
					cam_ang_yaw-=dt*(90/1000.0);
				if(I_KeyDown(K_RIGHTARROW))
					cam_ang_yaw+=dt*(90/1000.0);
			}
			
			if(I_KeyDown(K_PGDN))
				cam_ang_pitch-=dt*(90/1000.0);
			if(I_KeyDown(K_PGUP))
				cam_ang_pitch+=dt*(90/1000.0);

//			if(btm_noclip)
			if(1)
			{
				if(I_KeyDown(K_HOME))
					cam_org[2]+=dt*(12/1000.0);
				if(I_KeyDown(K_END))
					cam_org[2]-=dt*(12/1000.0);

				if(I_KeyDown(K_UPARROW) ||
						I_KeyDown('w') ||
						I_KeyDown('W'))
				{
					cam_org[0]+=dt*(12/1000.0)*sin(cam_ang_yaw*(M_PI/180));
					cam_org[1]-=dt*(12/1000.0)*cos(cam_ang_yaw*(M_PI/180));
				}
				if(I_KeyDown(K_DOWNARROW) ||
						I_KeyDown('s') ||
						I_KeyDown('S'))
				{
					cam_org[0]-=dt*(12/1000.0)*sin(cam_ang_yaw*(M_PI/180));
					cam_org[1]+=dt*(12/1000.0)*cos(cam_ang_yaw*(M_PI/180));
				}
				
				if(I_KeyDown(K_SHIFT) || btm_lmlook)
				{
					if(I_KeyDown(K_LEFTARROW) ||
						I_KeyDown('a') ||
						I_KeyDown('A'))
					{
						cam_org[0]-=dt*(12/1000.0)*
							cos(cam_ang_yaw*(M_PI/180));
						cam_org[1]-=dt*(12/1000.0)*
							sin(cam_ang_yaw*(M_PI/180));
					}
					if(I_KeyDown(K_RIGHTARROW) ||
						I_KeyDown('d') ||
						I_KeyDown('D'))
					{
						cam_org[0]+=dt*(12/1000.0)*
							cos(cam_ang_yaw*(M_PI/180));
						cam_org[1]+=dt*(12/1000.0)*
							sin(cam_ang_yaw*(M_PI/180));
					}
				}
			}

			if(	(I_KeyDown('r') && !I_KeyDownL('r')) ||
				(I_KeyDown('R') && !I_KeyDownL('R')) )
			{
				if(!btm_modelname)
				{
					BTM_ConPrintf("Can't Reload, No Model Loaded\n");
				}else
				{
					BTM_PgmFlushPrograms();
					btm_meshbase=BTM_LoadMeshListStl(btm_modelname);
				}
			}
		}


//		if(cam_org[0]<0)
//			{ cam_org[0]+=1<<16; }
//		if(cam_org[1]<0)
//			{ cam_org[1]+=1<<16; }
//		if(cam_org[0]>=(1<<16))
//			{ cam_org[0]-=1<<16; }
//		if(cam_org[1]>=(1<<16))
//			{ cam_org[1]-=1<<16; }

		if(cam_org[2]<-128)
			{ cam_org[2]=-128; }
		if(cam_org[2]>128)
			{ cam_org[2]=128; }
			
		if(cam_ang_pitch>85)
			cam_ang_pitch=85;
		if(cam_ang_pitch<-85)
			cam_ang_pitch=-85;
		
		btm_cam_yaw=(u16)(cam_ang_yaw*(65536.0/360.0));
		btm_cam_pitch=(s16)(cam_ang_pitch*(65536.0/360.0));

		cam_ang_yaw=btm_cam_yaw*(360.0/65536.0);
		cam_ang_pitch=btm_cam_pitch*(360.0/65536.0);

		t0=I_TimeMS();

// 		pglClear(TKRA_GL_DEPTH_BUFFER_BIT);
		pglClear(TKRA_GL_DEPTH_BUFFER_BIT | TKRA_GL_COLOR_BUFFER_BIT);

		pglMatrixMode(TKRA_MODELVIEW);
		pglLoadIdentity();

		pglMatrixMode(TKRA_PROJECTION);
		pglLoadIdentity();
		pglFrustum(-0.1, 0.1,  0.075, -0.075, 0.1, 1024);

#if 1
		pglRotatef(90, 1, 0, 0);
		pglRotatef(cam_ang_pitch, 1, 0, 0);

		pglRotatef(-cam_ang_yaw, 0, 0, 1);
		pglTranslatef(-cam_org[0], -cam_org[1], -cam_org[2]);
#endif

		pglDisable(GL_CULL_FACE);
		pglDepthFunc(GL_LEQUAL);
		pglEnable(GL_DEPTH_TEST);
//		pglEnable(GL_BLEND);
		pglDisable(GL_BLEND);

//		pglDisable(GL_BLEND);

		// render stuff...

		pglDisable(GL_TEXTURE_2D);
		pglBegin(GL_LINES);
		glColor4f(1,0,0,1);
		glVertex3f(0,0,0);
		glVertex3f(100,0,0);
		glColor4f(0,1,0,1);
		glVertex3f(0,0,0);
		glVertex3f(0,100,0);
		glColor4f(0,0,1,1);
		glVertex3f(0,0,0);
		glVertex3f(0,0,100);
		pglEnd();
		
		BTM_DrawMeshList(btm_meshbase);


		pglEnable(GL_TEXTURE_2D);


		pglMatrixMode(TKRA_MODELVIEW);
		pglLoadIdentity();

		pglMatrixMode(TKRA_PROJECTION);
		pglLoadIdentity();
//		pglOrtho(-160, 160, 100, -100, -999.0, 999.0);
		pglOrtho(-160, 160, -100, 100, -999.0, 999.0);

		pglDisable(GL_CULL_FACE);
		pglDisable(GL_DEPTH_TEST);
		pglDisable(GL_BLEND);
		pglEnable(GL_ALPHA_TEST);

#if 0
//		j=wrl->sel_bt;
//		k=btmgl_vox_atlas_side[j&255];
		x=k&15;
		y=(k>>4)&15;
//		y=15-((j>>4)&15);

		f0=(x+0)*(1.0/16)+(1.0/128);
		f1=(x+1)*(1.0/16)-(1.0/128);
		f2=(y+0)*(1.0/16)+(1.0/128);
		f3=(y+1)*(1.0/16)-(1.0/128);

		pglColor4f(1.0, 1.0, 1.0, 1.0);
		pglBindTexture(GL_TEXTURE_2D, 2);
		pglBegin(GL_QUADS);
		pglTexCoord2f(f0, f2);
		pglVertex2f(64, -66);
		pglTexCoord2f(f0, f3);
		pglVertex2f(64, -98);
		pglTexCoord2f(f1, f3);
		pglVertex2f(96, -98);
		pglTexCoord2f(f1, f2);
		pglVertex2f(96, -66);

		pglEnd();
		
		sprintf(tb, "%02X\n%s", wrl->sel_bt,
			BTM_BlockMiniDesc(wrl, wrl->sel_bt));
		BTM_DrawString8px(96, -74, tb, 0xFFFFFFFFU);
#endif

//		BTM_DrawInventory(wrl);
		BTM_DrawMenu();
		BTM_DrawConsole();

		sprintf(tb, "%03d", 1000/dt);
		BTM_DrawString8px(160-24, 100-8, tb, 0xFFFFFFFFU);

		sprintf(tb, "%04d", dt);
		BTM_DrawString8px(160-32, 100-16, tb, 0xFFFFFFFFU);

		sprintf(tb, "%05d", ((int)floor(cam_org[0]))&0xFFFF);
		BTM_DrawString8px(160-40, 100-24, tb, 0xFFFFFFFFU);
		sprintf(tb, "%05d", ((int)floor(cam_org[1]))&0xFFFF);
		BTM_DrawString8px(160-40, 100-32, tb, 0xFFFFFFFFU);
		sprintf(tb, "%03d", ((int)floor(cam_org[2]))&0xFF);
		BTM_DrawString8px(160-24, 100-40, tb, 0xFFFFFFFFU);

//		sprintf(tb, "%03d", ((int)floor(cam_ang_yaw))&0xFF);
		sprintf(tb, "%03d", (btm_cam_yaw>>8)&255);
		BTM_DrawString8px(160-24, 100-48, tb, 0xFFFFFFFFU);
//		sprintf(tb, "%03d", ((int)floor(cam_ang_pitch))&0xFF);
		sprintf(tb, "%03d", (btm_cam_pitch>>8)&255);
		BTM_DrawString8px(160-24, 100-56, tb, 0xFFFFFFFFU);

		BTM_DrawString8px(160-24, 100-40, NULL, 0xFFFFFFFFU);

		I_SystemFrame(fbuf, xs, ys);
	}
}
