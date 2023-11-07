/* Preprocesses */
#define FILE_HEADER_SIZE 14
#define BITMAP_ID 0x4D42
#define MAX_HT 256
#include <stdio.h>

/* Typedefs */
typedef struct tuple
{
    unsigned char *data;
    unsigned int size;
} tuple;

typedef struct HuffNode
{
    unsigned char data;
    unsigned freq;
    struct HuffNode *left, *right;
} HuffNode;

typedef struct
{
    int size;
    int capacity;
    HuffNode **array;
} PriorityQueue;

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;      /*specifies the file type*/
    DWORD bfSize;     /*specifies the size in bytes of the bitmap file*/
    WORD bfReserved1; /*reserved; must be 0*/
    WORD bfReserved2; /*reserved; must be 0*/
    DWORD bfOffBits;  /*species the offset in bytes from the bitmapfileheader to the bitmap bits*/
} BITMAPFILEHEADER;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;         /*specifies the number of bytes required by the struct*/
    LONG biWidth;         /*specifies width in pixels*/
    LONG biHeight;        /*species height in pixels*/
    WORD biPlanes;        /*specifies the number of color planes, must be 1*/
    WORD biBitCount;      /*specifies the number of bit per pixel*/
    DWORD biCompression;  /*spcifies the type of compression*/
    DWORD biSizeImage;    /*size of image in bytes*/
    LONG biXPelsPerMeter; /*number of pixels per meter in x axis*/
    LONG biYPelsPerMeter; /*number of pixels per meter in y axis*/
    DWORD biClrUsed;      /*number of colors used by the bitmap*/
    DWORD biClrImportant; /*number of colors that are important*/
} BITMAPINFOHEADER;

#pragma pack(pop)

/* Prototypes */
unsigned char *grayScale(unsigned char *, BITMAPINFOHEADER *);
unsigned int *countFreq(unsigned char *, BITMAPINFOHEADER *);

BITMAPFILEHEADER *readFileHeader(const char *);
BITMAPINFOHEADER *readInfoHeader(const char *);
BITMAPFILEHEADER *getFileHeader(FILE *);
BITMAPINFOHEADER *getInfoHeader(FILE *);
unsigned char *loadBMP(const char *, BITMAPINFOHEADER *);
void writeBMP(const char *, unsigned char *, BITMAPFILEHEADER *, BITMAPINFOHEADER *);
unsigned char avg(unsigned char, unsigned char, unsigned char);
HuffNode *buildHuffmanTree(unsigned int *, int);
HuffNode *newNode(char, unsigned);
PriorityQueue *createQueue(int);
void enQueue(PriorityQueue *, HuffNode *);
HuffNode *deQueue(PriorityQueue *);
int isQueueEmpty(PriorityQueue *);
char **generatePaths(unsigned int *, HuffNode *);
void generatePathsHelper(HuffNode *, int[], int, char **);
tuple *compress(char **, unsigned char *, BITMAPINFOHEADER *);
unsigned char *decompress(FILE *, unsigned int *, BITMAPINFOHEADER *, int);
int getBitSize(FILE *);
unsigned int *buildFreqTable(FILE *);
void writeFile(BITMAPFILEHEADER *, BITMAPINFOHEADER *, unsigned char *, FILE *);
int readVariety(FILE *);
void freeTree(HuffNode *);
void freePaths(char **);
