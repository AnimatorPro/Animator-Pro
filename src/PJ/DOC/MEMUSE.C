/* memuse.c - generates a table of memory required to run PJ at
 * various resolutions */

#define PROG_SPACE (1024L*1024)
#define DOS_SPACE  (1024L*200)
#define SCREEN_BUFS 5


long mem_for_res(int x, int y, int screens)
{
long screen_size;
long total;

screen_size = x;
screen_size *= y;
total = PROG_SPACE+DOS_SPACE+screens*screen_size+screen_size/3;
return(total);
}

print_res(int w, int h)
{
char rbuf[80];

sprintf(rbuf, "%dx%d", w, h);
printf("%9s   %9ld   %9ld   %9ld   %9ld   %9ld\n", rbuf, 
	mem_for_res(w,h,2),
	mem_for_res(w,h,3),
	mem_for_res(w,h,4),
	mem_for_res(w,h,5),
	mem_for_res(w,h,6));
}


typedef struct res
	{
	int width, height;
	} Res;

Res res_table[] = 
	{
	{320,200},
	{512,480},
	{640,400},
	{640,480},
	{800,600},
	{1024,768},
	{1024,1024},
	{1500,1500},
	{1800,1800},
	{2000,2000},
	{300*8+150, 11*300},
	};

#define Array_els(arr) (sizeof(arr)/sizeof(arr[0]))

main()
{
int i,j, megs;

megs = 0;
for (i=0; i<4;i++ )
	{
	for (j=0; j<4; j++)
		{
		++megs;
		printf("%2d meg = %8ld  ", megs, megs*1024L*1024L);
		}
	puts("");
	}
for (i=0;  i<Array_els(res_table); i++)
	print_res(res_table[i].width, res_table[i].height);
}
