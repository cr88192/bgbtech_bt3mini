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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>


#ifndef BTM_ENGINE_H
#define BTM_ENGINE_H

#ifndef M_PI
#define M_PI	3.1415926535897932384626433832795
#endif

/*
DrawSpan Parameter Array
*/
#define		BTMRA_DS_CTX		0			//context
#define		BTMRA_DS_TEXIMG	1			//texture image
#define		BTMRA_DS_TPOS	2			//texture position
#define		BTMRA_DS_TSTEP	3			//texture step
#define		BTMRA_DS_XMASK	4			//texture position
#define		BTMRA_DS_YMASK	5			//texture step
#define		BTMRA_DS_CPOS	6			//color position
#define		BTMRA_DS_CSTEP	7			//color step
#define		BTMRA_DS_ZPOS	8			//color position
#define		BTMRA_DS_ZSTEP	9			//color step
#define		BTMRA_DS_TEXBCN	10			//texture image (BCn)
#define		BTMRA_DS_BLEND	11			//Blend Function
#define		BTMRA_DS_ZATEST	12			//Depth+Alpha Test
#define		BTMRA_DS_NPARM	16			//number of drawspan params

/*
Edge Parameter Arrays
 */
#define		BTMRA_ES_XPOS	0			//screen X position
#define		BTMRA_ES_XSTEP	1			//screen X step
#define		BTMRA_ES_TPOS	2			//texture position
#define		BTMRA_ES_TSTEP	3			//texture step
#define		BTMRA_ES_CPOS	4			//color position
#define		BTMRA_ES_CSTEP	5			//color step
#define		BTMRA_ES_ZPOS	6			//Z position
#define		BTMRA_ES_ZSTEP	7			//Z step
#define		BTMRA_ES_NPARM	8			//number of edge params

typedef uint8_t	byte;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;
// typedef unsigned long long	u64;

typedef int8_t		sbyte;
typedef int16_t	s16;
typedef int32_t	s32;
typedef int64_t	s64;
// typedef signed long long	s64;

#include "cca_ast2.h"


#define	btmra_mulhi(a, b)				((((s64)a)*((s64)b))>>32)
#define	btmra_mulhi1(a, b)				((((s64)a)*((s64)b))>>31)
#define	btmra_mulhi8(a, b)				((((s64)a)*((s64)b))>>24)
#define	btmra_mulhi9(a, b)				((((s64)a)*((s64)b))>>23)
#define	btmra_mulhi16(a, b)				((((s64)a)*((s64)b))>>16)
#define	btmra_mulhi_lsh(a, b, sh)		((((s64)a)*((s64)b))>>(32-sh))

#define	btmra_mullo8(a, b)				((((s64)a)*((s64)b))>>8)

#define BTM_BLKTY_NULL				0x00	/* NULL / Void */
#define BTM_BLKTY_AIR1				0x01	/* Cave Air */
#define BTM_BLKTY_AIR2				0x02	/* Open Air, Normal */
#define BTM_BLKTY_AIR3				0x03	/* Open Air, Sky */

#define BTM_BLKTY_HARDSTONE			0x04
#define BTM_BLKTY_DIRT				0x05
#define BTM_BLKTY_GRASS				0x06
#define BTM_BLKTY_STONE				0x07

#define BTM_BLKTY_BRICK_RED			0x08
#define BTM_BLKTY_STONE_RED			0x09
#define BTM_BLKTY_SOLIDNOFACE		0x0A
#define BTM_BLKTY_GLASS				0x0B
#define BTM_BLKTY_LAVA				0x0C
#define BTM_BLKTY_WATER				0x0D
#define BTM_BLKTY_SLIME				0x0E

#define BTM_BLKTY_LANTERN_RED		0x0F
#define BTM_BLKTY_LANTERN_BLUE		0x10
#define BTM_BLKTY_LANTERN_YEL		0x11
#define BTM_BLKTY_LANTERN_GRN		0x12
#define BTM_BLKTY_LANTERN_CYA		0x13
#define BTM_BLKTY_LANTERN_VIO		0x14
#define BTM_BLKTY_LANTERN_WHI		0x15

