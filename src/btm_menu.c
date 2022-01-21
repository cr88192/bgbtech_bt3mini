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

typedef struct BTM_Menu_s BTM_Menu;

struct BTM_Menu_s {
BTM_Menu *next;
char *name;
BCCX_Node *root;
BCCX_Node *cur_node;
};

BTM_Menu *btm_menuroot;
BTM_Menu *btm_showmenu;
int btm_menu_selitem;
int btm_menu_selchars;
int btm_menu_selchardt;

int			btm_texfont_rov=0;
int			btm_texfont_8px;
int			btm_texfont_16px;

int			btm_texfont_curi;

int			btm_menu_inhibit;

int BTM_MenuInit()
{
	byte *buf;
	int sz;
	
	if(btm_texfont_rov)
		return(0);
	
	btm_texfont_rov=16;
	
	buf=BTM_LoadFile("gfx/fixed_8px.dds", &sz);
	if(buf)
	{
		btm_texfont_8px=btm_texfont_rov++;
		tkra_glBindTexture(TKRA_TEXTURE_2D, btm_texfont_8px);
		BTMGL_UploadCompressed(buf, 0, 1);
	}

	buf=BTM_LoadFile("gfx/fixed_16px.dds", &sz);
	if(buf)
	{
		btm_texfont_16px=btm_texfont_rov++;
		tkra_glBindTexture(TKRA_TEXTURE_2D, btm_texfont_16px);
		BTMGL_UploadCompressed(buf, 0, 1);
	}
	
	btm_texfont_curi=0;
	return(0);
}

int BTM_DrawCharPrim(int x, int y, int xs, int ys,
	int ch, int tex, u32 clr)
{
	float x0, x1, y0, y1;
	float s0, s1, t0, t1;
	int i, j;
	
	if((tex!=btm_texfont_curi) || (ch<=0) || (tex<=0))
	{
		if(btm_texfont_curi)
		{
			tkra_glEnd();
			btm_texfont_curi=0;
		}
		
		if((ch<=0) || (tex<=0))
		{
			return(0);
		}
	}

	x0=x;	x1=x+xs;
	y0=y;	y1=y+ys;
	i=(ch>>4)&15;
	j=(ch   )&15;
	s0=j*(1.0/16.0); s1=s0+(1.0/16.0);
//	t0=i*(1.0/16.0); t1=t0+(1.0/16.0);
//	t1=i*(1.0/16.0)+(2/256.0); t0=t1+(1.0/16.0);
	t1=i*(1.0/16.0)+(2/256.0);
	t0=t1+(1.0/16.0)+(3/256.0);

	tkra_glColor4ubv((void *)(&clr));

	if(btm_texfont_curi<=0)
	{
		tkra_glBindTexture(TKRA_TEXTURE_2D, tex);
		btm_texfont_curi=tex;

		tkra_glBegin(GL_QUADS);
	}

//	tkra_glBegin(GL_QUADS);
	tkra_glTexCoord2f(s0, t0);
	tkra_glVertex2f(x0, y0);
	tkra_glTexCoord2f(s0, t1);
	tkra_glVertex2f(x0, y1);
	tkra_glTexCoord2f(s1, t1);
	tkra_glVertex2f(x1, y1);
	tkra_glTexCoord2f(s1, t0);
	tkra_glVertex2f(x1, y0);
//	tkra_glEnd();

	return(0);
}

int BTM_DrawStringPrim(int x, int y, int xs, int ys,
	char *str, int tex, u32 clr)
{
	char *cs;
	int cx, cy, ch;
	
	cs=str; cx=x; cy=y;
	while(*cs)
	{
		ch=*cs++;
		if(ch=='\n')
		{
			cx=x;
			cy-=ys;
			continue;
		}
	
		BTM_DrawCharPrim(cx, cy, xs, ys, ch, tex, clr);
		cx+=xs;
	}
	return(0);
}

int BTM_DrawString8px(int x, int y, char *str, u32 clr)
{
	if(!str)
	{
		BTM_DrawCharPrim(0, 0, 0, 0, 0, 0, 0);
		return(0);
	}

	return(BTM_DrawStringPrim(x, y, 8, 8, str, btm_texfont_8px, clr));
}

int BTM_DrawString16px(int x, int y, char *str, u32 clr)
{
	if(!str)
	{
		BTM_DrawCharPrim(0, 0, 0, 0, 0, 0, 0);
		return(0);
	}

	return(BTM_DrawStringPrim(x, y, 16, 16, str, btm_texfont_16px, clr));
}

