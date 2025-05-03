label0:

goto body0

box1=(csgaabb mins=[-10,-10,-10],maxs=[10,10,10],color=0x55AAFF)
box2=(csgaabb mins=[-4,-4,9],maxs=[4,4,30],color=0xFFAA55)
box1n=(csgaabb mins=[-15,-4,-4],maxs=[15,4,4],color=0x55AAFF)
box3=(csgunion box1,box2)
box4=(csgdiff box3,box1n)
return box4


leg0:
temp box1=(csgaabb org=[0,0,16],size=[3,3,16],bone=(bidb+0))
temp box2=(csgaabb org=[0,0,0],size=[3,3,16],bone=(bidb+1))
temp box3=(csgaabb org=[0,0,0],size=[3,8,3],bone=(bidb+1))
temp un1=(csgunion box1,box2,box3,org=offs,color=0x555555)
// temp un1=(csgunion box1,box2,box3)
return un1

arm0:
temp box1=(csgaabb org=[0,0,-1.5],size=[12,3,3],color=0xAAAAAA,bone=(bidb+0))
temp box2=(csgaabb org=[12,0,-1.5],size=[12,3,3],color=0xAAAAAA,bone=(bidb+1))
temp box3=(csgaabb org=[24,-0.5,-1],size=[5,4,2],color=0xAA5500,bone=(bidb+2))
// temp box4=(csgaabb org=[33,2.2,1],size=[5,3,2],rot=[0,0,25],color=0xAA5500)
// temp un1b=(csgunion box1,box2,box3,box4,org=offs,angles=ang)
temp un1b=(csgunion box1,box2,box3,org=offs+[0,0,1.5],angles=ang)
// temp un1b=(csgunion box1,box2,box3,org=offs)
return un1b

neck0:
temp box1=(csgaabb org=[-1,0.5,0]+offs,size=[2,2,16],color=0xFF5555,bone=6)
return box1

head0:
temp cyl0=(csgcylinder org=[0,4.5,8],h=8,r1=1.5,r2=2.5,fn=4,color=0xFF5555)
temp box1=(csgcylinder org=[0,6.45,11],h=4,r1=0.5,r2=0.25,fn=6,color=0xFF5555)
temp eye1=(csgaabb org=[-1.75,6.8,14],size=[0.75,0.25,0.75],color=0x111111)
temp eye2=(csgaabb org=[ 1.0 ,6.8,14],size=[0.75,0.25,0.75],color=0x111111)
temp mouth1=(csgaabb org=[-1.0 ,6.4,9],size=[2.0,0.25,1.0],color=0x111111)
temp hat1=(csgcylinder org=[0,4.0,16],h=4,r1=3.0,r2=1.5,fn=4,color=0xAA5500)
temp un1=(csgunion cyl0,box1,eye1,eye2,mouth1,hat1,org=offs,bone=7)
return un1

body0:
temp box1=(csgaabb org=[-6,-1,32],size=[12,5,6])
temp box2=(csgaabb org=[-4,-0.5,33],size=[1,4,19.5])
temp box3=(csgaabb org=[ 3,-0.5,33],size=[1,4,19.5])
temp un1=(csgunion box1,box2,box3,color=0x555555,bone=1)

temp box4=(csgaabb org=[-5.25,-0.25,42],size=[10.5,3.5,1.0],color=0xFF55FF)
temp box5=(csgaabb org=[-5.25,-0.25,45],size=[10.5,3.5,1.0],color=0xFF55FF)
temp box6=(csgaabb org=[-5,0,38],size=[10,3,14],color=0xAAAAAA)
temp un2=(csgunion box4,box5,box6,bone=1)

temp leg1=gosub leg0 offs=[-5,0,0],bidb=2
temp leg2=gosub leg0 offs=[ 2,0,0],bidb=4
temp arm1=gosub arm0 offs=[ 5,0,48],ang=[0,0,0],bidb=12
temp arm2=gosub arm0 offs=[-5,0,48],ang=[180,0,180],bidb=8

temp neck1=gosub neck0 offs=[0,0,48]
temp head1=gosub head0 offs=[0,-1,48]

temp un3=(csgunion leg1,leg2,arm1,arm2,neck1,head1)
// temp un3=(csgunion leg1,leg2,arm1)
// temp un3=(csgunion leg1,leg2)

temp un4=(csgunion un1,un2,un3)

return un4
