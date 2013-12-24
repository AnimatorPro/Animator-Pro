typedef struct ram_flic
	{
	Fli_head *head;		/* Actually points to the whole damn thing. */
	char *name;
	char *head_as_char;
	long frame1_oset;
	long frame2_oset;
	long cur_frame_oset;
	int cur_frame_ix;
	} Ram_flic;
void init_ram_flic(Ram_flic *rf);
void cleanup_ram_flic(Ram_flic *rf);
Errcode open_ram_flic(Ram_flic *rf, char *name);
Fli_frame *ram_flic_get_cur_frame(Ram_flic *rf);
void ram_flic_advance_cur_frame(Ram_flic *rf);

typedef struct list_of_ram_flics
	{
	Ram_flic *flics;
	unsigned int count;
	} List_of_ram_flics;
void init_list_of_ram_flics(List_of_ram_flics *rfl);
void cleanup_list_of_ram_flics(List_of_ram_flics *rfl);
Errcode set_list_of_ram_flics_size(List_of_ram_flics *rfl, int count);
Errcode add_to_list_of_ram_flics(List_of_ram_flics *rfl, char *name, int ix);
Errcode open_list_of_ram_flics(List_of_ram_flics *rfl, char *names[], int count
, Boolean report_progress);