int BTM_DrawString8pxLen(int x, int y, char *str, u32 clr, int len)
{
	char tb[1024];
	
	if(len<=0)
		return(0);
	
	if(len>=strlen(str))
	{
		return(BTM_DrawStringPrim(x, y, 8, 8, str, btm_texfont_8px, clr));
	}
	
	strcpy(tb, str);
	tb[len]=0;
	return(BTM_DrawStringPrim(x, y, 8, 8, tb, btm_texfont_8px, clr));
}


int BTM_LoadMenu(char *fname)
{
	BTM_Menu *menu;
	BCCX_Node *root, *n1;
	BCCX_Node *c;
	char *buf, *s1;
	int sz, sz1;
	int na, ci;
	
	BTM_MenuInit();

//	buf=BTM_LoadFile(fname, &sz);
	buf=BTM_LoadFileTmp(fname, &sz);
	if(!buf)
		return(-1);
	
	root=BCCX_ParseExprStr(buf);
	BCCX_MarkTreeZone(root, BCCX_ZTY_GLOBAL);

	if(BCCX_TagIsP(root, "loadfiles"))
	{
		na=BCCX_GetNodeChildCount(root);
		for(ci=0; ci<na; ci++)
		{
			c=BCCX_GetNodeIndex(root, ci);
//			BTM_InstanceStructureNodeAt(wrl, bcx, bcy, bcz, c);

			if(BCCX_TagIsP(c, "loadmenu"))
			{
				s1=BCCX_Get(c, "path");
				if(s1)
				{
					BTM_LoadMenu(s1);
				}
			}

		}
		return(0);
	}
	
	menu=malloc(sizeof(BTM_Menu));
	memset(menu, 0, sizeof(BTM_Menu));
	
	menu->root=root;
	menu->name=BCCX_Get(root, "name");
	
	menu->next=btm_menuroot;
	btm_menuroot=menu;

#if 1
	sz1=BCCX_AbxeEncodeNodeBuffer(root, buf, sz);
	printf("ABXE %d->%d\n", sz, sz1);
	n1=BCCX_AbxeParseBuffer(buf, sz1);
	BCCX_Print(n1);
#endif

	return(0);
}

int BTM_ShowMenu(char *name, char *subname)
{
	BTM_Menu *mcur;
	BCCX_Node *ncur;
	
	BTM_MenuInit();

	if(!name || !(*name))
	{
		btm_showmenu=NULL;
		return(0);
	}
	
	mcur=btm_menuroot;
	while(mcur)
	{
		if(mcur->name && !strcmp(mcur->name, name))
			break;
		mcur=mcur->next;
	}
	
	if(!mcur)
	{
		btm_showmenu=NULL;
		return(0);
	}

	btm_showmenu=mcur;
	btm_menu_inhibit=4;
	
	if(subname)
	{
		ncur=BCCX_FindAttr(mcur->root, "name", subname);
		mcur->cur_node=ncur;
		
		if(!mcur->cur_node)
		{
			btm_showmenu=NULL;
			return(-1);
		}
	}else
	{
		subname=BCCX_Get(mcur->root, "root");

		ncur=BCCX_FindAttr(mcur->root, "name", subname);
		mcur->cur_node=ncur;
		
		if(!mcur->cur_node)
		{
			btm_showmenu=NULL;
			return(-1);
		}
	}
	
	return(0);
}

BCCX_Node *BTM_LookupMenuNode(char *name, char *subname)
{
	BTM_Menu *mcur;
	BCCX_Node *ncur;
	
	BTM_MenuInit();

	if(!name)
	{
		return(NULL);
	}
	
	mcur=btm_menuroot;
	while(mcur)
	{
		if(mcur->name && !strcmp(mcur->name, name))
			break;
		mcur=mcur->next;
	}
	
	if(!mcur)
	{
		return(NULL);
	}

	if(subname)
	{
		ncur=BCCX_FindAttr(mcur->root, "name", subname);
		return(ncur);
	}else
	{
		subname=BCCX_Get(mcur->root, "root");
		ncur=BCCX_FindAttr(mcur->root, "name", subname);
		return(ncur);
	}
	
	return(0);
}

char *BTM_CvarGetStrUI(char *name);

