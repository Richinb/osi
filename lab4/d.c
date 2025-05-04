#include <stdio.h>

int *return_local_variable()
{
    int local = 1;
    int *result = &local;
    return result;
}

void D()
{
    printf("D:\n");
    int *ptr = return_local_variable();
    printf("[Local variable] Address: %p, Value: %d\n\n", (void *)ptr, *ptr);
}

int main()
{
    D();
    return 0;
}