#define BTM_BLKTY_GRATE				0x16
#define BTM_BLKTY_BARS				0x17
#define BTM_BLKTY_STONE2			0x18
#define BTM_BLKTY_STONE3			0x19
#define BTM_BLKTY_BIGBRICK_STONE	0x1A
#define BTM_BLKTY_HAY				0x1B

#define BTM_BLKTY_TRIDIRT0			0x1C
#define BTM_BLKTY_TRIDIRT1			0x1D
#define BTM_BLKTY_TRIDIRT2			0x1E
#define BTM_BLKTY_TRIDIRT3			0x1F

// #define BTM_BLKTY_ENTMOB			0x1F
// #define BTM_BLKTY_FLIPUP			0x20
// #define BTM_BLKTY_FLIPDN			0x21
#define BTM_BLKTY_FARM1				0x20
#define BTM_BLKTY_FARM2				0x21

#define BTM_BLKTY_SKY1				0x22
#define BTM_BLKTY_SKY2				0x23
#define BTM_BLKTY_SKY3				0x24
#define BTM_BLKTY_FENCE1			0x25
#define BTM_BLKTY_FENCE2			0x26
#define BTM_BLKTY_FENCE3			0x27
#define BTM_BLKTY_BRICKS			0x28
#define BTM_BLKTY_LOG				0x29
#define BTM_BLKTY_LEAVES			0x2A
#define BTM_BLKTY_FRUITLEAVES		0x2B
#define BTM_BLKTY_SANDSTONE			0x2C
#define BTM_BLKTY_SAND				0x2D
#define BTM_BLKTY_SNOW				0x2E
#define BTM_BLKTY_COBBLE			0x2F

#define BTM_BLKTY_GREENCOBBLE		0x30
#define BTM_BLKTY_GRASSCLUMP		0x31
#define BTM_BLKTY_SOMATRED			0x32
#define BTM_BLKTY_SOMATBLU			0x33
#define BTM_BLKTY_SOMATYEL			0x34
#define BTM_BLKTY_SOMATGRN			0x35
#define BTM_BLKTY_SOMATCYN			0x36
#define BTM_BLKTY_SOMATPUR			0x37
#define BTM_BLKTY_SOMATWHT			0x38
#define BTM_BLKTY_STAIRS			0x39
#define BTM_BLKTY_GUNGEROCK			0x3A
#define BTM_BLKTY_STONESLAB			0x3B
#define BTM_BLKTY_WOODSLAB			0x3C
#define BTM_BLKTY_TRIGGER			0x3D
#define BTM_BLKTY_PLANKS			0x3E
#define BTM_BLKTY_PLANKS2			0x3F

#define BTM_BLKTY_SOMATBLK			0x40
#define BTM_BLKTY_BRICKBLK			0x41
#define BTM_BLKTY_CLOUD				0x42
#define BTM_BLKTY_BRICKBLU			0x43
#define BTM_BLKTY_STONE2BLU			0x44
#define BTM_BLKTY_BRICKGRN			0x45
#define BTM_BLKTY_COBBLEBLU			0x46
#define BTM_BLKTY_COBBLERED			0x47
#define BTM_BLKTY_STONE3BLU			0x48
#define BTM_BLKTY_BIGBRICK_BLU		0x49
#define BTM_BLKTY_HEXTILE			0x4A
#define BTM_BLKTY_BRICK2RED			0x4B
#define BTM_BLKTY_MYCELIUM			0x4C
#define BTM_BLKTY_REDGRASS			0x4D
#define BTM_BLKTY_LADDER			0x4E
#define BTM_BLKTY_HAYNEST			0x4F

#define BTM_BLKTY_BIGBRICK_BRN		0x50
#define BTM_BLKTY_BIGBRICK_GRY		0x51

