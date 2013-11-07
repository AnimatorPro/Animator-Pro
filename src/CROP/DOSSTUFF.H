
struct fndata 
	{
	char reserved[21];
	char attribute;
	int time, date;
	long size;
	char name[13];
	};
extern struct fndata *find_dta();
