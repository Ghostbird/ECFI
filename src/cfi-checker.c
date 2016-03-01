#include <stdio.h> /* printf() */
int main(int argc, char *argv[])
{
    printf("Checker ran with arguments:\n");
    for (int i = 0; i < argc; i++)
    {
        printf(" %i: %s\n", i, argv[i]);
    }
    return 0;
}
