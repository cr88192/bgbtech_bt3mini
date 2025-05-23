void QuatF_Identity(float *a)
{
	a[0]=0;
	a[1]=0;
	a[2]=0;
	a[3]=1;
}

void QuatF_Negate(float *a, float *b)
{
	b[0]=-a[0]; b[1]=-a[1]; b[2]=-a[2]; b[3]=-a[3];
}

void QuatF_Conjugate(float *a, float *b)
{
//	int i;
//	for(i=0; i<4; i++)b[i]=-a[i];
	b[0]=-a[0]; b[1]=-a[1]; b[2]=-a[2]; b[3]=a[3];
}

void QuatF_Recip(float *a, float *b)
{
	float f;
//	ci[0]=-(a[3]*a[0])+(a[0]*a[3])-(a[1]*a[2])+(a[2]*a[1]);
//	ci[1]=(a[3]*b[1])+(a[1]*b[3])+(a[2]*b[0])-(a[0]*b[2]);
//	ci[2]=(a[3]*b[2])+(a[2]*b[3])+(a[0]*b[1])-(a[1]*b[0]);

	f=(a[3]*a[3])+(a[0]*a[0])+(a[1]*a[1])+(a[2]*a[2]);
	b[0]=-a[0]/f;
	b[1]=-a[1]/f;
	b[2]=-a[2]/f;
	b[3]=a[3]/f;
}

float QuatF_Magnitude(float *a)
{
	float f;

	f=(a[0]*a[0])+(a[1]*a[1])+(a[2]*a[2])+(a[3]*a[3]);
	return(sqrt(f));
}

float QuatF_Normalize(float *a, float *b)
{
	float f, g;

	f=QuatF_Magnitude(a);

	g=(f>0)?f:1;
	b[0]=a[0]/g;
	b[1]=a[1]/g;
	b[2]=a[2]/g;
	b[3]=a[3]/g;

	return(f);
}

void QuatF_Multiply(float *a, float *b, float *c)
{
	float ci[4];
	ci[0]=(a[3]*b[0])+(a[0]*b[3])+(a[1]*b[2])-(a[2]*b[1]);
	ci[1]=(a[3]*b[1])+(a[1]*b[3])+(a[2]*b[0])-(a[0]*b[2]);
	ci[2]=(a[3]*b[2])+(a[2]*b[3])+(a[0]*b[1])-(a[1]*b[0]);
	ci[3]=(a[3]*b[3])-(a[0]*b[0])-(a[1]*b[1])-(a[2]*b[2]);
	QuatF_Normalize(ci, c);
}

void QuatF_ToMatrix(float *a, float *b)
{
#if 1
	float xx, xy, xz, xw, yy, yz, yw, zz, zw;
	int i;

	for(i=0; i<16; i++)b[i]=0;
	for(i=0; i<4; i++)b[(i*4)+i]=1;

	xx=a[0]*a[0];
	xy=a[0]*a[1];
	xz=a[0]*a[2];
	xw=a[0]*a[3];
	yy=a[1]*a[1];
	yz=a[1]*a[2];
	yw=a[1]*a[3];
	zz=a[2]*a[2];
	zw=a[2]*a[3];
#endif

#if 1
	b[0]= 1-2*(yy+zz);
	b[1]=   2*(xy+zw);
	b[2]=   2*(xz-yw);

	b[4]=   2*(xy-zw);
	b[5]= 1-2*(xx+zz);
	b[6]=   2*(yz+xw);

	b[8]=   2*(xz+yw);
	b[9]=   2*(yz-xw);
	b[10]=1-2*(xx+yy);
#endif
#if 0
	b[0]= 1-(2*yy)-(2*zz);
	b[1]=   (2*xy)-(2*zw);
	b[2]=   (2*xz)+(2*yw);

	b[4]=   (2*xy)+(2*zw);
	b[5]= 1-(2*xx)-(2*zz);
	b[6]=   (2*yz)-(2*xw);

	b[8]=   (2*xz)-(2*yw);
	b[9]=   (2*yz)+(2*xw);
	b[10]=1-(2*xx)-(2*yy);
#endif
}

