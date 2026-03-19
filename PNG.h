#ifndef PNG_H
#define PNG_H

#include <stdint.h>

int steganographyPNG(const char *message, const char *filepath);
int isPNG(const uint8_t byte[8]);
char *extractSecretMessage(const char *filepath);

#endif