#define BTM_BLKTY_WHEAT				0x52
#define BTM_BLKTY_CARROTS			0x53
#define BTM_BLKTY_BEETS				0x54
#define BTM_BLKTY_POTATOES			0x55
#define BTM_BLKTY_PUMPKIN_VINE		0x56
#define BTM_BLKTY_MELLON_VINE		0x57

#define BTM_BLKTY_PLANKS3			0x58
#define BTM_BLKTY_PLANKS4			0x59

#define BTM_BLKTY_SCONCE			0x5A

#define BTM_BLKPAT_AIR3_DFL			0x00F00003	/* Open Air, Sky */

#define BTM_BLKDFL_NODRAW		0x01000000U		//Not Drawn
#define BTM_BLKDFL_SEETHRU		0x02000000U		//Does not block visibility.
#define BTM_BLKDFL_NONSOLID		0x04000000U		//Block is Non-Solid.
#define BTM_BLKDFL_FLUID		0x08000000U		//Block is Fluid

#define BTM_BLKDFL_TY_MASK		0xF0000000U		//Block is a Slab
#define BTM_BLKDFL_TY_FULL		0x00000000U		//Full Block
#define BTM_BLKDFL_TY_SLAB		0x10000000U		//Block is a Slab
#define BTM_BLKDFL_TY_STAIR		0x20000000U		//Block is Stairs
#define BTM_BLKDFL_TY_PLANT		0x30000000U		//Block is Plant
#define BTM_BLKDFL_TY_CROP		0x40000000U		//Block is Plant (that grows)


#define BTM_CVTY_INT			0x0000
#define BTM_CVTY_LONG			0x0001
#define BTM_CVTY_FLOAT			0x0002
#define BTM_CVTY_DOUBLE			0x0003
#define BTM_CVTY_STRING			0x0004

#define BTM_CVTY_SBYTE			0x0008
#define BTM_CVTY_UBYTE			0x0009
#define BTM_CVTY_SHORT			0x000A
#define BTM_CVTY_USHORT			0x000B

#define BTM_CVTY_BOOL			0x003F

#define BTM_UITY_NONE			0x0000
#define BTM_UITY_PERCENT100		0x0100	//0..100 -> 0% .. 100%
#define BTM_UITY_PERCENT256		0x0200	//00..FF -> 0% .. 100%

#ifdef __BJX2__
#define BTM_TARGET_SMALL
#define BTM_TARGET_DRAWDIST	28
#endif

#ifndef BTM_TARGET_DRAWDIST
#define BTM_TARGET_DRAWDIST	128
// #define BTM_HITEXPAMD		3
// #define BTM_HITEXPAMD		4
// #define BTM_HITEXPAMD		5
// #define BTM_HITEXPAMD		7
#endif

#define BRM_VART16_SMALLLIT		0x1000ULL
#define BRM_VART16_SMALLSTRA	0x1001ULL
#define BRM_VART16_SMALLSTRB	0x1002ULL

#define BRM_PGMVAR_PTR			(0x0ULL<<60)
#define BRM_PGMVAR_FIXNUM		(0x4ULL<<60)
#define BRM_PGMVAR_FLONUM		(0x8ULL<<60)

#define BRM_PGMVAR_SMALLLIT		(0x1000ULL<<48)
#define BRM_PGMVAR_SMALLSTRA	(0x1001ULL<<48)
#define BRM_PGMVAR_SMALLSTRB	(0x1002ULL<<48)


#define	BRM_PGMVAR_NULL			(BRM_PGMVAR_SMALLLIT+0)
#define	BRM_PGMVAR_UNDEFINED	(BRM_PGMVAR_SMALLLIT+1)

