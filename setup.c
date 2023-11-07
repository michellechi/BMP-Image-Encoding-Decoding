#include <stdio.h>
#include <stdlib.h>
#include "header.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

unsigned char *grayScale(unsigned char *imageData, BITMAPINFOHEADER *infoHeader)
{
    unsigned char *gray = NULL;
    int pid;
    int flag = 1;
    int i, j, padding = 0, mid = 0;
    if ((infoHeader->biWidth * 3) % 4 != 0)
    {
        padding = 4 - ((infoHeader->biWidth * 3) % 4);
    }
    gray = mmap(NULL, sizeof(unsigned char) * (infoHeader->biWidth + padding) * infoHeader->biHeight * 3, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);
    if (flag)
    {
        pid = fork();
        if (pid == 0)
        {
            for (i = 0; i < infoHeader->biHeight / 2; i++)
            {
                for (j = 0; j < infoHeader->biWidth + padding; j++)
                {
                    if (j < infoHeader->biWidth)
                    {
                        /* Turn all pixels grayscale */
                        mid = avg(imageData[(i * infoHeader->biWidth) * 3 + j * 3],
                                  imageData[(i * infoHeader->biWidth) * 3 + j * 3 + 1],
                                  imageData[(i * infoHeader->biWidth) * 3 + j * 3 + 2]);
                        gray[(i * infoHeader->biWidth) * 3 + j * 3] = mid;
                        gray[(i * infoHeader->biWidth) * 3 + j * 3 + 1] = mid;
                        gray[(i * infoHeader->biWidth) * 3 + j * 3 + 2] = mid;
                    }
                    else
                    {
                        /* Add padding */
                        gray[(i * infoHeader->biWidth) * 3 + j * 3] = 0;
                    }
                }
            }
            exit(0);
        }
        else
        {
            for (i = infoHeader->biHeight / 2; i < infoHeader->biHeight; i++)
            {
                for (j = 0; j < infoHeader->biWidth + padding; j++)
                {
                    if (j < infoHeader->biWidth)
                    {
                        /* Turn all pixels grayscale */
                        mid = avg(imageData[(i * infoHeader->biWidth) * 3 + j * 3],
                                  imageData[(i * infoHeader->biWidth) * 3 + j * 3 + 1],
                                  imageData[(i * infoHeader->biWidth) * 3 + j * 3 + 2]);
                        gray[(i * infoHeader->biWidth) * 3 + j * 3] = mid;
                        gray[(i * infoHeader->biWidth) * 3 + j * 3 + 1] = mid;
                        gray[(i * infoHeader->biWidth) * 3 + j * 3 + 2] = mid;
                    }
                    else
                    {
                        /* Add padding */
                        gray[(i * infoHeader->biWidth) * 3 + j * 3] = 0;
                    }
                }
            }
            wait(NULL);
        }
    }
    else
    {
        for (i = 0; i < infoHeader->biHeight; i++)
        {
            for (j = 0; j < infoHeader->biWidth + padding; j++)
            {
                if (j < infoHeader->biWidth)
                {

                    mid = avg(imageData[(i * infoHeader->biWidth) * 3 + j * 3],
                              imageData[(i * infoHeader->biWidth) * 3 + j * 3 + 1],
                              imageData[(i * infoHeader->biWidth) * 3 + j * 3 + 2]);
                    gray[(i * infoHeader->biWidth) * 3 + j * 3] = mid;
                    gray[(i * infoHeader->biWidth) * 3 + j * 3 + 1] = mid;
                    gray[(i * infoHeader->biWidth) * 3 + j * 3 + 2] = mid;
                }
                else
                {

                    gray[(i * infoHeader->biWidth) * 3 + j * 3] = 0;
                }
            }
        }
    }
    return gray;
}

unsigned int *countFreq(unsigned char *imageData, BITMAPINFOHEADER *infoHeader)
{
    unsigned int *freq = malloc(sizeof(unsigned int) * 256);
    int i, padding = 0;
    if ((infoHeader->biWidth * 3) % 4 != 0)
    {
        padding = 4 - ((infoHeader->biWidth * 3) % 4);
    }
    for (i = 0; i < (infoHeader->biWidth + padding) * infoHeader->biHeight * 3; i++)
    {
        freq[imageData[i]]++;
    }
    return freq;
}

int getVariety(unsigned int *frequency)
{
    int i, count = 0;
    for (i = 0; i < 256; i++)
    {
        if (frequency[i] != 0)
        {
            count++;
        }
    }

    return count;
}

