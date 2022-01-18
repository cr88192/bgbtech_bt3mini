/*
( 7: 0): Side
(15: 8): Top
(23:16): Bottom, If(Side!=Top), Else Flags
(31:24): Flags

Flags:
  0x01: Not Drawn
  0x02: See-Through / Transparent
  0xC0: Solid Type
    0: Solid
    1: Non-Solid
    2: Fluid
 */

static const u32 btmgl_vox_atlas_side[256]={
0x05000000,		/* 00: null */
0x05000000,		/* 01: air (cave) */
0x05000000,		/* 02: air (open) */
0x05000000,		/* 03: air (open) */

0x00000101,		/* 04: hardstone */
0x00000202,		/* 05: dirt */
0x00000303,		/* 06: grass */
0x00000404,		/* 07: stone */

0x00001212, 0x00001313, 0x0000FFFF, 0x00000F0F,		/* 08-0B */
0x08007171, 0x08007373, 0x08009191, 0x00002323,		/* 0C-0F */
0x00002424, 0x00002525, 0x00002626, 0x00002727,		/* 10-13 */
0x00002828, 0x00002929, 0x00001F1F, 0x00000E0E,		/* 14-17 */
0x00000505, 0x00001414, 0x00001515, 0x00001616,		/* 18-1B */
0x00000707, 0x00000606, 0x00001717, 0x00003C3C,		/* 1C-1F */
0x00002020, 0x00002121, 0x0100E0E0, 0x0100E1E1,		/* 20-23 */
0x0100F1F1, 0x00001E1E, 0x00002E2E, 0x00002F2F,		/* 24-27 */
0x00001818, 0x00000808, 0x00000909, 0x00001919,		/* 28-2B */
0x00000A0A, 0x00001A1A, 0x00002A2A, 0x00000B0B,		/* 2C-2F */
0x00001B1B, 0x00003E3E, 0x00003333, 0x00003434,		/* 30-33 */
0x00003535, 0x00003636, 0x00003737, 0x00003838,		/* 34-37 */
0x00003939, 0x00000B0B, 0x00002B2B, 0x10003A3A,		/* 38-3B */
0x10003B3B, 0x0400EDED, 0x00001C1C, 0x00002C2C,		/* 3C-3F */
0x00003232, 0x00004242, 0x00003030, 0x00003434,		/* 40-43 */
0x00004444, 0x00004545, 0x00001414, 0x00001313,		/* 44-47 */
0x00005353, 0x00005454, 0x00005151, 0x00005252,		/* 48-4B */
0x00005555, 0x00005656, 0x00004F4F, 0x14003C3C,		/* 4C-4F */
0x00001010,		/* 50: bigbrick_brn */
0x00001111,		/* 51: bigbrick_gry */
};


int BTM_BlockIsFluidP(BTM_World *wrl, u32 blk)
{
	u32 blkd;
	
	blkd=btmgl_vox_atlas_side[blk&255];
	if(blkd&BTM_BLKDFL_FLUID)
		return(1);
	return(0);
}

int BTM_BlockIsAirP(BTM_World *wrl, u32 blk)
{
	int j;
	
	j=blk&255;
	return((j>0)&(j<4));
//	u32 blkd;
//	blkd=btmgl_vox_atlas_side[blk&255];
//	if(blkd&BTM_BLKDFL_FLUID)
//		return(1);
//	return(0);
}

int BTM_BlockIsTransparentP(BTM_World *wrl, u32 blk)
{
	u32 blkd;
	int j;
	
	j=blk&255;
	if((j>0)&&(j<4))
		return(1);
	
	blkd=btmgl_vox_atlas_side[j];
	if(blkd&(
			BTM_BLKDFL_FLUID|BTM_BLKDFL_NODRAW|
			BTM_BLKDFL_SEETHRU|BTM_BLKDFL_TY_MASK))
		return(1);
	return(0);
}

