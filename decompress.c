#include "header.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Decompresses a custom .cbmp image */
BITMAPFILEHEADER *getFileHeader(FILE *fp)
{
    BITMAPFILEHEADER *fileHeader = (BITMAPFILEHEADER *)malloc(sizeof(BITMAPFILEHEADER));
    fread(fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    return fileHeader;
}

BITMAPINFOHEADER *getInfoHeader(FILE *fp)
{
    BITMAPINFOHEADER *infoHeader = (BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER));
    fseek(fp, sizeof(BITMAPFILEHEADER), SEEK_SET);
    fread(infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
    return infoHeader;
}

unsigned char *getData(FILE *fp, BITMAPINFOHEADER *infoHeader)
{
    int variety = 0;
    unsigned char *imageData = NULL;
    unsigned int bitSize = 0;
    fseek(fp, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), SEEK_SET);
    fread(&bitSize, sizeof(unsigned int), 1, fp);
    fread(&variety, sizeof(int), 1, fp);
    fseek(fp, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(int) + sizeof(unsigned int) + variety * (sizeof(unsigned char) + sizeof(int)), SEEK_SET);
    imageData = (unsigned char *)malloc(sizeof(unsigned char) * bitSize);
    fread(imageData, sizeof(unsigned char), bitSize, fp);
    return imageData;
}

unsigned int *buildFreqTable(FILE *fp)
{
    unsigned int *freq = malloc(sizeof(int) * 256);
    int i, variety;
    unsigned char index;
    fseek(fp, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(unsigned int), SEEK_SET);
    variety = readVariety(fp);
    for (i = 0; i < variety; i++)
    {
        fread(&index, sizeof(unsigned char), 1, fp);
        fread(&freq[index], sizeof(unsigned int), 1, fp);
    }
    return freq;
}

int getBitSize(FILE *fp)
{
    int bitSize;
    fseek(fp, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), SEEK_SET);
    fread(&bitSize, sizeof(int), 1, fp);
    return bitSize;
}

int readVariety(FILE *fp)
{
    int variety;
    fseek(fp, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(unsigned int), SEEK_SET);
    fread(&variety, sizeof(int), 1, fp);
    return variety;
}

/* Decompress the image by reconstructing the compressed image data using huffman codes */
unsigned char *decompress(FILE *fp, unsigned int *freq, BITMAPINFOHEADER *infoHeader, int bitSize)
{
    int i, j, padding = 0, variety = 0;
    int bitCount = 0;
    unsigned char *imageData = NULL;
    unsigned char *compressedData = getData(fp, infoHeader);
    HuffNode *temp = NULL;
    HuffNode *root = NULL;
    if (infoHeader->biWidth * 3 % 4 != 0)
    {
        padding = 4 - (infoHeader->biWidth * 3 % 4);
    }
    variety = readVariety(fp);
    root = buildHuffmanTree(freq, variety);

    fseek(fp, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(unsigned int), SEEK_SET);
    imageData = (unsigned char *)malloc(sizeof(unsigned char) * (infoHeader->biWidth + padding) * infoHeader->biHeight * 3);

    /* read each bit to travese the huffman tree until a leaf node is reached */
    temp = root;
    for (i = 0; i < bitSize; i++)
    {
        for (j = 0; j < 8; j++)
        {
            if (temp->left == NULL && temp->right == NULL)
            {
                imageData[bitCount] = temp->data;
                bitCount++;
                temp = root;
            }
            if (compressedData[i] & (1 << (7 - j)))
            {
                temp = temp->right;
            }
            else
            {
                temp = temp->left;
            }
        }
    }
    freeTree(root);
    free(compressedData);
    return imageData;
}

void writeFile(BITMAPFILEHEADER *fileHeader, BITMAPINFOHEADER *infoHeader, unsigned char *imageData, FILE *fp)
{
    int padding = 0;
    if ((infoHeader->biWidth * 3) % 4 != 0)
    {
        padding = 4 - ((infoHeader->biWidth * 3) % 4);
    }
    fseek(fp, 0, SEEK_SET);
    fwrite(fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
    fwrite(imageData, sizeof(unsigned char), (infoHeader->biWidth + padding) * infoHeader->biHeight * 3, fp);
}
