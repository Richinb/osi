#include <stdio.h>
#include <unistd.h>

int global_initialized = 4;
int global_uninitialized;
const int global_const = 5;

void A()
{
    printf("A:\n");
    int local_var = 1;
    static int static_var = 2;
    const int const_var = 3;

    printf("[Local] Address: %p\n", (void *)&local_var);
    printf("[Static] Address: %p\n", (void *)&static_var);
    printf("[Constant] Address: %p\n", (void *)&const_var);
    printf("[Global initialized] Address: %p\n", (void *)&global_initialized);
    printf("[Global uninitialized] Address: %p\n", (void *)&global_uninitialized);
    printf("[Global constant] Address: %p\n\n", (void *)&global_const);
}

void B()
{
    int pid = getpid();
    printf("B:\nPID: %d\n", pid);
    printf("cat /proc/%d/maps", pid);
    getchar();
}

int main()
{
    A();
    B();
    return 0;
}