void QuatF_To3Matrix(float *a, float *b)
{
	float xx, xy, xz, xw, yy, yz, yw, zz, zw;
	int i;

	for(i=0; i<9; i++)b[i]=0;
	for(i=0; i<3; i++)b[(i*3)+i]=1;

	xx=a[0]*a[0];
	xy=a[0]*a[1];
	xz=a[0]*a[2];
	xw=a[0]*a[3];
	yy=a[1]*a[1];
	yz=a[1]*a[2];
	yw=a[1]*a[3];
	zz=a[2]*a[2];
	zw=a[2]*a[3];

#if 1
	b[0]= 1-2*(yy+zz);
	b[1]=   2*(xy+zw);
	b[2]=   2*(xz-yw);

	b[3]=   2*(xy-zw);
	b[4]= 1-2*(xx+zz);
	b[5]=   2*(yz+xw);

	b[6]=   2*(xz+yw);
	b[7]=   2*(yz-xw);
	b[8]=1-2*(xx+yy);
#endif
#if 0
	b[0]=1-(2*yy)-(2*zz);
	b[1]=  (2*xy)-(2*zw);
	b[2]=  (2*xz)+(2*yw);
	b[3]=  (2*xy)+(2*zw);
	b[4]=1-(2*xx)-(2*zz);
	b[5]=  (2*yz)-(2*xw);
	b[6]=  (2*xz)-(2*yw);
	b[7]=  (2*yz)+(2*xw);
	b[8]=1-(2*xx)-(2*yy);
#endif
}

void QuatF_FromMatrix(float *a, float *b)
{
	float f, g;

	f=1+a[0]+a[5]+a[10];
	if(f>0.0001)
	{
		g=sqrt(f)*2;
		b[0]=(a[9]-a[6])/g;
		b[1]=(a[2]-a[8])/g;
		b[2]=(a[4]-a[1])/g;
		b[3]=g*0.25;
		return;
	}

	if((a[0]>a[5]) && (a[0]>a[10]))
	{
		f=sqrt(1+a[0]-a[5]-a[10])*2;
		b[0]=0.25*f;
		b[1]=(a[4]+a[1])/f;
		b[2]=(a[2]+a[8])/f;
		b[3]=(a[9]-a[6])/f;
	}else if(a[5]>a[10])
	{
		f=sqrt(1+a[5]-a[0]-a[10])*2;
		b[0]=(a[4]+a[1])/f;
		b[1]=0.25*f;
		b[2]=(a[9]+a[6])/f;
		b[3]=(a[2]-a[8])/f;
	}else
	{
		f=sqrt(1+a[10]-a[0]-a[5])*2;
		b[0]=(a[2]+a[8])/f;
		b[1]=(a[9]+a[6])/f;
		b[2]=0.25*f;
		b[3]=(a[4]-a[1])/f;
	}
}

void QuatF_From3Matrix(float *a, float *b)
{
	float f, g;

	f=1+a[0]+a[4]+a[8];
	if(f>0.0001)
	{
		g=sqrt(f)*2;
		b[0]=(a[7]-a[5])/g;
		b[1]=(a[2]-a[6])/g;
		b[2]=(a[3]-a[1])/g;
		b[3]=g*0.25;
		return;
	}

	if((a[0]>a[4]) && (a[0]>a[8]))
	{
		f=sqrt(1+a[0]-a[4]-a[8])*2;
		b[0]=0.25*f;
		b[1]=(a[3]+a[1])/f;
		b[2]=(a[2]+a[6])/f;
		b[3]=(a[7]-a[5])/f;
	}else if(a[5]>a[10])
	{
		f=sqrt(1+a[4]-a[0]-a[8])*2;
		b[0]=(a[3]+a[1])/f;
		b[1]=0.25*f;
		b[2]=(a[7]+a[5])/f;
		b[3]=(a[2]-a[6])/f;
	}else
	{
		f=sqrt(1+a[8]-a[0]-a[4])*2;
		b[0]=(a[2]+a[6])/f;
		b[1]=(a[7]+a[5])/f;
		b[2]=0.25*f;
		b[3]=(a[3]-a[1])/f;
	}
}


