#include <stdlib.h>
#include <stdio.h>
#include "header.h"
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char const *argv[])
{
    int i;
    BITMAPINFOHEADER *imageInfo = NULL;
    BITMAPFILEHEADER *imageFile = NULL;
    unsigned char *orig = NULL;
    unsigned int *freqs = NULL;
    char **codes = NULL;
    char ft = 0;
    HuffNode *root = NULL;
    int variety = 0, padding = 0;
    unsigned char *compressed = NULL;
    unsigned char *decompressed = NULL;
    unsigned char *temp = NULL;
    unsigned int size = 0;
    tuple *sizeByte = NULL;
    FILE *out, *fp = NULL;
    char *name = NULL;
    clock_t start, end;
    /*unsigned char *result = NULL;
    const char *dname = "decomp.bmp"; */

    imageFile = readFileHeader(argv[2]);
    imageInfo = readInfoHeader(argv[2]);
    padding = 4 - ((imageInfo->biWidth * 3) % 4);
    ft = argv[2][strlen(argv[2]) - 4];

    if (ft == '.')
    {
        printf("Compressing\n");
        start = clock();
        orig = loadBMP(argv[2], imageInfo);
        if (argv[1][1] == 'g')
        {
            temp = grayScale(orig, imageInfo);
            free(orig);
        }
        else
        {
            temp = orig;
        }
        freqs = countFreq(temp, imageInfo);
        for (i = 0; i < 256; i++)
        {
            if (freqs[i] != 0)
            {
                variety++;
            }
        }
        root = buildHuffmanTree(freqs, variety);
        codes = generatePaths(freqs, root);
        sizeByte = compress(codes, temp, imageInfo);
        size = sizeByte->size;
        compressed = sizeByte->data;
        name = malloc(sizeof(char) * (strlen(argv[2]) + 1));
        strcpy(name, argv[2]);
        name[strlen(argv[2]) - 4] = '\0';
        strcat(name, ".cbmp");
        out = fopen(name, "wb");
        fwrite(imageFile, sizeof(BITMAPFILEHEADER), 1, out);
        fwrite(imageInfo, sizeof(BITMAPINFOHEADER), 1, out);
        fwrite(&size, sizeof(unsigned int), 1, out);
        fwrite(&variety, sizeof(int), 1, out);
        for (i = 0; i < 256; i++)
        {
            if (freqs[i] != 0)
            {
                fwrite(&i, sizeof(unsigned char), 1, out);
                fwrite(&freqs[i], sizeof(unsigned int), 1, out);
            }
        }
        fwrite(compressed, sizeof(unsigned char) * size, 1, out);
        fclose(out);
        end = clock();
        printf("Time: %fms\n", (double)(end - start) / CLOCKS_PER_SEC * 1000);
        freeTree(root);
        free(name);
        free(sizeByte);
        freePaths(codes);
        free(freqs);
        free(compressed);
        free(codes);
        if (argv[1][1] == 'g')
            munmap(orig, (imageInfo->biWidth + padding) * imageInfo->biHeight * 3);
        else
            free(orig);
    }
    else
    {
        printf("Decompressing\n");
        start = clock();
        fp = fopen(argv[2], "rb");
        freqs = buildFreqTable(fp);
        size = getBitSize(fp);
        decompressed = decompress(fp, freqs, imageInfo, size);
        fclose(fp);
        if (argv[1][1] == 'g')
        {
            temp = grayScale(decompressed, imageInfo);
            free(decompressed);
        }
        else
        {
            temp = decompressed;
        }
        name = malloc(sizeof(char) * (strlen(argv[2]) + 1));
        strcpy(name, argv[2]);
        name[strlen(argv[2]) - 5] = '\0';
        strcat(name, ".bmp");
        out = fopen(name, "wb");
        writeFile(imageFile, imageInfo, temp, out);
        fclose(out);
        end = clock();
        printf("Time: %fms\n", (double)(end - start) / CLOCKS_PER_SEC * 1000);
        if (argv[1][1] != 'g')
        {
            free(decompressed);
        }
        free(freqs);
        free(name);
    }
    free(imageFile);
    free(imageInfo);
    return 0;
}
