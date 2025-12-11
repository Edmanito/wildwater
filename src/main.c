#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 4) {
        printf("Erreur : usage = ./run <add|sub> nb1 nb2\n");
        return 1;
    }

    char *cmd = argv[1];
    int a = atoi(argv[2]);
    int b = atoi(argv[3]);
    int result = 0;

    if (strcmp(cmd, "add") == 0) {
        result = a + b;
    }
    else if (strcmp(cmd, "sub") == 0) {
        result = a - b;
    }
    else {
        printf("Erreur : commande inconnue '%s'\n", cmd);
        return 2;
    }

    printf("\n%d\n\n", result);
    return 0;
}