int BTM_DrawMenu()
{
	BCCX_Node *mcur, *c, *nface;
	char *s0, *s1, *s2, *s3;
	char *scvn, *scvv;
	float f0, f1, f2, f3;
	int na, ci, nopt, tex, fibase;
	int i, j, k, l;

	BTM_MenuInit();

	if(!btm_showmenu)
		return(0);

	mcur=btm_showmenu->cur_node;
	if(btm_menu_inhibit>0)
		btm_menu_inhibit--;

	if(BCCX_TagIsP(mcur, "menu"))
	{
		s0=BCCX_Get(mcur, "title");
		
		if(s0)
		{
			l=strlen(s0);
			BTM_DrawString8px(-l*4, 72, s0, 0xFFFFFFFFU);
		}
		
		i=2; nopt=0;
		na=BCCX_GetNodeChildCount(mcur);
		for(ci=0; ci<na; ci++)
		{
			c=BCCX_GetNodeIndex(mcur, ci);

			if(BCCX_TagIsP(c, "option"))
			{
				scvv=NULL;
				scvn=BCCX_Get(c, "cvar");
				if(scvn)
					scvv=BTM_CvarGetStrUI(scvn);
			
				s1=BCCX_Get(c, "text");
				if(s1)
				{
					l=strlen(s1);
					if(scvv)
						l+=strlen(scvv)+1;
					BTM_DrawString8px(-l*4, 72-i*8, s1, 0xFFFFFFFFU);

					if(scvv)
					{
						BTM_DrawString8px(
							(-l)*4+(strlen(s1)+1)*8, 72-i*8,
							scvv, 0xFFFFFFFFU);
					}
				}
				
				if(btm_menu_selitem==(i-2))
				{
					BTM_DrawString8px(-(l+4)*4, 72-i*8,
						"*", 0xFFFFFFFFU);

					if(	I_KeyDown(K_LEFTARROW) &&
						!I_KeyDownL(K_LEFTARROW))
					{
						if(scvn)
						{
							BTM_CvarNudge(scvn, K_LEFTARROW);
						}
					}

					if(	I_KeyDown(K_RIGHTARROW) &&
						!I_KeyDownL(K_RIGHTARROW))
					{
						if(scvn)
						{
							BTM_CvarNudge(scvn, K_RIGHTARROW);
						}
					}

					if(I_KeyDown(K_ENTER) &&
						!I_KeyDownL(K_ENTER) &&
						!btm_menu_inhibit)
					{
//						if(scvn && scvv)
						if(scvn)
						{
							BTM_CvarNudge(scvn, K_ENTER);
//							if(!strcmp(scvv, "true"))
//								{ BTM_CvarSetStr(scvn, "false"); }
//							if(!strcmp(scvv, "false"))
//								{ BTM_CvarSetStr(scvn, "true"); }
						}
					
						s1=BCCX_Get(c, "goname");
						s2=BCCX_Get(c, "gosubname");

						if(s1)
						{
							BTM_ShowMenu(s1, s2);
							btm_menu_selitem=0;
							btm_menu_selchars=0;
						}else if(s2)
						{
							BTM_ShowMenu(btm_showmenu->name, s2);
							btm_menu_selitem=0;
							btm_menu_selchars=0;
						}
					}
				}
				
				i++;
				nopt++;
			}
		}

		BTM_DrawCharPrim(0, 0, 0, 0, 0, 0, 0);

		if(I_KeyDown(K_DOWNARROW) && !I_KeyDownL(K_DOWNARROW))
		{
			btm_menu_selitem++;
		}

		if(I_KeyDown(K_UPARROW) && !I_KeyDownL(K_UPARROW))
		{
			btm_menu_selitem--;
		}

		if(btm_menu_selitem>=nopt)
			btm_menu_selitem-=nopt;
		if(btm_menu_selitem<0)
			btm_menu_selitem+=nopt;

		return(0);
	}




	if(BCCX_TagIsP(mcur, "convo"))
	{
		s0=NULL;

		BTM_DrawCharPrim(0, 0, 0, 0, 0, 0, 0);

		f0=(9.0/16.0)+(1.0/256.0);
		f1=(10.0/16.0)-(1.0/256.0);
		f2=(3.0/16.0)+(1.0/256.0);
		f3=(4.0/16.0)-(1.0/256.0);

		tkra_glColor4f(1.0, 1.0, 1.0, 1.0);
		tkra_glBindTexture(GL_TEXTURE_2D, 2);
		tkra_glBegin(GL_QUADS);
		tkra_glTexCoord2f(f0, f2);
		tkra_glVertex2f(-140, 88);
		tkra_glTexCoord2f(f0, f3);
		tkra_glVertex2f(-140, -88);
		tkra_glTexCoord2f(f1, f3);
		tkra_glVertex2f(140, -88);
		tkra_glTexCoord2f(f1, f2);
		tkra_glVertex2f(140, 88);

		tkra_glEnd();

		c=BCCX_Fetch(mcur, "text");
		if(c)
		{
			s0=BCCX_Text(c);
		}
	
		if(!s0)
			s0=BCCX_Get(mcur, "text");

		nface=NULL;

		s1=BCCX_Get(mcur, "face");
		if(s1)
			nface=BCCX_FindAttr(btm_showmenu->root, "name", s1);
		
		fibase=BCCX_GetInt(mcur, "face_idx");
		
		if(nface)
		{
			s2=BCCX_Get(nface, "image");
			tex=BTMGL_LoadSpriteForName(s2, 0, 0);

			i=(btm_menu_selchardt>50) && (btm_menu_selchars<strlen(s0));
			i+=fibase;
			j=(i   )&3;
			k=(i>>2)&3;

			f0=(j+0)*(1.0/4.0)+(1.0/256.0);
			f1=(j+1)*(1.0/4.0)-(1.0/256.0);
			f2=(k+0)*(1.0/4.0)+(1.0/256.0);
			f3=(k+1)*(1.0/4.0)-(1.0/256.0);

			tkra_glColor4f(1.0, 1.0, 1.0, 1.0);
			tkra_glBindTexture(GL_TEXTURE_2D, tex);
			tkra_glBegin(GL_QUADS);
			tkra_glTexCoord2f(f0, f2);
			tkra_glVertex2f(-132, 80);
			tkra_glTexCoord2f(f0, f3);
			tkra_glVertex2f(-132, 48);
			tkra_glTexCoord2f(f1, f3);
			tkra_glVertex2f(-100, 48);
			tkra_glTexCoord2f(f1, f2);
			tkra_glVertex2f(-100, 80);

			tkra_glEnd();
		}
		
		if(s0)
		{
//			l=strlen(s0);
			l=20;
			BTM_DrawString8pxLen(-l*4, 72, s0, 0xFFFFFFFFU,
				btm_menu_selchars);
		}
		
		if(btm_menu_selchardt>100)
		{
			btm_menu_selchars++;
			btm_menu_selchardt-=100;
		}
		
		i=12; nopt=0;
		na=BCCX_GetNodeChildCount(mcur);
		for(ci=0; ci<na; ci++)
		{
			c=BCCX_GetNodeIndex(mcur, ci);

			if(BCCX_TagIsP(c, "option"))
			{
				s1=BCCX_Get(c, "text");
				if(s1)
				{
					l=strlen(s1);
					BTM_DrawString8px(-l*4, 72-i*8, s1, 0xFFFFFFFFU);
				}
				
				if(btm_menu_selitem==(i-12))
				{
					BTM_DrawString8px(-(l+4)*4, 72-i*8,
						"*", 0xFFFFFFFFU);

					if(I_KeyDown(K_ENTER) &&
						!I_KeyDownL(K_ENTER) &&
						!btm_menu_inhibit)
					{
						s1=BCCX_Get(c, "goname");
						s2=BCCX_Get(c, "gosubname");

						if(s1)
						{
							BTM_ShowMenu(s1, s2);
							btm_menu_selitem=0;
							btm_menu_selchars=0;
						}else if(s2)
						{
							BTM_ShowMenu(btm_showmenu->name, s2);
							btm_menu_selitem=0;
							btm_menu_selchars=0;
						}
					}
				}
				
				i++;
				nopt++;
			}
		}

		BTM_DrawCharPrim(0, 0, 0, 0, 0, 0, 0);

		if(I_KeyDown(K_DOWNARROW) && !I_KeyDownL(K_DOWNARROW))
		{
			btm_menu_selitem++;
		}

		if(I_KeyDown(K_UPARROW) && !I_KeyDownL(K_UPARROW))
		{
			btm_menu_selitem--;
		}

		if(btm_menu_selitem>=nopt)
			btm_menu_selitem-=nopt;
		if(btm_menu_selitem<0)
			btm_menu_selitem+=nopt;

		return(0);
	}


	BTM_DrawCharPrim(0, 0, 0, 0, 0, 0, 0);
	return(0);
}

int BTM_ToggleMenu()
{
	if(btm_showmenu)
	{
		btm_showmenu=NULL;
		return(0);
	}

	if(!btm_showmenu)
	{
		BTM_ShowMenu("main", 0);
		return(0);
	}

	return(0);
}

int BTM_MenuDownP()
{
	if(btm_showmenu)
		return(1);
	return(0);
}

int BTM_MenuDoInput(int dt)
{
	if(!BTM_MenuDownP())
		return(0);
	btm_menu_selchardt+=dt;
	return(0);
}