#define BRM_PGMOPR_ADD			0
#define BRM_PGMOPR_SUB			1
#define BRM_PGMOPR_MUL			2
#define BRM_PGMOPR_DIV			3
#define BRM_PGMOPR_MOD			4
#define BRM_PGMOPR_AND			5
#define BRM_PGMOPR_OR			6
#define BRM_PGMOPR_XOR			7
#define BRM_PGMOPR_SHL			8
#define BRM_PGMOPR_SHR			9

#define BRM_PGMOPR_EQ			10
#define BRM_PGMOPR_NE			11
#define BRM_PGMOPR_GT			12
#define BRM_PGMOPR_LE			13
#define BRM_PGMOPR_LT			14
#define BRM_PGMOPR_GE			15

#define BRM_PGMOPR_NOT			16
#define BRM_PGMOPR_BITNOT		17


typedef u16		btmra_rastpixel;
typedef u16		btmra_zbufpixel;

typedef struct BTM_World_s BTM_World;
typedef struct BTM_Region_s BTM_Region;
typedef struct BTM_Screen_s BTM_Screen;
typedef struct BTM_TexImg_s BTM_TexImg;

typedef struct BTM_MobEntity_s BTM_MobEntity;
typedef struct BTM_MobSprite_s BTM_MobSprite;

#ifdef BTM_TARGET_SMALL

#define	BTM_RAYCAST_MAXHITS		16384
#define	BTM_RAYCAST_HASHSZ		256

#else

// #define	BTM_RAYCAST_MAXHITS		16384
// #define	BTM_RAYCAST_MAXHITS		32768
// #define	BTM_RAYCAST_MAXHITS		65536
// #define	BTM_RAYCAST_MAXHITS			131072
#define	BTM_RAYCAST_MAXHITS			262144
// #define	BTM_RAYCAST_HASHSZ			1024
// #define	BTM_RAYCAST_HASHSZ			4096
#define	BTM_RAYCAST_HASHSZ			16384

#endif

#define BTM_MOVETYPE_NONE	0
#define BTM_MOVETYPE_SLIDE	1

#define BTM_SOLIDTYPE_NONE		0	//non-solid
#define BTM_SOLIDTYPE_MOB		1	//mob-like
#define BTM_SOLIDTYPE_MODEL		2	//solid comes from 3D model

#define BTM_INVEN_NOTHING		0
#define BTM_INVEN_CUBE			1
#define BTM_INVEN_COIN			2
#define BTM_INVEN_CYLINDER		3
#define BTM_INVEN_MILK			4
#define BTM_INVEN_COOKIE		5
#define BTM_INVEN_SCROLL		6
#define BTM_INVEN_PEANUTBUTTER	7

#define BTM_INVEN_JUNK			0x1000
#define BTM_INVEN_PBJUNK		0x2000


