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

/*
Inventory System

(31:24): Reserved
(23:16): Item Count (0..99)
(15:12): Item Attribute
(11: 0): Item Type
 */

byte	btm_inven_open;
byte btm_condown;
byte btm_conchat;

int			btm_texfont_tinycnt;
int			btm_texfont_inven;

struct {
char *name;
int id;
} btm_inven_names[]={
{"cube",			BTM_INVEN_CUBE},
{"coin",			BTM_INVEN_COIN},
{"cylinder",		BTM_INVEN_CYLINDER},
{"milk",			BTM_INVEN_MILK},
{"cookie",			BTM_INVEN_COOKIE},
{"scroll",			BTM_INVEN_SCROLL},
{"peanutbutter",	BTM_INVEN_PEANUTBUTTER},
{"pb",				BTM_INVEN_PEANUTBUTTER},
{"junk",			BTM_INVEN_JUNK},
{"pb_junk",			BTM_INVEN_PBJUNK},
{NULL, 0}};

char		btm_menu_looktext[1024];

int BTM_InvenOpenP()
{
	return(btm_inven_open);
}

int BTM_InvenDoInput(int dt)
{
	return(0);
}

int BTM_InvenIdForName(char *name)
{
	int i;
	
	for(i=0; btm_inven_names[i].name; i++)
		if(!strcmp(btm_inven_names[i].name, name))
			return(btm_inven_names[i].id);
	return(-1);
}

char *BTM_InvenNameForId(int id)
{
	int i;
	for(i=0; btm_inven_names[i].name; i++)
		if(btm_inven_names[i].id==id)
			return(btm_inven_names[i].name);
	return(NULL);
}

int BTM_LookupInvenSlotForItem(BTM_World *wrl, int id)
{
	u32 b;
	int x, y, z;
	
	for(y=0; y<5; y++)
		for(x=0; x<8; x++)
	{
		z=y*8+x;
		b=wrl->cam_inven[z];
//		if((b&4095)==(id&4095))
		if((b&65535)==(id&65535))
			return(z);
	}
	return(-1);
}

int BTM_LookupInvenSlotForItemtype(BTM_World *wrl, int id)
{
	u32 b;
	int x, y, z;
	
	for(y=0; y<5; y++)
		for(x=0; x<8; x++)
	{
		z=y*8+x;
		b=wrl->cam_inven[z];
		if((b&4095)==(id&4095))
//		if((b&65535)==(id&65535))
			return(z);
	}
	return(-1);
}

int BTM_CheckInvenCountForItem(BTM_World *wrl, int id)
{
	u32 b;
	int z;
	
	if(!(id&4095))
		return(0);

	z=BTM_LookupInvenSlotForItem(wrl, id);
	if(z<0)
		return(0);

	b=wrl->cam_inven[z];
	return((b>>16)&255);
}

int BTM_AddInvenCountForItem(BTM_World *wrl, int id, int cnt)
{
	u32 b;
	int z, cn1;
	
//	if(!(id&4095))
	if(!(id&65535))
		return(0);

	z=BTM_LookupInvenSlotForItem(wrl, id);
	if(z>=0)
	{
		b=wrl->cam_inven[z];
		cn1=((b>>16)&255)+cnt;
		if(cn1>99)
			cn1=99;
		if(cn1<=0)
		{
			wrl->cam_inven[z]=0;
			return(0);
		}
		b=b&~0x00FF0000U;
		b|=cn1<<16;
		wrl->cam_inven[z]=b;
		return(0);
	}

	z=BTM_LookupInvenSlotForItem(wrl, 0);
	if(z>=0)
	{
		b=(id&0xFFFF)|(cnt<<16);
		wrl->cam_inven[z]=b;
		return(0);
	}
	return(0);
}

