#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>


#include "tkrast/tkr_multi.c"

#include "btm_engine.h"

#ifdef _WIN32
#include "gfxdrv_w32.c"
#include "gfxdrv_input.c"
#include "sound_w32.c"
#endif

#include "gfxdrv_keys.h"

#define TGVLZ_NOMAIN
#include "tgvlz1.c"

#include "bt1h_targa.c"

#include "cca_node.c"
#include "cca_print.c"
#include "cca_parse.c"
#include "cca_abxe.c"

#include "btm_voxtype.c"

// #include "btra_span.c"
// #include "btra_col.c"
// #include "btra_wall.c"

// #include "rat_loadply.c"
// #include "tkrast/tkr_multi.c"

#include "btra_ray2.c"

#include "btm_world.c"
#include "btm_phys.c"
#include "btm_region.c"
#include "btm_tgen.c"
#include "btm_light.c"
#include "btm_mob.c"

#include "btm_wad4.c"

#include "btm_menu.c"
#include "btm_console.c"

#include "btm_drawtext.c"

#include "btm_main.c"
