TkMod_MixSample *tkm_samples;

TkMod_MixSample *tkm_chan_samp[16];
int tkm_chan_pos[16];
byte tkm_chan_vol_lf[16];
byte tkm_chan_vol_rt[16];

TkMod_Info *tkm_snd_song;

byte tkm_vol_sfx=192;
byte tkm_vol_music=192;

short tkm_snd_mixbuf[16384];

u64 tkm_snd_vpos;

#define FCC_RIFF	BTMGL_FOURCC('R', 'I', 'F', 'F')
#define FCC_WAVE	BTMGL_FOURCC('W', 'A', 'V', 'E')
#define FCC_fmt		BTMGL_FOURCC('f', 'm', 't', ' ')
#define FCC_data	BTMGL_FOURCC('d', 'a', 't', 'a')

byte *TkMod_DecodeWavBuf(byte *ibuf, int ibsz, int *rlen)
{
	byte *cs, *cse, *cs1;
	byte *data, *ddat;
	u32 fcc0, sz0, fcc1, dsz;
	int chan, rate, bits, len, len1, ssz, ilen;
	int spos, sstep;
	int i, j, k;
	
	fcc0=get_u32le(ibuf+0);
	sz0=get_u32le(ibuf+4);
	fcc1=get_u32le(ibuf+8);
	
	if(	(fcc0!=FCC_RIFF) ||
		(fcc1!=FCC_WAVE))
			return(NULL);
	cs=ibuf+12;
	cse=cs+sz0;
	
	data=NULL;
	chan=1;
	rate=16000;
	bits=8;
	while(cs<cse)
	{
		fcc0=get_u32le(cs+0);
		sz0=get_u32le(cs+4);
		cs1=cs+8;
		if(fcc0==FCC_fmt)
		{
			chan=cs1[2];
			rate=cs1[4]+(cs1[5]<<8);
			bits=cs1[14];
		}

		if(fcc0==FCC_data)
		{
			data=cs1;
			dsz=sz0;
		}
		
		cs+=8+((sz0+1)&(~1));
	}
	
	ssz=(chan*bits)/8;
	ilen=dsz/ssz;
	
	len=(ilen*16000)/rate;

	sstep=(256*rate)/16000;
	spos=0;
	
	len1=(len+511)&(~255);
	
	ddat=btm_malloc(len1);
	memset(ddat, 128, len1);
	
	for(i=0; i<len; i++)
	{
		j=(spos>>8);
		spos+=sstep;
		if(bits==16)
		{
			k=(s16)(get_u16le(data+(j*ssz)));
			k=(k>>8)+128;
		}else
		{
			k=data[j*ssz];
		}
		ddat[i]=k;
	}
	
	*rlen=len;
	return(ddat);
}

TkMod_MixSample *TkMod_GetSample(char *name)
{
	TkMod_MixSample *cur;
	byte *buf;
	int sz, len;

	cur=tkm_samples;
	while(cur)
	{
		if(!strcmp(cur->name, name))
			return(cur);
		cur=cur->next;
	}

	cur=btm_malloc(sizeof(TkMod_MixSample));

	cur->name=bccx_strdup(name);
	cur->next=tkm_samples;
	tkm_samples=cur;

	cur->data=NULL;
	buf=BTM_LoadFileTmp(name, &sz);
	if(buf)
	{
		cur->data=TkMod_DecodeWavBuf(buf, sz, &len);
		cur->len=len;
	}

	if(!cur->data)
	{
		cur->data=btm_malloc(512);
		memset(cur->data, 128, 512);
		cur->len=1;
	}

	return(cur);
}

int BTM_FindFreeMixChan()
{
	int i;

	for(i=0; i<16; i++)
		if(!tkm_chan_samp[i])
			return(i);
	return(-1);
}

int BTM_PlaySample(char *name, int vol)
{
	TkMod_MixSample *samp;
	int chan;
	
	samp=TkMod_GetSample(name);
	chan=BTM_FindFreeMixChan();
	
	if(chan<0)
		return(-1);
	
	tkm_chan_samp[chan]=samp;
	tkm_chan_pos[chan]=0;
	tkm_chan_vol_lf[chan]=vol;
	tkm_chan_vol_rt[chan]=vol;
	
	return(0);
}

int BTM_SoundSetVpos(u64 vpos, byte yaw, byte pitch)
{
	tkm_snd_vpos=vpos;
}

int BTM_PlaySample3D(char *name, u64 spos, int vol)
{
	int da, attn, i;

	da=BTM_CalcRayDistApprox(spos, tkm_snd_vpos);
	if(da<256)
		da=256;
	attn=65536/da;
	attn=(attn*vol)>>8;

	i=BTM_PlaySample(name, attn);
	return(i);
}

int BTM_PlaySong(char *name)
{
	TkMod_Info *song;
	song=TkMod_GetMod(name);
	tkm_snd_song=song;
	
	return(0);
}

int BTM_DoMixSamples(int dt)
{
	static int accdt;
	TkMod_Info *song;
	int i, j, k, a_lf, a_rt;
	
	accdt+=dt;
	while(accdt>=16)
	{
		song=tkm_snd_song;
		if(song)
		{
			if((song->mixpos_cs-song->mixbufs)>=8000)
			{
				k=song->mixpos-song->mixpos_cs;
				memcpy(song->mixbufs,
					song->mixpos_cs,
					k*2);
				song->mixpos_cs=song->mixbufs;
				song->mixpos=song->mixbufs+k;
			}
		
			while((song->mixpos_cs+512)>song->mixpos)
			{
				TkMod_StepSongRow(song);
				for(i=0; i<song->tick_div; i++)
					TkMod_StepMixCtx(song, song->tick_samp);
			}
		}
	
		for(i=0; i<(2*16*16); i++)
		{
			a_lf=0;
			a_rt=0;
			for(j=0; j<16; j++)
			{
				if(!tkm_chan_samp[j])
					continue;
					
//				k=tkm_chan_pos[j]+(i>>1);
				k=tkm_chan_pos[j]+i;
				k=tkm_chan_samp[j]->data[k];
				k=(k-128)<<8;
				a_lf+=(k*tkm_chan_vol_lf[j])>>8;
				a_rt+=(k*tkm_chan_vol_rt[j])>>8;
			}
			
			a_lf=(a_lf*tkm_vol_sfx)>>8;

			if(song)
			{
//				l+=song->mixpos_cs[i];
				k=(song->mixpos_cs[i]*tkm_vol_music)>>8;
				a_lf+=k;
				a_rt+=k;
			}

			if(a_lf<-32767)		a_lf=-32767;
			if(a_lf> 32767)		a_lf= 32767;
			if(a_rt<-32767)		a_rt=-32767;
			if(a_rt> 32767)		a_rt= 32767;

			tkm_snd_mixbuf[i*2+0]=a_lf;
			tkm_snd_mixbuf[i*2+1]=a_rt;
		}

		for(j=0; j<16; j++)
		{
			if(!tkm_chan_samp[j])
				continue;
				
//			k=tkm_chan_pos[j]+(16*16)/2;
			k=tkm_chan_pos[j]+(16*16);
			tkm_chan_pos[j]=k;
			if(k>=tkm_chan_samp[j]->len)
				tkm_chan_samp[j]=0;
		}

		if(song)
		{
			song->mixpos_cs+=256;
		}
	
//		tkm_snd_mixbuf[]
		SoundDev_WriteStereoSamples2(tkm_snd_mixbuf, 16*16, 16*16*2);
		accdt-=16;
	}
}