void QuatF_FromAxis(float *a, float b, float *c)
{
	float v[3], sa, ca;

	sa=sin(b/2);
	ca=cos(b/2);
	TKRA_Vec3F_Normalize(a, v);

	c[0]=v[0]*sa;
	c[1]=v[1]*sa;
	c[2]=v[2]*sa;
	c[3]=ca;
}

void QuatF_ToAxis(float *a, float *b, float *c)
{
	float v[4], sa, ca, th;

	QuatF_Normalize(a, v);
	ca=v[3];
	th=acos(ca)*2;
	sa=sqrt(1.0-(ca*ca));
	if(fabs(sa)<0.0005)sa=1;

	b[0]=v[0]/sa;
	b[1]=v[1]/sa;
	b[2]=v[2]/sa;
	*c=th;
}

void QuatF_FromAngles(float *a, float *b)
{
#if 0
	float qx[4], qy[4], qz[4], qt[4];

//	QuatF_FromAxis(kx, b[2], qz);
//	QuatF_FromAxis(jx, b[1], qy);
//	QuatF_FromAxis(ix, b[0], qx);

	QuatF_FromAxis(ix, b[0], qx);
	QuatF_FromAxis(jx, b[1], qy);
	QuatF_FromAxis(kx, b[2], qz);
	QuatF_Multiply(qx, qy, qt);
	QuatF_Multiply(qt, qz, b);
#endif
	float sx, cx, sy, cy, sz, cz;

	sx=sin(a[0]*0.5);
	cx=cos(a[0]*0.5);
	sy=sin(a[1]*0.5);
	cy=cos(a[1]*0.5);
	sz=sin(a[2]*0.5);
	cz=cos(a[2]*0.5);
	
	if((sx!= 0.0) && (fabs(sx    )<0.000001))	sx= 0.0;
	if((cx!= 0.0) && (fabs(cx    )<0.000001))	cx= 0.0;
	if((sx!= 1.0) && (fabs(sx-1.0)<0.000001))	sx= 1.0;
	if((cx!= 1.0) && (fabs(cx-1.0)<0.000001))	cx= 1.0;
	if((sx!=-1.0) && (fabs(sx+1.0)<0.000001))	sx=-1.0;
	if((cx!=-1.0) && (fabs(cx+1.0)<0.000001))	cx=-1.0;

	if((sy!= 0.0) && (fabs(sy    )<0.000001))	sy= 0.0;
	if((cy!= 0.0) && (fabs(cy    )<0.000001))	cy= 0.0;
	if((sy!= 1.0) && (fabs(sy-1.0)<0.000001))	sy= 1.0;
	if((cy!= 1.0) && (fabs(cy-1.0)<0.000001))	cy= 1.0;
	if((sy!=-1.0) && (fabs(sy+1.0)<0.000001))	sy=-1.0;
	if((cy!=-1.0) && (fabs(cy+1.0)<0.000001))	cy=-1.0;

	if((sz!= 0.0) && (fabs(sz    )<0.000001))	sz= 0.0;
	if((cz!= 0.0) && (fabs(cz    )<0.000001))	cz= 0.0;
	if((sz!= 1.0) && (fabs(sz-1.0)<0.000001))	sz= 1.0;
	if((cz!= 1.0) && (fabs(cz-1.0)<0.000001))	cz= 1.0;
	if((sz!=-1.0) && (fabs(sz+1.0)<0.000001))	sz=-1.0;
	if((cz!=-1.0) && (fabs(cz+1.0)<0.000001))	cz=-1.0;

	b[0]=sx*cy*cz-cx*sy*sz; // X
	b[1]=cx*sy*cz+sx*cy*sz; // Y
	b[2]=cx*cy*sz-sx*sy*cz; // Z
	b[3]=cx*cy*cz+sx*sy*sz; // W
	
	QuatF_Normalize(b, b);
}

