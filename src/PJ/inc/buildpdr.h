
#ifndef BUILDPDR_H
#define BUILDPDR_H

typedef struct config_pdr_info {
	char *save_pdr;
	char *default_pdr;
	char save_suffi[PDR_SUFFI_SIZE];
	UBYTE suffi_loaded;
	char *local_name;
	char *local_title;
	char *local_info;
	char *local_suffi;
	Errcode (*local_get_ainfo)(char *ifname,Anim_info *ainfo);
	LONG read_frames, write_frames;
	char last_read[FILE_NAME_SIZE]; /* type of last successful read */
	UBYTE type;
} Config_pdr;


typedef struct pdr_entry {
	Names hdr;
	char *pdr_name;
	char name_buf[50];
	char suffi[PDR_SUFFI_SIZE];
	UBYTE does_read;
	UBYTE does_write;
} Pdr_entry;


extern Errcode pic_anim_info(char *ifname,Anim_info *ainfo);
extern Errcode pj_fli_info(char *ifname,Anim_info *ainfo);
Errcode select_save_pdr(Config_pdr *cpdr);

#define PICTYPE 0
#define FLICTYPE 1

#endif /* BUILDPDR_H */
