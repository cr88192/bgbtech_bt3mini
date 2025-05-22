label0:
temp box0=(csgaabb mins=[-40,-20,0],maxs=[40,20,24])
temp box1=(csgaabb mins=[-40,-20,0],maxs=[40,-10,60])
temp box2=(csgaabb mins=[-40,-20,0],maxs=[-30,20,35])
temp box3=(csgaabb mins=[30,-20,0],maxs=[40,20,35])
temp un1=(csgunion box0,box1,box2,box3,color=0xFF5555)
return un1

chair0:
temp box0=(csgaabb mins=[-15,-15, 0],maxs=[-13,-13,60])
temp box1=(csgaabb mins=[ 13,-15, 0],maxs=[ 15,-13,60])
temp box2=(csgaabb mins=[-15, 13, 0],maxs=[-13, 15,25])
temp box3=(csgaabb mins=[ 13, 13, 0],maxs=[ 15, 15,25])
temp box4=(csgaabb mins=[-15,-15,45],maxs=[ 15,-13,60])
temp box5=(csgaabb mins=[-15,-15,23],maxs=[ 15, 15,25])
temp un1=(csgunion box0,box1,box2,box3,box4,box5,color=0xFFAA55)
return un1
