#include <nfd.h>
#include <stdio.h>

int main(void)
{
    
	NFD_Init();

	nfdchar_t *outPath;
	nfdfilteritem_t filterItem[2] = { { "Source code", "c,cpp,cc" }, { "Headers", "h,hpp" } };
	nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, "/Users/kiki/dev");
	if (result == NFD_OKAY)
	{
		puts("Success!");
		puts(outPath);
		NFD_FreePath(outPath);
	}
	else if (result == NFD_CANCEL)
	{
		puts("User pressed cancel.");
	}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}

	NFD_Quit();
	return 0;
}