struct BTM_World_s {
// u32		*vox;
// byte	xsh;
// byte	zsh;
// u64		*voxbm;		//solid bitmap

u32	magic1;

byte *tg_nmap[16];
u64 tg_baseseed;
u64 tg_curseed;

BTM_Region	*region;
BTM_Region	*free_region;

BTM_Region	*rgn_guess;
BTM_Region	*rgn_luhash[64];

u32	magic2;

BTM_MobEntity	*free_mobent;

BTM_MobEntity	*nearby_mobent;

BTM_MobEntity	*move_selfent;		//current entity trying to move
BTM_MobEntity	*move_mqlist;		//list of queried ents for move

void	*mm_p2alloc[20];

u32	magic3;

byte	cam_yaw;
sbyte	cam_pitch;
u64		cam_org;
u64		cam_vel;
u64		cam_flags;

float	cam_fw[3];
float	cam_rt[3];
float	cam_up[3];

u64		scr_pts_list[BTM_RAYCAST_MAXHITS];		//results 2 (merged)
u32		magic4;
int		scr_pts_hit[BTM_RAYCAST_MAXHITS];		//raycast hit results
int		scr_pts_chn[BTM_RAYCAST_MAXHITS];		//chain
byte	scr_pts_rcnt[BTM_RAYCAST_MAXHITS];		//recency count
int		scr_pts_hash[BTM_RAYCAST_HASHSZ];		//hash
u32		magic5;
int		scr_npts;

u64		scr_cxpred[BTM_RAYCAST_HASHSZ];			//cix predictor 2
u64		scr_hpred;				//hit prediction

u64		scr_lhit;				//last hit
u64		scr_lahit;				//last air before hit

u64		scr_laspos;				//last air start position
u64		scr_laepos;				//last air end position

int		frame;
u32		sel_blk;
byte	sel_bt;

int		daytimer;
short	day;
byte	daylight;
byte	dodaytimer;

int			drawnear;
int			drawfar;

BTM_TexImg	*texlist[256];

u32		dblk_data[32*24];
u64		dblk_pos[32*24];
int		dblk_dist[32*24];

byte	*lz_tdecbuf;
byte	*lz_tencbuf;
byte	*lz_tenc2buf;
int		lz_szdecbuf;
int		lz_szencbuf;
int		lz_szenc2buf;

u16		tgen_varstk_name[512];
u64		tgen_varstk_val[512];
int		tgen_varstk_pos;
int		tgen_varstk_mark;

u16		tgen_vargbl_name[512];
u64		tgen_vargbl_val[512];
int		tgen_vargbl_cnt;

u32		cam_inven[6*8];


u16			*pgm_gbl_vars;
u64			*pgm_gbl_vals;
s16			pgm_n_gbls;
s16			pgm_m_gbls;

u16			pgm_dyn_vars[256];
u64			pgm_dyn_vals[256];
byte		pgm_dyn_pos;

char		*pgm_print_stdout_ct;

};

struct BTM_Screen_s {
btmra_rastpixel		*rgb;
btmra_zbufpixel		*zb;
int					xs, ys;

sbyte				*fov_ang;			//angle adjustment based on relative X
int					fov_angbi;			//angle bias (center of table)
};

struct BTM_TexImg_s {
btmra_rastpixel		*rgb;		//rgb planes
int					sz_lsh;		//size, left-shift
};

#define		BTM_MAGIC1	0x12345678

struct BTM_Region_s {
u32			magic1;
BTM_Region	*next;
BTM_Region	*unext;
// u32			*vox;
u32			*voxa[512];		//raw voxel values
byte		*voxb[512];		//voxel byte index
byte		*voxh[512];		//voxel half-byte index
u32			magic4;
u32			voxu[512];		//block-type (if n==1)
byte		vox_n[512];		//number of assigned indices
byte		vox_m[512];		//max allocated indices (log2)
u32			chk_fl[512];	//per chunk flags
u32			chk_ofsz[768];	//per chunk offset/size
u32			chk_vseq[512];	//chunk version sequence
u32			magic2;
u16			voxbmix[512];	//voxel bitmap, index
u32			magic3;
u64			*voxbm;			//voxel bitmap, bits

byte		chkhit[512];	//chunk hit by raycast
byte		facehit[192];	//faces hit by raycast
u16			chkraix[512];	//chunk render arrays.

int			rgnix;
u32			rgnvseq;		//region version sequence

byte		*etb_dat[64];
int			etb_sz[64];

byte		*img_dat;
u64			*img_bmp;
int			img_sz;
int			img_ncell;
int			img_lnzcell;

byte		voxbm_n;		//voxel bitmap, num blocks (0 if flat)
byte		dirty;
byte		useflag;		//access / use flags

BCCX_Node		*static_ent_tree;
BTM_MobEntity	*live_entity;

BTM_MobEntity	*live_entity_hash[256];		//hash for block position
u32			magic5;

byte		*img_rosdat;
int			img_rossz;
u32			*ros_cels;
u16			*ros_maps[6];
int			ros_mapofs[6];
u16			ros_ncels;
};