void QuatF_FromAnglesB(float *a, float *b)
{
	static float ix[3]={1,0,0};
	static float jx[3]={0,1,0};
	static float kx[3]={0,0,1};

	float qx[4], qy[4], qz[4], qt[4];

//	QuatF_FromAxis(kx, a[2], qz);
//	QuatF_FromAxis(jx, a[1], qy);
//	QuatF_FromAxis(ix, a[0], qx);

	QuatF_FromAxis(ix, a[0], qx);
	QuatF_FromAxis(jx, a[1], qy);
	QuatF_FromAxis(kx, a[2], qz);
	QuatF_Multiply(qx, qy, qt);
	QuatF_Multiply(qt, qz, b);
}

void QuatF_ToAngles(float *a, float *b)
{
	float u[4], v[4];

	TKRA_Vec4F_Zero(u);
	u[1]=1;
	QuatF_Multiply(a, u, v);
	b[0]=atan2(v[2], v[1]);

	TKRA_Vec4F_Zero(u);
	u[0]=1;
	QuatF_Multiply(a, u, v);
	b[1]=atan2(v[2], v[0]);

	TKRA_Vec4F_Zero(u);
	u[0]=1;
	QuatF_Multiply(a, u, v);
	b[2]=atan2(v[1], v[0]);
}

void QuatF_Lerp(float *a, float *b, float t, float *c)
{
	TKRA_Vec4F_ScaleAddScale(a, 1-t, b, t, c);
	QuatF_Normalize(c, c);
}

void QuatF_SLerp(float *a, float *ib, float t, float *c)
{
	float e, f, g, h;
	float b[4];

//	VC4(b, ib);
	TKRA_Vec4F_Copy(ib, b);
	if(a[0]==b[0] && a[1]==b[1] && 
		a[2]==b[2] && a[3]==b[3])
	{
//		VC4(c, a);
		TKRA_Vec4F_Copy(a, c);
		return;
	}

//	f=VD4(a, b);
	f=TKRA_Vec4F_DotProduct(a, b);
	if(f<0)
	{
		QuatF_Conjugate(b, b);
		f=-f;
	}

	if((1-f)>0.1)
//	if(1)
	{
		e=acos(f);
		g=sin((1-t)*e)/sin(e);
		h=sin((t*e))/sin(e);
	}else
	{
		g=1-t;
		h=t;
	}

	c[0]=(g*a[0])+(h*b[0]);
	c[1]=(g*a[1])+(h*b[1]);
	c[2]=(g*a[2])+(h*b[2]);
	c[3]=(g*a[3])+(h*b[3]);
}


int QuatF_TransformPoint(float *iv, float *ov, float *trans)
{
	float tv[3];

	tv[0]=
		(iv[0]*trans[0])+
		(iv[1]*trans[4])+
		(iv[2]*trans[8])+
		trans[12];
	tv[1]=
		(iv[0]*trans[1])+
		(iv[1]*trans[5])+
		(iv[2]*trans[9])+
		trans[13];
	tv[2]=
		(iv[0]*trans[2])+
		(iv[1]*trans[6])+
		(iv[2]*trans[10])+
		trans[14];
	ov[0]=tv[0];
	ov[1]=tv[1];
	ov[2]=tv[2];
	return(0);
}

int QuatF_TransformNormal(float *iv, float *ov, float *trans)
{
	float tv[3];

	tv[0]=
		(iv[0]*trans[0])+
		(iv[1]*trans[4])+
		(iv[2]*trans[8]);
	tv[1]=
		(iv[0]*trans[1])+
		(iv[1]*trans[5])+
		(iv[2]*trans[9]);
	tv[2]=
		(iv[0]*trans[2])+
		(iv[1]*trans[6])+
		(iv[2]*trans[10]);
	ov[0]=tv[0];
	ov[1]=tv[1];
	ov[2]=tv[2];
	return(0);
}


int QuatF_TransformPlane(float *inorm, float *onorm, float *trans)
{
	float txy0[3], txy1[3], tnv0[4], tnv1[4];
	
	TKRA_Vec3F_Scale(inorm, inorm[3], txy0);
	QuatF_TransformPoint(txy0, txy1, trans);

	QuatF_TransformNormal(inorm, tnv0, trans);
	TKRA_Vec3F_Normalize(tnv0, tnv1);
	tnv1[3]=TKRA_Vec3F_DotProduct(txy1, tnv1);
	TKRA_Vec4F_Copy(tnv1, onorm);
	return(0);
}
