
char *names[] = {
	"Jim",
	"Heidi",
	"Chris",
	"Ann",
	"Peter",
	"Eric",
	"Cindy",
	"Jack",
	"Gary",
	"Lewis",
	"Jo",
	"Heidi",
	"Margeret",
	"Alice",
	"Billy",
	"Juanita",
	"Sally",
	"James",
	"Jacob",
	"Kevin",
	"Charlotte",
	"Alvin",
	"Fred",
};


ncmp(char **a, char **b)
{
return(strcmp(*a,*b));
}

main()
{
int i;
int name_size;

name_size = sizeof(names)/sizeof(names[0]);
qsort(names, name_size,  sizeof(names[0]), ncmp);
for (i=0; i<name_size; i++)
	{
	puts(names[i]);
	}
}