struct BTM_MobEntity_s {
BTM_MobEntity	*next;
BTM_MobEntity	*chain;
BTM_MobEntity	*nxt_bpos;
BTM_MobEntity	*chn_bpos;
BTM_MobEntity	*chain_near;
int				org_x;		//16.8
int				org_y;		//16.8
int				org_z;		//8.8
byte			yaw;		//yaw angle
byte			pitch;		//pitch angle (if relevant)
// int				spr_id;		//sprite
char			*cname;		//entity class name
char			*npcname;	//NPC character name
char			*dfl_looktext;		//object generic look text
char			*looktext;			//object look text
char			*modelname;			//modelname (props)

int				orgl_x;		//16.8
int				orgl_y;		//16.8
int				orgl_z;		//8.8

s16				vel_x;
s16				vel_y;
s16				vel_z;
int				mvflag;

s16				ivel_x;
s16				ivel_y;
s16				ivel_z;

byte			rad_x;
byte			rad_z;
byte			rad_ofs_z;

byte			movetype;
byte			solidtype;

u64				spos;
u64				bpos;

float			spr_dxs;	//sprite width
float			spr_dzs;	//sprite height

char			*spr_base;
int				spr_seq;
int				spr_frame;

int				mob_rtick;
int				mob_mvtick;

u16				mobvar_name[32];
u64				mobvar_val[32];
int				mobvar_cnt;

BTM_World		*wrl;
BTM_Region		*rgn;
int				(*Tick)(BTM_World *wrl, BTM_MobEntity *self);
int				(*Draw)(BTM_World *wrl, BTM_MobEntity *self);
};


struct BTM_MobSprite_s {
BTM_MobSprite	*next;
int				org_x;		//16.8
int				org_y;		//16.8
int				org_z;		//8.8
byte			yaw;		//yaw angle
byte			pitch;		//pitch angle (if relevant)

// int				orgl_x;		//16.8
// int				orgl_y;		//16.8
// int				orgl_z;		//8.8

// byte			rad_x;
// byte			rad_z;
// byte			rad_ofs_z;

u32				spr_rgb;

float			spr_dxs;	//sprite width
float			spr_dzs;	//sprite height

char			*spr_base;
int				spr_seq;
int				spr_frame;
};

typedef struct BTM_ConCmd_s BTM_ConCmd;
typedef struct BTM_ConPgm_s BTM_ConPgm;
typedef struct BTM_PgmExt_s BTM_PgmExt;

struct BTM_ConCmd_s {
BTM_ConCmd	*next;
char		*name;
int			(*Run)(BTM_ConCmd *cmd, char **args);
void		*cvar;
byte		cvty;	//type (storage)
byte		uity;	//UI subtype.
byte		flag;
};

struct BTM_ConPgm_s {
BTM_ConPgm	*next;
char		*name;
// u16			*gbl_vars;
// u64			*gbl_vals;
// s16			n_gbls;

u16			*ltokb;			//line token buffer
u16			*ltoki;			//line token index
u16			*lbl_id;		//label IDs
u16			*lbl_ix;		//label line index
int			n_lines;
int			n_tokens;
s16			n_lbls;

int			cur_line;
int			nxt_line;
u64			retval;

int			gosub_retidx[16];
int			gosub_retdst[16];
byte		gosub_dynidx[16];
byte		gosub_retpos;

int			loop_retidx[16];
u64			loop_endval[16];
u64			loop_stpval[16];
byte		loop_retpos;
};

struct BTM_PgmExt_s {
BTM_PgmExt *next;
u64 (*Run)(BTM_World *wrl, BTM_ConPgm *pgm, u16 **rtcs);
u64 (*Infix)(BTM_World *wrl, BTM_ConPgm *pgm, u16 tok, u64 vala, u64 valb);
u16 token;	//keyword token
u16 level;	//level this applies to
};


typedef struct BTM_BtModel_s BTM_BtModel;
typedef struct BTM_BtModelMesh_s BTM_BtModelMesh;
typedef struct BTM_BtModelBone_s BTM_BtModelBone;
typedef struct BTM_BtModelAnim_s BTM_BtModelAnim;

