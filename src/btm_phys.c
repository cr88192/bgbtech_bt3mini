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

int BTM_CheckWorldMoveBlockContents(BTM_World *wrl, u32 btm)
{
	u32 blkd;

	blkd=btmgl_vox_atlas_side[btm&0xFF];

	if(blkd&BTM_BLKDFL_NONSOLID)
		{ return(0); }
	if(blkd&BTM_BLKDFL_FLUID)
		{ return(2); }

	if(blkd&BTM_BLKDFL_TY_MASK)
		{ return(16); }

	return(1);
}

int BTM_GetBBoxListForBlockData(BTM_World *wrl,
	u32 blk, int cx, int cy, int cz,
	float *bboxlst, int *rnbbox)
{
	u32 blkd;

	blkd=btmgl_vox_atlas_side[blk&0xFF];

	if((blkd&BTM_BLKDFL_TY_MASK)==BTM_BLKDFL_TY_FULL)
	{
		bboxlst[0]=cx+0;	bboxlst[1]=cy+0;	bboxlst[2]=cz+0;
		bboxlst[3]=cx+1;	bboxlst[4]=cy+1;	bboxlst[5]=cz+1;
		*rnbbox=1;
		return(0);
	}

	if((blkd&BTM_BLKDFL_TY_MASK)==BTM_BLKDFL_TY_SLAB)
	{
		bboxlst[0]=cx+0;	bboxlst[1]=cy+0;	bboxlst[2]=cz+0;
		bboxlst[3]=cx+1;	bboxlst[4]=cy+1;	bboxlst[5]=cz+0.5;
		*rnbbox=1;
		return(0);
	}

	if((blkd&BTM_BLKDFL_TY_MASK)==BTM_BLKDFL_TY_STAIR)
	{
		bboxlst[0]=cx+0;	bboxlst[1]=cy+0;	bboxlst[2]=cz+0;
		bboxlst[3]=cx+1;	bboxlst[4]=cy+1;	bboxlst[5]=cz+0.3;
		
		switch((blk>>8)&3)
		{
		case 0:
			bboxlst[ 6]=cx+0.00; bboxlst[ 7]=cy+0.00; bboxlst[ 8]=cz+0.00;
			bboxlst[ 9]=cx+1.00; bboxlst[10]=cy+0.33; bboxlst[11]=cz+1.00;
			bboxlst[12]=cx+0.00; bboxlst[13]=cy+0.33; bboxlst[14]=cz+0.00;
			bboxlst[15]=cx+1.00; bboxlst[16]=cy+0.66; bboxlst[17]=cz+0.66;
			break;
		case 2:
			bboxlst[ 6]=cx+0.00; bboxlst[ 7]=cy+0.66; bboxlst[ 8]=cz+0.00;
			bboxlst[ 9]=cx+1.00; bboxlst[10]=cy+1.00; bboxlst[11]=cz+1.00;
			bboxlst[12]=cx+0.00; bboxlst[13]=cy+0.33; bboxlst[14]=cz+0.00;
			bboxlst[15]=cx+1.00; bboxlst[16]=cy+0.66; bboxlst[17]=cz+0.66;
			break;

		case 1:
			bboxlst[ 6]=cx+0.66; bboxlst[ 7]=cy+0.00; bboxlst[ 8]=cz+0.00;
			bboxlst[ 9]=cx+1.00; bboxlst[10]=cy+1.00; bboxlst[11]=cz+1.00;
			bboxlst[12]=cx+0.33; bboxlst[13]=cy+0.00; bboxlst[14]=cz+0.00;
			bboxlst[15]=cx+0.66; bboxlst[16]=cy+1.00; bboxlst[17]=cz+0.66;
			break;
		case 3:
			bboxlst[ 6]=cx+0.00; bboxlst[ 7]=cy+0.00; bboxlst[ 8]=cz+0.00;
			bboxlst[ 9]=cx+0.33; bboxlst[10]=cy+1.00; bboxlst[11]=cz+1.00;
			bboxlst[12]=cx+0.33; bboxlst[13]=cy+0.00; bboxlst[14]=cz+0.00;
			bboxlst[15]=cx+0.66; bboxlst[16]=cy+1.00; bboxlst[17]=cz+0.66;
			break;
		}
		
		*rnbbox=3;
		return(0);
	}

	*rnbbox=0;
	return(0);
}

