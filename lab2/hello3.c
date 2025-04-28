#include <unistd.h>
#include <sys/syscall.h>

int main()
{
    const char msg[] = "Hello World\n";
    syscall(SYS_write, 1, msg, sizeof(msg) - 1);
    return 0;
}