int BTM_InvenUseBasic(BTM_World *wrl, int id)
{
	char *s0;
	int z;

	if(id==BTM_INVEN_JUNK)
	{
		strcpy(btm_menu_looktext,
			"You grab hold\n"
			"  of your junk.\n"
			"Nothing happens...\n");
		BTM_ShowMenu("main", "do_look");
		return(0);
	}

	z=BTM_LookupInvenSlotForItem(wrl, id);
	if(z<0)
	{
		s0=BTM_InvenNameForId(id);
		if(!s0)
			return(0);
		sprintf(btm_menu_looktext,
			"You don't have\n  %s.\n", s0);		
		BTM_ShowMenu("main", "do_look");
		return(0);
	}

	return(0);
}

int BTM_InvenUseWith(BTM_World *wrl, int id, int id2)
{
	char *s0;
	int z;

	z=BTM_LookupInvenSlotForItem(wrl, id);
	if(z<0)
	{
		s0=BTM_InvenNameForId(id);
		if(!s0)
			return(0);
		sprintf(btm_menu_looktext,
			"You don't have\n  %s.\n", s0);		
		BTM_ShowMenu("main", "do_look");
		return(0);
	}

	if((id==BTM_INVEN_PEANUTBUTTER) && (id2==BTM_INVEN_JUNK))
	{
		BTM_AddInvenCountForItem(wrl, BTM_INVEN_JUNK, -1);
		BTM_AddInvenCountForItem(wrl, BTM_INVEN_PBJUNK, 1);
		strcpy(btm_menu_looktext,
			"You rub peanut butter\n"
			"  on your junk.\n"
			"Nice and creamy,\n"
			"  goes on smooth.\n"
			"Now you feel ready...\n"
			"  For anything...\n");
		BTM_ShowMenu("main", "do_look");
		return(0);
	}

	return(0);
}

int BTM_InvenCheckHasName(BTM_World *wrl, char *name)
{
	int id, z;

	id=BTM_InvenIdForName(name);
	if(id<=0)
		return(0);
	z=BTM_LookupInvenSlotForItem(wrl, id);
	return(z>=0);
}

int BTM_InvenAddHasName(BTM_World *wrl, char *name, int cnt)
{
	int id, z;

	id=BTM_InvenIdForName(name);
	if(id<=0)
		return(0);
	BTM_AddInvenCountForItem(wrl, id, cnt);
	return(0);
}

int BTM_DrawInventoryCell(BTM_World *wrl, int cx, int cy, u32 cell)
{
	int cnt, cbcd;

	if(!(cell&0xFF))
		return(0);

	cnt=(cell>>16)&0xFF;
	cbcd=(cnt%10)+((cnt/10)<<4);

	BTM_DrawCharPrim(
		cx, cy, 32, 32, cell&0xFF,
		btm_texfont_inven, 0xFFFFFFFFU);
	if(cnt>1)
	{
		BTM_DrawCharPrim(
			cx, cy, 8, 8, cbcd,
			btm_texfont_tinycnt, 0xFF000000U);
	}
	return(0);
}

int BTM_DrawInventoryGrid(BTM_World *wrl)
{
	int x, y;
	
	for(y=0; y<5; y++)
		for(x=0; x<8; x++)
	{
		BTM_DrawCharPrim(
			-128+(x*32), -80+(y*32),
			32, 32, 0xFE,
			btm_texfont_inven, 0xFFFFFFFFU);

		BTM_DrawInventoryCell(wrl,
			-128+(x*32), -80+(y*32),
			wrl->cam_inven[y*8+x]);
	}
	return(0);
}

int BTM_DrawInventory(BTM_World *wrl)
{
	if(!btm_inven_open)
		return(0);

	BTM_DrawInventoryGrid(wrl);
	return(0);
}

int BTM_InventoryHandleKey(BTM_World *wrl, u16 key)
{
//	if(BTM_ConDownP() || BTM_MenuDownP())
	if(btm_condown || BTM_MenuDownP())
		return(0);
	
	if(key=='\t')
	{
		btm_inven_open=!btm_inven_open;
		btm_conchat=0;
		return(0);
	}
	
	if(btm_inven_open)
	{
		btm_conchat=1;
	}

	if(!btm_inven_open)
		return(0);

	return(0);	
}