int BTM_AabbCheckCollide(float *box1, float *box2)
{
	if(box1[3]<box2[0])
		return(0);
	if(box2[3]<box1[0])
		return(0);

	if(box1[4]<box2[1])
		return(0);
	if(box2[4]<box1[1])
		return(0);

	if(box1[5]<box2[2])
		return(0);
	if(box2[5]<box1[2])
		return(0);
	
	return(1);
}

int BTM_CheckWorldMoveBBoxBlockContents(BTM_World *wrl,
	const float *org, const float *bbox,
	int cx, int cy, int cz)
{
	float bboxen[6*6];
	float aabb[6];
	u32 blk;
	u32 blkd;
	int nbbox;
	int i, j, k;

	blk=BTM_GetWorldBlockXYZ(wrl, cx, cy, cz);
	blkd=btmgl_vox_atlas_side[blk&0xFF];

	if(!(blkd&BTM_BLKDFL_TY_MASK))
	{
		/* Solid Block */
		return(BTM_CheckWorldMoveBlockContents(wrl, blk));
	}
	
	aabb[0]=org[0]+bbox[0];
	aabb[1]=org[1]+bbox[1];
	aabb[2]=org[2]+bbox[2];
	aabb[3]=org[0]+bbox[3];
	aabb[4]=org[1]+bbox[4];
	aabb[5]=org[2]+bbox[5];
	
	BTM_GetBBoxListForBlockData(wrl, blk, cx, cy, cz, bboxen, &nbbox);
	
	for(i=0; i<nbbox; i++)
	{
		if(BTM_AabbCheckCollide(aabb, bboxen+i*6))
			break;
	}
	
	if(i<nbbox)
	{
		return(1);
	}
	
	return(0);
}

int BTM_CheckWorldMovePoint(BTM_World *wrl, float *org)
{
	u32 blk;
	int cont;
	int cx, cy, cz;

	cx=floor(org[0]);
	cy=floor(org[1]);
	cz=floor(org[2]);
	blk=BTM_GetWorldBlockXYZ(wrl, cx, cy, cz);
	cont=BTM_CheckWorldMoveBlockContents(wrl, blk);
	return(cont);
}

