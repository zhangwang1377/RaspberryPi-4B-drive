#include <stdio.h>
#include <string.h>
#include <wchar.h>

int main(int argc, char const *argv[])
{
    wchar_t *chinese_str = "中";
    unsigned int *p = (wchar_t *)chinese_str;
    int i;

    printf("sizeof(wchar_t) = %d, str's Unicode: \n",(int)sizeof(wchar_t));
    for (i = 0; i < wcslen(chinese_str); i++)
    {
        printf("0x%x ",p[i]);
    }
    printf("\n");
    
    return 0;
}
