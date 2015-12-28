#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
	int i;
	for(i=0; i<argc; i++)
		fprintf(stderr, "arg[%d] is %s\n", i, argv[i]);
	return 0;
}