int BTM_CheckWorldMoveSpot(BTM_World *wrl,
	const float *org, const float *bbox, const float *vel)
{
	BTM_MobEntity *mlst, *mcur;
	int cont;
	int cxm, cxn, cym, cyn, czm, czn, cza;
	u64 cpos;
	u32 blk0, blk1, blk2, blk3;
	u32 blk4, blk5, blk6, blk7;
	int noenpts, ax;

//	cxm=org[0]-0.35;
//	cxn=org[0]+0.35;
//	cym=org[1]-0.35;
//	cyn=org[1]+0.35;

//	czm=org[2]-1.7;
//	czn=org[2]+0.1;

	cxm=floor(org[0]+bbox[0]);
	cxn=floor(org[0]+bbox[3]);
	cym=floor(org[1]+bbox[1]);
	cyn=floor(org[1]+bbox[4]);

	czm=floor(org[2]+bbox[2]);
	czn=floor(org[2]+bbox[5]);
	cza=floor(org[2]+(bbox[2]+bbox[5])*0.5);

	cont=0;

//	if(!wrl->move_selfent)
	if(1)
	{
//		mlst=BTM_QueryWorldEntitiesForBox(wrl, NULL,
//			cxm-1, cym-1, czm-1, cxn+1, cyn+1, czn+1,
//			wrl->move_selfent);
		mlst=wrl->move_mqlist;
		mcur=mlst;
		while(mcur)
		{
			if(BTM_CheckBoxCollideMob(wrl, mcur, org, bbox))
				{ cont|=1; }
			mcur=mcur->chn_bpos;
		}
	}

	if(fabs(vel[0])>fabs(vel[2]))
	{
		if(fabs(vel[0])>fabs(vel[1]))
			{ ax=0; }
		else
			{ ax=1; }
	}else
	{
		if(fabs(vel[1])>fabs(vel[2]))
			{ ax=1; }
		else
			{ ax=2; }
	}

	if((cxm==cxn) && (cym==cyn))
	{
//		cont=0;
		blk0=BTM_GetWorldBlockXYZ(wrl, cxm, cym, czm);
		blk1=BTM_GetWorldBlockXYZ(wrl, cxm, cym, czn);

		cont|=BTM_CheckWorldMoveBlockContents(wrl, blk0);
		cont|=BTM_CheckWorldMoveBlockContents(wrl, blk1);

		if((cza!=czm) && (cza!=czn))
		{
			blk2=BTM_GetWorldBlockXYZ(wrl, cxm, cym, cza);
			cont|=BTM_CheckWorldMoveBlockContents(wrl, blk2);
		}
	}else
	{

		noenpts=0;
		
#if 1
		if(vel[0]>1.5)
			{ noenpts|=0x555; }
		if(vel[0]<(-1.5))
			{ noenpts|=0xAAA; }
		if(vel[1]>1.5)
			{ noenpts|=0x333; }
		if(vel[1]<(-1.5))
			{ noenpts|=0xCCC; }
		if(vel[2]>1.5)
			{ noenpts|=0x00F; }
		if(vel[2]<(-1.5))
			{ noenpts|=0x0F0; }

		if(vel[0]>0.5)
			{ noenpts&=~0xAAA; }
		if(vel[0]<(-0.5))
			{ noenpts&=~0x555; }
		if(vel[1]>0.5)
			{ noenpts&=~0xCCC; }
		if(vel[1]<(-0.5))
			{ noenpts&=~0x333; }
		if(vel[2]>0.01)
			{ noenpts&=~0x0F0; }
		if(vel[2]<(-0.01))
			{ noenpts&=~0x00F; }
#endif
		
		if(!(noenpts&0x55))
		{
			if(cxm==cxn)
				{ noenpts|=0xAAA; }
		}

		if(!(noenpts&0x33))
		{
			if(cym==cyn)
				{ noenpts|=0xCCC; }
		}

		if(!(noenpts&0x0F))
		{
			if(czm==czn)
				{ noenpts|=0xFF0; }
		}

		noenpts=0;

	//	blk0=BTM_GetWorldBlockXYZ(wrl, org[0], org[1], org[2]-2);
	//	blk1=BTM_GetWorldBlockXYZ(wrl, org[0], org[1], org[2]-1);

//		cont=0;

		if(!(noenpts&0x01))
		{
			blk0=BTM_GetWorldBlockXYZ(wrl, cxm, cym, czm);
			cont|=BTM_CheckWorldMoveBlockContents(wrl, blk0);
		}
		if(!(noenpts&0x02))
		{
			blk1=BTM_GetWorldBlockXYZ(wrl, cxn, cym, czm);
			cont|=BTM_CheckWorldMoveBlockContents(wrl, blk1);
		}
		if(!(noenpts&0x04))
		{
			blk2=BTM_GetWorldBlockXYZ(wrl, cxm, cyn, czm);
			cont|=BTM_CheckWorldMoveBlockContents(wrl, blk2);
		}
		if(!(noenpts&0x08))
		{
			blk3=BTM_GetWorldBlockXYZ(wrl, cxn, cyn, czm);
			cont|=BTM_CheckWorldMoveBlockContents(wrl, blk3);
		}

		if(!(noenpts&0x10))
		{
			blk4=BTM_GetWorldBlockXYZ(wrl, cxm, cym, czn);
			cont|=BTM_CheckWorldMoveBlockContents(wrl, blk4);
		}
		if(!(noenpts&0x20))
		{
			blk5=BTM_GetWorldBlockXYZ(wrl, cxn, cym, czn);
			cont|=BTM_CheckWorldMoveBlockContents(wrl, blk5);
		}
		if(!(noenpts&0x40))
		{
			blk6=BTM_GetWorldBlockXYZ(wrl, cxm, cyn, czn);
			cont|=BTM_CheckWorldMoveBlockContents(wrl, blk6);
		}
		if(!(noenpts&0x80))
		{
			blk7=BTM_GetWorldBlockXYZ(wrl, cxn, cyn, czn);
			cont|=BTM_CheckWorldMoveBlockContents(wrl, blk7);
		}

#if 1
		if((cza!=czm) && (cza!=czn))
		{
			if(!(noenpts&0x100))
			{
				blk4=BTM_GetWorldBlockXYZ(wrl, cxm, cym, cza);
				cont|=BTM_CheckWorldMoveBlockContents(wrl, blk4);
			}
			if(!(noenpts&0x200))
			{
				blk5=BTM_GetWorldBlockXYZ(wrl, cxn, cym, cza);
				cont|=BTM_CheckWorldMoveBlockContents(wrl, blk5);
			}
			if(!(noenpts&0x400))
			{
				blk6=BTM_GetWorldBlockXYZ(wrl, cxm, cyn, cza);
				cont|=BTM_CheckWorldMoveBlockContents(wrl, blk6);
			}
			if(!(noenpts&0x800))
			{
				blk7=BTM_GetWorldBlockXYZ(wrl, cxn, cyn, cza);
				cont|=BTM_CheckWorldMoveBlockContents(wrl, blk7);
			}
		}
#endif
	}

	if(cont&1)
		return(cont);

	if(cont&16)
	{
//		cont=0;
		cont&=~16;
		
		cont|=BTM_CheckWorldMoveBBoxBlockContents(
			wrl, org, bbox, cxm, cym, czm);
		cont|=BTM_CheckWorldMoveBBoxBlockContents(
			wrl, org, bbox, cxn, cym, czm);
		cont|=BTM_CheckWorldMoveBBoxBlockContents(
			wrl, org, bbox, cxm, cyn, czm);
		cont|=BTM_CheckWorldMoveBBoxBlockContents(
			wrl, org, bbox, cxn, cyn, czm);
		
		cont|=BTM_CheckWorldMoveBBoxBlockContents(
			wrl, org, bbox, cxm, cym, czn);
		cont|=BTM_CheckWorldMoveBBoxBlockContents(
			wrl, org, bbox, cxn, cym, czn);
		cont|=BTM_CheckWorldMoveBBoxBlockContents(
			wrl, org, bbox, cxm, cyn, czn);
		cont|=BTM_CheckWorldMoveBBoxBlockContents(
			wrl, org, bbox, cxn, cyn, czn);

		if((cza!=czm) && (cza!=czn))
		{
			cont|=BTM_CheckWorldMoveBBoxBlockContents(
				wrl, org, bbox, cxm, cym, cza);
			cont|=BTM_CheckWorldMoveBBoxBlockContents(
				wrl, org, bbox, cxn, cym, cza);
			cont|=BTM_CheckWorldMoveBBoxBlockContents(
				wrl, org, bbox, cxm, cyn, cza);
			cont|=BTM_CheckWorldMoveBBoxBlockContents(
				wrl, org, bbox, cxn, cyn, cza);
		}
	}

//	if(((blk0&255)<2) && ((blk1&255)<2))
//		return(1);

	return(cont);
}

