#include <stdio.h>
#include <stdlib.h>

void G()
{
    printf("G:\n");
    const char *var_name = "NEW_VAR";
    printf("Name: %s\n", var_name);

    char *var_value = getenv(var_name);
    if (var_value == NULL)
    {
        fprintf(stderr, "Error geting env variable: %s\n", var_name);
        return;
    }
    printf("Original value: %s\n", var_value);

    int returned_setenv = setenv(var_name, "new_value", 1);
    if (returned_setenv == -1)
    {
        perror("setenv");
        return;
    }

    var_value = getenv(var_name);
    if (var_value == NULL)
    {
        fprintf(stderr, "no such env var");
        return;
    }
    printf("New value: %s\n\n", var_value);
}

int main()
{
    G();
    return 0;
}