unsigned char avg(unsigned char a, unsigned char b, unsigned char c)
{
    return (a + b + c) / 3;
}

/* Function to read bitmap file header (14 bytes) */
BITMAPFILEHEADER *readFileHeader(const char *fileName)
{
    BITMAPFILEHEADER *bitmapFileHeader = malloc(sizeof(BITMAPFILEHEADER));
    FILE *filePtr = fopen(fileName, "rb");
    if (filePtr == NULL)
    {
        printf("%s not found, provide valid .bmp for input\n", fileName);
        free(bitmapFileHeader);
        return NULL;
    }

    fread(&bitmapFileHeader->bfType, sizeof(WORD), 1, filePtr);
    fread(&bitmapFileHeader->bfSize, sizeof(DWORD), 1, filePtr);
    bitmapFileHeader->bfReserved1 = 0;
    bitmapFileHeader->bfReserved2 = 0;
    fseek(filePtr, 10, SEEK_SET);
    fread(&bitmapFileHeader->bfOffBits, sizeof(DWORD), 1, filePtr);
    fclose(filePtr);
    return bitmapFileHeader;
}

/* Function to read bitmap info header (40 bytes) */
BITMAPINFOHEADER *readInfoHeader(const char *fileName)
{
    BITMAPINFOHEADER *bitmapInfoHeader = malloc(sizeof(BITMAPINFOHEADER));
    FILE *filePtr = fopen(fileName, "rb");
    if (filePtr == NULL)
    {
        printf("%s not found, provide valid .bmp for input\n", fileName);
        free(bitmapInfoHeader);
        return NULL;
    }
    fseek(filePtr, FILE_HEADER_SIZE, SEEK_SET);
    fread(&bitmapInfoHeader->biSize, sizeof(DWORD), 1, filePtr);
    fread(&bitmapInfoHeader->biWidth, sizeof(LONG), 1, filePtr);
    fread(&bitmapInfoHeader->biHeight, sizeof(LONG), 1, filePtr);
    fread(&bitmapInfoHeader->biPlanes, sizeof(WORD), 1, filePtr);
    fread(&bitmapInfoHeader->biBitCount, sizeof(WORD), 1, filePtr);
    fread(&bitmapInfoHeader->biCompression, sizeof(DWORD), 1, filePtr);
    fread(&bitmapInfoHeader->biSizeImage, sizeof(DWORD), 1, filePtr);
    fread(&bitmapInfoHeader->biXPelsPerMeter, sizeof(LONG), 1, filePtr);
    fread(&bitmapInfoHeader->biYPelsPerMeter, sizeof(LONG), 1, filePtr);
    fread(&bitmapInfoHeader->biClrUsed, sizeof(DWORD), 1, filePtr);
    fread(&bitmapInfoHeader->biClrImportant, sizeof(DWORD), 1, filePtr);
    fclose(filePtr);
    return bitmapInfoHeader;
}

unsigned char *loadBMP(const char *fileName, BITMAPINFOHEADER *bitmapInfoHeader)
{
    unsigned char *bitmapImage = NULL;         /* bitmap image data (the returned value) */
    FILE *filePointer = fopen(fileName, "rb"); /* open bitmap file for reading */
    int padding = 0;                           /* number of padding bytes per row */
    if (bitmapInfoHeader->biWidth % 4 != 0)
    {
        padding = 4 - (bitmapInfoHeader->biWidth % 4);
    }

    /* extract image data */
    fseek(filePointer, 54, SEEK_SET);
    bitmapImage = malloc((bitmapInfoHeader->biWidth + padding) * bitmapInfoHeader->biHeight * 3);
    fread(bitmapImage, 1, (bitmapInfoHeader->biWidth + padding) * bitmapInfoHeader->biHeight * 3, filePointer);

    /* close file and return bitmap image data */
    fclose(filePointer);
    return bitmapImage;
}

void writeBMP(const char *filename, unsigned char *imageData, BITMAPFILEHEADER *bitmapFileHeader, BITMAPINFOHEADER *bitmapInfoHeader)
{
    int padding = 0;
    FILE *filePtr = fopen(filename, "wb");
    fwrite(bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
    fwrite(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
    if (bitmapInfoHeader->biWidth * 3 % 4 != 0)
    {
        padding = 4 - (bitmapInfoHeader->biWidth * 3 % 4);
    }
    fwrite(imageData, (bitmapInfoHeader->biWidth + padding) * bitmapInfoHeader->biHeight * 3, 1, filePtr);
    fclose(filePtr);
}
