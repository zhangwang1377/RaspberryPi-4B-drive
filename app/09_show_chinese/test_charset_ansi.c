#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    char *str = "A��";//ascii   ISO 8859-5 
	int i;
	
	printf("str's len = %d\n", (int)strlen(str));
	printf("Hex code: ");
	for (i = 0; i < strlen(str); i++)
	{
		printf("%02x ", (unsigned char)str[i]);
	}
	printf("\n");
    return 0;
}
