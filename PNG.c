#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define GOOD_ENDING 1
#define BAD_ENDING 0

/* ---- PNG ---- */

typedef struct PNG{
    uint8_t pixels_len[4];  // 4 bytes for the chunk len
    uint8_t chunk_name[4]; // 4 bytes for the chunk type
    uint8_t *pixels;  // The data depending
    uint32_t CRC;
}PNG;

int isPNG(const uint8_t byte[8]) {
    const uint8_t png[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };  // the first 8 bytes of a PNG
    return memcmp(byte, png, 8) == 0;  // memcmp returns 0 if the 2 memories are equals
}

void createPixelsLen(uint8_t pixel_len[4], const uint32_t len) {  // Big Endian
    int offset = 24;
    for (int i = 0; i < 4; i++) {
        pixel_len[i] = (len >> offset) & 0xFF;  // Mapping each byte of len inside each index
        offset -= 8;
    }
}

void createChunkName(uint8_t chunk_name[4], const char *name) {  // Creates our chunk name
    memcpy(chunk_name, name, 4);
}

void createCRC(uint32_t *crc, const uint8_t chunk_name[4], const uint8_t *pixels, const size_t len) {  // Create the crc of our chunk
    *crc = 0xFFFFFFFF;
    for (size_t i = 0; i < 4; i++) {
        *crc ^= chunk_name[i];
        for (size_t j = 0; j < 8; j++) {
            const uint8_t lsb = *crc & 1;  // least significant bit
            *crc = *crc >> 1;  // Right Shift
            if (lsb == 1) {
                *crc ^= 0xEDB88320;
            }
        }
    }
    for (size_t i = 0; i < len; i++) {
        *crc ^= pixels[i];
        for (size_t j = 0; j < 8; j++) {
            const uint8_t lsb = *crc & 1;  // least significant bit
            *crc = *crc >> 1;  // Right Shift
            if (lsb == 1) {
                *crc ^= 0xEDB88320;
            }
        }
    }
    *crc ^= 0xFFFFFFFF;  // Inverting crc bits
    *crc = htonl(*crc);  // Checks if your system uses Big Endian or Little Endian and inverts the order
}

void constructor(PNG *png, const uint8_t *s, const char *name, const size_t len) {
    createChunkName(png->chunk_name, name);  // the name would be 'prIv' because the system ignores it
    createPixelsLen(png->pixels_len, len);
    createCRC(&png->CRC, png->chunk_name, s, len);
    png->pixels = (uint8_t *)s;
}

int steganographyPNG(const char *message, const char *filepath) {
    // Let's open the file and create a new png which will look the same of the original
    FILE *fin = fopen(filepath, "rb");
    if (fin == NULL) {
        fprintf(stderr,"Error opening file.");
        return BAD_ENDING;
    }
    FILE *fout = fopen("secret.png", "wb");
    if (fout == NULL) {
        fprintf(stderr,"Error creating file.");
        return BAD_ENDING;
    }

    uint8_t firstbytes[8];  // 8 bytes where is written the format
    fread(firstbytes, sizeof(uint8_t), 8, fin);

    if (!isPNG(firstbytes)) {  // If is not a PNG returns BAD_ENDING
        fprintf(stderr,"Error this file is not a PNG.\nPass a PNG file please.");
        fclose(fin);
        fclose(fout);
        return BAD_ENDING;
    }

    fwrite(firstbytes, sizeof(uint8_t), 8, fout);

    /* Reading the ihdr chunk which is exactly 25 bytes */
    uint8_t ihdr_chunk[25];
    fread(ihdr_chunk, sizeof(uint8_t), 25, fin);
    fwrite(ihdr_chunk, sizeof(uint8_t), 25, fout);

    PNG png;  // Structure that will contain the new chunk
    constructor(&png, (uint8_t*)message, "prIv", strlen(message));

    fwrite(png.pixels_len, sizeof(uint8_t), 4, fout);
    fwrite(png.chunk_name, sizeof(uint8_t), 4, fout);
    fwrite(png.pixels, sizeof(uint8_t),strlen(message), fout);
    fwrite(&png.CRC, sizeof(uint32_t), 1, fout);

    uint8_t buffer[1024];
    size_t byteread;  // counts how many bytes are read
    while ((byteread = fread(buffer, sizeof(uint8_t), 1024, fin)) > 0) {
        fwrite(buffer, sizeof(uint8_t), byteread, fout);  // It writes exactly the byte read
    }

    fclose(fin);
    fclose(fout);

    return GOOD_ENDING;
}

char *extractSecretMessage(const char *filepath) {
    FILE *fin = fopen(filepath, "rb");
    if (fin == NULL) {
        fprintf(stderr,"Error opening file.");
        return NULL;
    }

    uint8_t firstbytes[8];
    fread(firstbytes, sizeof(uint8_t), 8, fin);

    if (!isPNG(firstbytes)) {  // If is not a PNG returns BAD_ENDING
        fprintf(stderr,"Error this file is not a PNG.\nPass a PNG file please.");
        fclose(fin);
        return NULL;
    }

    uint32_t pixels_len;
    uint8_t chunk_name[4];

    char *message = NULL;

    while (!feof(fin)) {
        /* Let's read the first 8 bytes of each chunk */
        fread(&pixels_len, sizeof(uint32_t), 1, fin);
        pixels_len = ntohl(pixels_len);  // Takes in consideration the Big Endian and Little Endian
        fread(chunk_name, sizeof(uint8_t), 4, fin);

        if (memcmp(chunk_name, "prIv", 4) != 0) {
            fseek(fin, (int)pixels_len + 4, SEEK_CUR);
        }else {
            message = (char*)malloc((pixels_len + 4) * sizeof(char));
            if (message == NULL) {
                fprintf(stderr,"Error allocating memory for buffer.");
                return NULL;
            }
            fread(message, sizeof(char), pixels_len, fin);
            message[pixels_len] = '\0'; // Closing the message
            fclose(fin);
            return message;
        }
    }
    fclose(fin);
    return message;
}

/* ---- PNG ---- */