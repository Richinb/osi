#include <stdio.h>
#include <stdlib.h>

void H()
{
    printf("H:\n");
    // h.i. Распечатать текущее значение переменной
    const char *var_name = "NEW_VAR";
    printf("Name: %s\n", var_name);

    char *var_value = getenv(var_name); // чтение
    if (var_value == NULL)
    {
        // обрабатываю это как ошибку, так как по заданию нужно увидеть различие до и после для уже созданной переменной
        fprintf(stderr, "no such env var: %s\n", var_name);
        return;
    }
    printf("Original value: %s\n", var_value);

    // h.ii. Изменить значение переменной
    int returned_setenv = setenv(var_name, "new_value", 1);
    if (returned_setenv == -1)
    {
        perror("setenv");
        return;
    }

    // h.iii. Распечатать новое значение
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
    H();
    return 0;
}