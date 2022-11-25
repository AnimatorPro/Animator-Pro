#include "jimk.h" /* to get globals */
#include "menus.h"

char *rex_name = "\\paa\\aadisp.drv";
char goodconf;


Boolean menu_dopull();
void test_selit();

extern sys_pull;

static Menuhdr test_pull = {
	320, 8, 0, 30,
	0,
	PULLMENU,
	&sys_pull,
	SCREEN_FONT,	/* font */
	seebg_ulwhite,
	test_selit,
	menu_dopull,
	(KEYHIT|MMOVE|MBRIGHT),
};


static void do_testpull()
{
	do_menuloop(vl.screen,NULL,NULL,&test_pull,NULL);
}
static void test_selit(SHORT *ixs)
{
	boxf("test selit %d %d", ixs[0], ixs[1]);
}



static do_qnumber()
{
Menuhdr *mh;
SHORT val = 50;
Errcode err;

	err = new_qreq_number(&val,0,111,"the top line of text");
  	boxf("ecode %d, val = %d", err, val ); 
}
static test_uscale()
{
SHORT i, j, oc;
Vscoor vc;
SHORT badhits = 0;

	for( j = 100; j < 16000; ++j)
	{
		printf("%d to %d bh %d\n", -j, j, badhits);


		for(i = -j;i < j+1; ++i)
		{
			vc = scale_vscoor(i,j);
			oc = uscale_vscoor(vc,j);

			if(oc != i)
			{
				++badhits;
				printf("j %d i %d, in %d out %d\n", j, i, i, oc); 
			}
		}

		if(!yes_no_line("continue ??"))
			break;
	}
}
void main(int argc, char **argv)
{
Errcode err;
int i;


	if(argc > 1)
		rex_name = argv[1];
	if(init_sys() < 0)
		exit(-1);

	pj_set_rast(vl.screen,4);

test_uscale();

/*	do_testpull(); */

	if(!yes_no_line("continue ??"))
		goto exitit;

	do_qnumber();

	if(!yes_no_line("continue ??"))
		goto exitit;
{
char *choices[] = {
	"choice 1",
	"choice 2",
	"a very very big choice",
};

	err = qchoice(NULL, "header", choices, sizeof(choices)/sizeof(char *));
	boxf("choice %d", err);
}

	goto exitit;

error:
	boxf("error %d", err);
exitit:
	uninit_sys();
	exit(-1);
}
