#define BTM_STATUS_CONVEX	1	//mesh is convex
#define BTM_STATUS_CONCAVE	2	//mesh is concave

typedef struct {
float x;
float y;
}btm_vec2f;

typedef struct {
float x;
float y;
float z;
}btm_vec3f;

typedef struct {
float x;
float y;
float z;
float w;
}btm_vec4f;

typedef struct BTM_SolidMesh_s BTM_SolidMesh;
typedef struct BTM_SolidSkel_s BTM_SolidSkel;

typedef struct BTM_CsgBrush_s BTM_CsgBrush;
typedef struct BTM_CsgNode_s BTM_CsgNode;
typedef struct BTM_CsgPoly_s BTM_CsgPoly;

struct BTM_SolidMesh_s {
BTM_SolidMesh *next;
char *fname;		//filename
char *name;			//name of mesh
float *tris;		//triangles
float *norm;		//triangles
int nTris;			//number of triangles
int mTris;			//max number of triangles
int status;			//status flags

float bbox[6];
float scale[3];

float baseorg[4];
float baseclr[4];
int lastcalc;
int rcrov;
int texnum;

u64 clrmat;

// float texscale_s;
// float texscale_t;
char *usetex[8];
float texvec_n[32];
float texvec_s[32];
float texvec_t[32];
int n_texvec;

float lcamorg[4];

// textured mesh
float *v_xyz;		//vertex XYZ (VA)
u32 *v_cl;			//vertex RGBA (VA)
byte *v_bn;			//vertex bone (VA)
float *v_st;		//vertex ST (VA)
float *v_nv;		//vertex ST (VA)
int *t_vidx;		//triangle vertex indices
int n_vtx;
int n_tris;

BTM_SolidSkel *skel;

};

struct BTM_SolidSkel_s {
int bone_parent[256];
char *bone_name[256];
float bone_baserot[256*4];
float bone_baseorg[256*3];
float bone_relorg[256*3];
int n_bones;

BTM_BtModelAnim *anim;
};

struct BTM_CsgBrush_s {
BTM_CsgBrush *next;
float *planes;
int n_planes;
};

struct BTM_CsgNode_s {
BTM_CsgNode *ltree;
BTM_CsgNode *rtree;
BTM_CsgBrush *brush;
u64 clrmat;
float trans[16];
int opr;
};

struct BTM_CsgPoly_s {
BTM_CsgPoly *next;
u64 clrmat;
float bbox[6];
float *pts;
int npts;
};

void QuatF_FromAngles(float *a, float *b);
void QuatF_FromAnglesB(float *a, float *b);
void QuatF_Identity(float *a);
float QuatF_Normalize(float *a, float *b);
void QuatF_From3Matrix(float *a, float *b);
