#include <stdio.h>
#include "PNG.h"

int main(void) {

    printf("\nIl messaggio segreto e' = %s", extractSecretMessage("secret.png"));
    return 0;
}