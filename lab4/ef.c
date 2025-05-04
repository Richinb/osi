#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void E()
{
    printf("E:\n");
    char *buf1 = (char *)malloc(100);
    if (buf1 == NULL)
    {
        perror("Failed to allocate buffer2");
        return;
    }

    strcpy(buf1, "hello world");
    printf("buf1 (before free): %s\n", buf1);
    free(buf1);
    printf("buf1 (after free): %s\n", buf1);

    char *buf2 = (char *)malloc(100);
    if (buf2 == NULL)
    {
        perror("Failed to allocate buffer2");
        return;
    }
    strcpy(buf2, "hello world");

    printf("buf2 (before free): %s\n", buf2);
    char *mid_ptr = buf2 + 50;
    free(mid_ptr);
    printf("buf2 (after free): %s\n", buf2);
}

int main()
{
    E();
    return 0;
}