struct BTM_BtModel_s {
BTM_BtModel *next;
char *name;
BTM_BtModelMesh *mesh[256];
BTM_BtModelBone *bone[256];
BTM_BtModelAnim *anim[256];
float lod_dist[64];
byte lod_base[64];
byte lod_cnt[64];
int n_mesh;
int n_lod;
int n_bone;
int n_anim;

byte *slab[256];
int nslab, slabpos;
};

struct BTM_BtModelMesh_s {
float *v_xy;
float *v_st;
float *v_nv;
byte *v_bn;
u32 *v_cl;
u16 *tris;
float *tri_nv;
char *matname;
u32 baseclr;
int n_vtx;
int n_tri;

float bbox[6];
};

struct BTM_BtModelBone_s {
float baserot[4];
float baseorg[4];
char *name;
byte id_parent;
byte id_solid;
};

struct BTM_BtModelAnim_s {
BTM_BtModelAnim *next;
char *name;
u32 *frm_rot;
u32 *frm_org;
u32 *frm_scl;
byte n_frames;
byte n_bones;
byte framerate;
};

#define		btm_malloc(sz)	btm_malloc_lln(sz, __FILE__, __LINE__)
#define		btm_realloc(ptr, sz)	btm_realloc_lln(ptr, sz, __FILE__, __LINE__)

#define		btm_malloc_va(sz)	\
	btm_malloc_va_lln(sz, __FILE__, __LINE__)
#define		btm_realloc_va(ptr, sz)	\
	btm_realloc_va_lln(ptr, sz, __FILE__, __LINE__)

#define		BTM_WorldAllocSq(wrl,pidx)	\
	BTM_WorldAllocSqLln(wrl,pidx,__FILE__, __LINE__)


#define BTMGL_FOURCC(a, b, c, d)	((a)|((b)<<8)|((c)<<16)|((d)<<24))

typedef struct  {
	u32 dwSize;
	u32 dwFlags;
	u32 dwFourCC;
	u32 dwRGBBitCount;
	u32 dwRBitMask;
	u32 dwGBitMask;
	u32 dwBBitMask;
	u32 dwABitMask;
} BTMGL_DDS_PIXFMT;

typedef struct {
	u32           dwSize;
	u32           dwFlags;
	u32           dwHeight;
	u32           dwWidth;
	u32           dwPitchOrLinearSize;
	u32           dwDepth;
	u32           dwMipMapCount;
	u32           dwReserved1[11];
	BTMGL_DDS_PIXFMT ddspf;
	u32           dwCaps;
	u32           dwCaps2;
	u32           dwCaps3;
	u32           dwCaps4;
	u32           dwReserved2;
} BTMGL_DDS_HEADER;

typedef struct {
	u32 dwMagic;
	BTMGL_DDS_HEADER head;
} BTMGL_DDS_FILEHEADER;

#include "btm_tkmod.h"

char *BTM_CvarGetStr(char *name);
int BTM_CvarSetStr(char *name, char *val);

char *BTM_PgmGetGlobalNameStr(BTM_World *wrl, char *name);
u32 BTM_GetWorldBlockCix(BTM_World *wrl, u64 rcix);

void *btm_malloc_lln(int sz, char *lfn, int lln);
void *btm_realloc_lln(void *ptr, int sz, char *lfn, int lln);
void btm_free(void *ptr);

void *btm_malloc_va_lln(int sz, char *lfn, int lln);
void *btm_realloc_va_lln(void *ptr, int sz, char *lfn, int lln);

BTM_BtModel *BTM_BmdLoadModelBuffer(byte *buf, int sz);

BTM_MobEntity *BTM_QueryWorldEntitiesForBox(
	BTM_World *wrl, BTM_MobEntity *slst,
	int mcx, int mcy, int mcz, int ncx, int ncy, int ncz,
	BTM_MobEntity *ignore);

#endif