int BTM_CheckWorldBoxMoveVel(BTM_World *wrl, float dt,
	float *sorg, float *svel, const float *bbox,
	float *dorg, float *dvel, int *rfl)
{
	float tdo[4], tdv[4];
	float f, g;
	int cxm, cym, czm, cxn, cyn, czn;
	int fl, cfl, cfl1;
	int i, j, k;

	f=fabs(svel[0])+fabs(svel[1])+fabs(svel[2]);

	
	TKRA_Vec3F_Zero(tdv);
	cfl=BTM_CheckWorldMoveSpot(wrl, sorg, bbox, tdv);
	if(cfl&1)
	{
		return(3);
	}


//	if(dt>0.1)
	if((dt>0.1) || ((f*dt)>=0.25))
	{
		BTM_CheckWorldBoxMoveVel(wrl, dt*0.5, sorg, svel, bbox, tdo, tdv, rfl);
		BTM_CheckWorldBoxMoveVel(wrl, dt*0.5, tdo, tdv, bbox, dorg, dvel, rfl);
		return(0);
	}

	TKRA_Vec3F_Copy(sorg, tdo);
	TKRA_Vec3F_Copy(svel, tdv);

	TKRA_Vec3F_AddScale(sorg, tdv, dt, tdo);
	fl=*rfl;

	if(svel[2]>0)
		fl&=~1;

	cxm=floor(sorg[0]+bbox[0]);
	cxn=floor(sorg[0]+bbox[3]);
	cym=floor(sorg[1]+bbox[1]);
	cyn=floor(sorg[1]+bbox[4]);

	czm=floor(sorg[2]+bbox[2]);
	czn=floor(sorg[2]+bbox[5]);
//	cza=floor(sorg[2]+(bbox[2]+bbox[5])*0.5);

	wrl->move_mqlist=
		BTM_QueryWorldEntitiesForBox(wrl, NULL,
			cxm-1, cym-1, czm-1, cxn+1, cyn+1, czn+1,
			wrl->move_selfent);

	cfl=BTM_CheckWorldMoveSpot(wrl, tdo, bbox, svel);

	if(!(cfl&1))
	{
		cfl1=BTM_CheckWorldMovePoint(wrl, tdo);
	
		if(svel[2]<0)
			fl&=~1;

		if(cfl&2)		
			fl|=2;
		else
			fl&=~2;

		if(cfl1&2)
			fl|=4;
		else
			fl&=~4;

		TKRA_Vec3F_Copy(tdo, dorg);
		TKRA_Vec3F_Copy(tdv, dvel);
		*rfl=fl;
		return(0);
	}
	
//	if(tdv[2]<0)
	if(1)
	{
		TKRA_Vec3F_Copy(svel, tdv);
		tdv[2]=0;

		TKRA_Vec3F_AddScale(sorg, tdv, dt, tdo);
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox, tdv)&1))
		{
			if(svel[2]<=0)
				fl|=1;
		
			TKRA_Vec3F_Copy(tdo, dorg);
			TKRA_Vec3F_Copy(tdv, dvel);
			*rfl=fl;
			return(0);
		}
	}

	if(1)
	{
		TKRA_Vec3F_Copy(svel, tdv);
//		tdv[2]=0;

		TKRA_Vec3F_AddScale(sorg, tdv, dt, tdo);
//		tdo[2]+=0.5;
		tdo[2]+=0.65;
		
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox, tdv)&1))
		{
//			if(svel[2]<0)
//				fl|=1;
		
			TKRA_Vec3F_Copy(tdo, dorg);
			TKRA_Vec3F_Copy(tdv, dvel);
			*rfl=fl;
			return(0);
		}
	}

	if(1)
	{
		TKRA_Vec3F_Copy(svel, tdv);
		tdv[0]=0;

		TKRA_Vec3F_AddScale(sorg, tdv, dt, tdo);
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox, tdv)&1))
		{
			TKRA_Vec3F_Copy(tdo, dorg);
			TKRA_Vec3F_Copy(tdv, dvel);
			*rfl=fl;
			return(0);
		}
	}

	if(1)
	{
		TKRA_Vec3F_Copy(svel, tdv);
		tdv[1]=0;

		TKRA_Vec3F_AddScale(sorg, tdv, dt, tdo);
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox, tdv)&1))
		{
			TKRA_Vec3F_Copy(tdo, dorg);
			TKRA_Vec3F_Copy(tdv, dvel);
			*rfl=fl;
			return(0);
		}
	}

	if(1)
	{
		TKRA_Vec3F_Copy(svel, tdv);
		tdv[2]=0;
		tdv[0]=0;

		TKRA_Vec3F_AddScale(sorg, tdv, dt, tdo);
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox, tdv)&1))
		{
			if(svel[2]<=0)
				fl|=1;

			TKRA_Vec3F_Copy(tdo, dorg);
			TKRA_Vec3F_Copy(tdv, dvel);
			*rfl=fl;
			return(0);
		}
	}

	if(1)
	{
		TKRA_Vec3F_Copy(svel, tdv);
		tdv[2]=0;
		tdv[1]=0;

		TKRA_Vec3F_AddScale(sorg, tdv, dt, tdo);
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox, tdv)&1))
		{
			if(svel[2]<=0)
				fl|=1;

			TKRA_Vec3F_Copy(tdo, dorg);
			TKRA_Vec3F_Copy(tdv, dvel);
			*rfl=fl;
			return(0);
		}
	}

	if(1)
	{
		TKRA_Vec3F_Copy(svel, tdv);
		tdv[0]=0;
		tdv[1]=0;

		TKRA_Vec3F_AddScale(sorg, tdv, dt, tdo);
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox, tdv)&1))
		{
			TKRA_Vec3F_Copy(tdo, dorg);
			TKRA_Vec3F_Copy(tdv, dvel);
			*rfl=fl;
			return(0);
		}
	}

	tdv[0]=0;
	tdv[1]=0;
	tdv[2]=0;

	if(BTM_CheckWorldMoveSpot(wrl, sorg, bbox, tdv)&1)
	{
//		TKRA_Vec3F_Copy(svel, tdv);
		TKRA_Vec3F_Copy(sorg, tdo);
		tdo[2]+=1.25;

		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox, tdv)&1))
		{
			TKRA_Vec3F_Copy(tdo, dorg);
			TKRA_Vec3F_Copy(tdv, dvel);
			*rfl=fl;
			return(0);
		}
		
		for(i=-1; i<2; i++)
			for(j=-1; j<2; j++)
				for(k=-1; k<2; k++)
		{
			TKRA_Vec3F_Copy(sorg, tdo);
			tdo[0]+=k*0.25;
			tdo[1]+=j*0.25;
			tdo[2]+=i*0.25;
			if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox, tdv)&1))
			{
				TKRA_Vec3F_Copy(tdo, dorg);
				TKRA_Vec3F_Copy(tdv, dvel);
				*rfl=fl;
				return(0);
			}

		}
	}

	tdv[0]=0;
	tdv[1]=0;
	tdv[2]=0;

	TKRA_Vec3F_Copy(sorg, dorg);
	TKRA_Vec3F_Copy(tdv, dvel);
	*rfl=fl;
	return(0);
}

int BTM_CheckWorldMoveVel(BTM_World *wrl, float dt,
	float *sorg, float *svel,
	float *dorg, float *dvel, int *rfl)
{
//	static const float box[6] = { -0.35, -0.35, -1.7, 0.35, 0.35, 0.1};
	static const float box[6] = { -0.25, -0.25, -1.5, 0.25, 0.25, 0.1};
	return(BTM_CheckWorldBoxMoveVel(wrl, dt,
		sorg, svel, box, dorg, dvel, rfl));
}

int BTM_CheckWorldMoveVelSz(BTM_World *wrl, float dt,
	float *sorg, float *svel,
	float xrad, float zrad, float zofs,
	float *dorg, float *dvel, int *rfl)
{
//	static const float box[6] = { -0.35, -0.35, -1.7, 0.35, 0.35, 0.1};
	float box[6];
	
	box[0]=-xrad;	box[1]=-xrad;
	box[3]= xrad;	box[4]= xrad;
	box[2]=-zofs;	box[5]=-zofs+zrad;
	
	return(BTM_CheckWorldBoxMoveVel(wrl, dt,
		sorg, svel, box, dorg, dvel, rfl));
}
