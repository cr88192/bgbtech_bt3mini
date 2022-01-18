int BTM_CheckWorldMoveBlockContents(BTM_World *wrl, int btm)
{
	u32 blkd;

	blkd=btmgl_vox_atlas_side[btm&0xFF];

	if(blkd&BTM_BLKDFL_NONSOLID)
		{ return(0); }
	if(blkd&BTM_BLKDFL_FLUID)
		{ return(2); }

	return(1);
}

int BTM_CheckWorldMoveSpot(BTM_World *wrl, float *org, const float *bbox)
{
	int cont;
	int cxm, cxn, cym, cyn, czm, czn;
	u32 blk0, blk1, blk2, blk3;
	u32 blk4, blk5, blk6, blk7;

//	cxm=org[0]-0.35;
//	cxn=org[0]+0.35;
//	cym=org[1]-0.35;
//	cyn=org[1]+0.35;

//	czm=org[2]-1.7;
//	czn=org[2]+0.1;

	cxm=org[0]+bbox[0];
	cxn=org[0]+bbox[3];
	cym=org[1]+bbox[1];
	cyn=org[1]+bbox[4];

	czm=org[2]+bbox[2];
	czn=org[2]+bbox[5];

//	blk0=BTM_GetWorldBlockXYZ(wrl, org[0], org[1], org[2]-2);
//	blk1=BTM_GetWorldBlockXYZ(wrl, org[0], org[1], org[2]-1);

	blk0=BTM_GetWorldBlockXYZ(wrl, cxm, cym, czm);
	blk1=BTM_GetWorldBlockXYZ(wrl, cxn, cym, czm);
	blk2=BTM_GetWorldBlockXYZ(wrl, cxm, cyn, czm);
	blk3=BTM_GetWorldBlockXYZ(wrl, cxn, cyn, czm);

	cont=0;

	cont|=BTM_CheckWorldMoveBlockContents(wrl, blk0);
	cont|=BTM_CheckWorldMoveBlockContents(wrl, blk1);
	cont|=BTM_CheckWorldMoveBlockContents(wrl, blk2);
	cont|=BTM_CheckWorldMoveBlockContents(wrl, blk3);

//	if(((blk0&255)>=2) && BTM_CheckWorldMoveBlockSolidP(wrl, blk0))
//		cont|=1;
//	if(((blk1&255)>=2) && BTM_CheckWorldMoveBlockSolidP(wrl, blk1))
//		cont|=1;
//	if(((blk2&255)>=2) && BTM_CheckWorldMoveBlockSolidP(wrl, blk2))
//		cont|=1;
//	if(((blk3&255)>=2) && BTM_CheckWorldMoveBlockSolidP(wrl, blk3))
//		cont|=1;

	blk4=BTM_GetWorldBlockXYZ(wrl, cxm, cym, czn);
	blk5=BTM_GetWorldBlockXYZ(wrl, cxn, cym, czn);
	blk6=BTM_GetWorldBlockXYZ(wrl, cxm, cyn, czn);
	blk7=BTM_GetWorldBlockXYZ(wrl, cxn, cyn, czn);

//	if(((blk4&255)>=2) && BTM_CheckWorldMoveBlockSolidP(wrl, blk4))
//		cont|=1;
//	if(((blk5&255)>=2) && BTM_CheckWorldMoveBlockSolidP(wrl, blk5))
//		cont|=1;
//	if(((blk6&255)>=2) && BTM_CheckWorldMoveBlockSolidP(wrl, blk6))
//		cont|=1;
//	if(((blk7&255)>=2) && BTM_CheckWorldMoveBlockSolidP(wrl, blk7))
//		cont|=1;

	cont|=BTM_CheckWorldMoveBlockContents(wrl, blk4);
	cont|=BTM_CheckWorldMoveBlockContents(wrl, blk5);
	cont|=BTM_CheckWorldMoveBlockContents(wrl, blk6);
	cont|=BTM_CheckWorldMoveBlockContents(wrl, blk7);

	if((blk4|blk5|blk6|blk7)&2)
		cont|=4;

//	if(((blk0&255)<2) && ((blk1&255)<2))
//		return(1);

	return(cont);
}

int BTM_CheckWorldBoxMoveVel(BTM_World *wrl, float dt,
	float *sorg, float *svel, const float *bbox,
	float *dorg, float *dvel, int *rfl)
{
	float tdo[4], tdv[4];
	int fl, cfl;
	int i, j, k;

	if(dt>0.1)
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

	cfl=BTM_CheckWorldMoveSpot(wrl, tdo, bbox);

	if(!(cfl&1))
	{
		if(svel[2]<0)
			fl&=~1;

		if(cfl&2)		
			fl|=2;
		else
			fl&=~2;

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
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox)&1))
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
		tdo[2]+=0.5;
		
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox)&1))
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
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox)&1))
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
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox)&1))
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
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox)&1))
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
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox)&1))
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
		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox)&1))
		{
			TKRA_Vec3F_Copy(tdo, dorg);
			TKRA_Vec3F_Copy(tdv, dvel);
			*rfl=fl;
			return(0);
		}
	}

	if(BTM_CheckWorldMoveSpot(wrl, sorg, bbox)&1)
	{
		tdv[0]=0;
		tdv[1]=0;
		tdv[2]=0;

//		TKRA_Vec3F_Copy(svel, tdv);
		TKRA_Vec3F_Copy(sorg, tdo);
		tdo[2]+=1.25;

		if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox)&1))
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
			if(!(BTM_CheckWorldMoveSpot(wrl, tdo, bbox)&1))
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
	static const float box[6] = { -0.35, -0.35, -1.7, 0.35, 0.35, 0.1};
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
	box[2]=-zofs;	box[4]=-zofs+zrad;
	
	return(BTM_CheckWorldBoxMoveVel(wrl, dt,
		sorg, svel, box, dorg, dvel, rfl));
}
