#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/mman.h>
#include<time.h>
#include<sys/types.h>
#include<sys/wait.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

typedef struct tagBITMAPFILEHEADER{
    WORD bfType; /*specifies the file type*/
    DWORD bfSize; /*specifies the size in bytes of the bitmap file*/
    WORD bfReserved1; /*reserved; must be 0*/
    WORD bfReserved2; /*reserved; must be 0*/
    DWORD bfOffBits; /*specifies the offset in bytes from the bitmapfileheader to the bitmap bits*/
}BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
    DWORD biSize; /*specifies the number of bytes required by the struct*/
    LONG biWidth; /*specifies width in pixels*/
    LONG biHeight; /*species height in pixels*/
    WORD biPlanes; /*specifies the number of color planes, must be 1*/
    WORD biBitCount; /*specifies the number of bit per pixel*/
    DWORD biCompression;/*spcifies the type of compression*/
    DWORD biSizeImage; /*size of image in bytes*/
    LONG biXPelsPerMeter; /*number of pixels per meter in x axis*/
    LONG biYPelsPerMeter; /*number of pixels per meter in y axis*/
    DWORD biClrUsed; /*number of colors used by th ebitmap*/
    DWORD biClrImportant; /*number of colors that are important*/
}BITMAPINFOHEADER;

typedef struct HuffmanNode{
    int data;
    int freq;
    struct HuffmanNode *left, *right;
}HuffmanNode;

/* Function Declarations */
int* cnt_freq_gray(unsigned char*, BITMAPINFOHEADER*);
HuffmanNode create_huff_tree(int*);
unsigned char** create_code(HuffmanNode, unsigned char*);
void create_code_helper(HuffmanNode, unsigned char*, unsigned char**);
unsigned char* stringmaker(unsigned char*, unsigned char);

unsigned char* readimage(char*, BITMAPFILEHEADER*, BITMAPINFOHEADER*);
int* readcompress(char*, BITMAPFILEHEADER*, BITMAPINFOHEADER*, int*);
void writeimage(char*, BITMAPFILEHEADER*, BITMAPINFOHEADER*, unsigned char*);
void get_color(unsigned char*, int, int, int, int, int*, int*, int*);
void get_gray(unsigned char*, int, int, int, int, int*);
void makegray(unsigned char*, BITMAPINFOHEADER*, int, int, unsigned char*);
int comparator(const void*, const void*);
void shift(HuffmanNode**, int);

void writecompress(char*, BITMAPFILEHEADER*, BITMAPINFOHEADER*, unsigned char*, unsigned char**, int*);
void freetree(HuffmanNode*);
void decode(BITMAPINFOHEADER*, int*, HuffmanNode, unsigned char*, int, int);

int main(int argc, char *argv[]){
    BITMAPFILEHEADER fileheader;
    BITMAPINFOHEADER infoheader;
    unsigned char *bitmapImage;
    unsigned char *bitmapRes;
    int *freqs;
    unsigned char **hcodes;
    HuffmanNode root;
    size_t length;
    char *inputtype;
    char *input;
    char dot;
    int midy;
    int pid;
    int* huffmancode; /* for decompress */
    char* outputname;
    
    clock_t start;
    clock_t end;
    double timediff;

    start = clock();

    if(argc != 3){
        printf("Incorrect number of parameters\n");
        return 1;
    }

    dot = '.';
    inputtype = strrchr(argv[2], dot);
    if(strcmp(inputtype, ".bmp") == 0){ /* compressing */
        bitmapImage = readimage(argv[2], &fileheader, &infoheader);
        bitmapRes = mmap(NULL, ((infoheader.biWidth * 3) + (4 - ((infoheader.biWidth * 3) % 4)) % 4) * infoheader.biHeight*2, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);

        midy = (int)(infoheader.biHeight / 2);

        pid = fork();
        if(pid == 0){
            makegray(bitmapImage, &infoheader, 0, midy, bitmapRes);
            exit(0);
        }
        else{
            makegray(bitmapImage, &infoheader, midy, infoheader.biHeight, bitmapRes);
            wait(NULL);
        }

        /*makegray(bitmapImage, &infoheader, 0, infoheader.biHeight, bitmapRes);*/

        freqs = cnt_freq_gray(bitmapRes, &infoheader);
        root = create_huff_tree(freqs);
        hcodes = create_code(root, "");

        input = argv[2];
        length = strlen(input);
        strcpy(input + length - 4, ".cbmp");

        writecompress(input, &fileheader, &infoheader, bitmapRes, hcodes, freqs);
        freetree(&root);
        free(freqs);
        free(bitmapImage);
        free(hcodes);
        munmap(bitmapRes, ((infoheader.biWidth * 3) + (4 - ((infoheader.biWidth * 3) % 4)) % 4) * infoheader.biHeight*2);
        end = clock();
        timediff = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Result Time: %f seconds\n", timediff);
        printf("Done\n");
        return 0;
    }
    else if(strcmp(inputtype, ".cbmp") == 0){ /* decompressing */
        freqs = (int*)calloc(1, 256 * sizeof(int));  /* freed */
        huffmancode = readcompress(argv[2], &fileheader, &infoheader, freqs);

        root = create_huff_tree(freqs);

        bitmapRes = mmap(NULL, ((infoheader.biWidth * 3) + (4 - ((infoheader.biWidth * 3) % 4)) % 4) * infoheader.biHeight*2, PROT_READ | PROT_WRITE, MAP_SHARED | 0x20, -1, 0);

        decode(&infoheader, huffmancode, root, bitmapRes, 0, infoheader.biHeight);

        input = argv[2];
        length = strlen(input);
        strcpy(input + length - 5, ".bmp");

        writeimage(input, &fileheader, &infoheader, bitmapRes);
        freetree(&root);
        free(freqs);
        free(huffmancode);
        munmap(bitmapRes, ((infoheader.biWidth * 3) + (4 - ((infoheader.biWidth * 3) % 4)) % 4) * infoheader.biHeight*2);
        
        end = clock();
        timediff = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Result Time: %f seconds\n", timediff);
        printf("Done\n");
        return 0;
    }   
}

/* Read Image */
unsigned char* readimage(char* filename, BITMAPFILEHEADER* fileheader, BITMAPINFOHEADER* infoheader){
    FILE *fileptr;      /* File pointer */
    unsigned char* bitmapImage;
    int rowSize;
    int padding;
    int readret;

    /* Open file in binary mode*/
    fileptr = fopen(filename, "rb");
    if (fileptr == NULL){
        printf("At least one inputfile is missing\n");
        exit(1);
    }
    
    readret = fread(&(fileheader->bfType), sizeof(WORD), 1, fileptr); /* read filetype */
    if(readret == 0){
        fclose(fileptr);
        printf("One or more input images are not of type .BMP\n");
        exit(1); 
    }
    if(fileheader->bfType != 0x4D42){      /* Check if bmp file */
        fclose(fileptr);
        printf("One or more input images are not of type .BMP\n");
        exit(1); 
    }   
    fread(&(fileheader->bfSize), sizeof(DWORD), 1, fileptr); 
    fread(&(fileheader->bfReserved1), sizeof(WORD), 1, fileptr); 
    fread(&(fileheader->bfReserved2), sizeof(WORD), 1, fileptr); 
    fread(&(fileheader->bfOffBits), sizeof(DWORD), 1, fileptr); 

    fread(&(infoheader->biSize), sizeof(DWORD), 1, fileptr); /* start of infoheader */
    fread(&(infoheader->biWidth), sizeof(LONG), 1, fileptr); 
    fread(&(infoheader->biHeight), sizeof(LONG), 1, fileptr); 
    fread(&(infoheader->biPlanes), sizeof(WORD), 1, fileptr); 
    fread(&(infoheader->biBitCount), sizeof(WORD), 1, fileptr); 
    fread(&(infoheader->biCompression), sizeof(DWORD), 1, fileptr); 
    fread(&(infoheader->biSizeImage), sizeof(DWORD), 1, fileptr); 
    fread(&(infoheader->biXPelsPerMeter), sizeof(LONG), 1, fileptr); 
    fread(&(infoheader->biYPelsPerMeter), sizeof(LONG), 1, fileptr); 
    fread(&(infoheader->biClrUsed), sizeof(DWORD), 1, fileptr); 
    fread(&(infoheader->biClrImportant), sizeof(DWORD), 1, fileptr); 

    rowSize = infoheader->biWidth * 3;
    padding = (4 - (rowSize % 4)) % 4;

    bitmapImage = (unsigned char*)calloc(1, (rowSize + padding) * infoheader->biHeight * 2); /* freed */
  
    if(!bitmapImage){                   /* verify calloc */
        free(bitmapImage);
        fclose(fileptr);
        printf("Failed to allocate memory\n");
        exit(1);
    }

    fread(bitmapImage, (rowSize + padding) * infoheader->biHeight, 1, fileptr);

    if(bitmapImage == NULL){
        fclose(fileptr);
        printf("Failed to read bitmap image data\n");
        exit(1);
    }

    fclose(fileptr);
    return bitmapImage;
}

void writeimage(char* outfile, BITMAPFILEHEADER* fileheader, BITMAPINFOHEADER* infoheader, unsigned char* bitmapimage){
    FILE *outptr;
    int rowSize;
    int padding;
    outptr = fopen(outfile, "wb");

    fwrite(&(fileheader->bfType), sizeof(WORD), 1, outptr); 
    if(fileheader->bfType != 0x4D42){      /* Check if bmp file */
        fclose(outptr);
        printf("Output file not of type .BMP\n");
        exit(1); 
    }

    fwrite(&(fileheader->bfSize), sizeof(DWORD), 1, outptr); 
    fwrite(&(fileheader->bfReserved1), sizeof(WORD), 1, outptr); 
    fwrite(&(fileheader->bfReserved2), sizeof(WORD), 1, outptr); 
    fwrite(&(fileheader->bfOffBits), sizeof(DWORD), 1, outptr); 

    fwrite(&(infoheader->biSize), sizeof(DWORD), 1, outptr); /* start of infoheader */
    fwrite(&(infoheader->biWidth), sizeof(LONG), 1, outptr); 
    fwrite(&(infoheader->biHeight), sizeof(LONG), 1, outptr); 
    fwrite(&(infoheader->biPlanes), sizeof(WORD), 1, outptr); 
    fwrite(&(infoheader->biBitCount), sizeof(WORD), 1, outptr); 
    fwrite(&(infoheader->biCompression), sizeof(DWORD), 1, outptr); 
    fwrite(&(infoheader->biSizeImage), sizeof(DWORD), 1, outptr); 
    fwrite(&(infoheader->biXPelsPerMeter), sizeof(LONG), 1, outptr); 
    fwrite(&(infoheader->biYPelsPerMeter), sizeof(LONG), 1, outptr); 
    fwrite(&(infoheader->biClrUsed), sizeof(DWORD), 1, outptr); 
    fwrite(&(infoheader->biClrImportant), sizeof(DWORD), 1, outptr); 

    rowSize = infoheader->biWidth * 3;
    padding = (4 - (rowSize % 4)) % 4;

    fwrite(bitmapimage, (rowSize + padding) * infoheader->biHeight, 1, outptr);

    fclose(outptr);
    return;
}

void get_color(unsigned char* bitmapimage, int width, int height, int x, int y, int* blue, int* green, int* red){

    int padding;
    int index;
    int rowSize;
    rowSize = width * 3;
    padding = (4-(rowSize%4))%4;

    if(x >= 0 && x < width && y >= 0 && y < height){
        index = y * (rowSize + padding) + x * 3;
        *blue = bitmapimage[index];
        *green = bitmapimage[index+1];
        *red = bitmapimage[index+2];
    }
    else{
        *blue = 0;
        *green = 0;
        *red = 0;
    }
}

void get_gray(unsigned char* bitmapimage, int width, int height, int x, int y, int* gray){
    int padding;
    int index;
    int rowSize;
    rowSize = width * 3;
    padding = (4-(rowSize%4))%4;

    if(x >= 0 && x < width && y >= 0 && y < height){
        index = y * (rowSize + padding) + x * 3;
        *gray = bitmapimage[index];
    }
    else{
        *gray = 0;
    }
}

int* cnt_freq_gray(unsigned char* bitmap, BITMAPINFOHEADER* infoheader){
    int *freqs;
    int x;
    int y;
    int gray;
    
    freqs = (int*)calloc(1, 256 * sizeof(int)); /* freed */

    y = 0;
    while(y < infoheader->biHeight){      /* y < infoheader->biHeight */
        x = 0;
        while(x < infoheader->biWidth){    
            get_gray(bitmap, infoheader->biWidth, infoheader->biHeight, x, y, &gray);
            freqs[gray]++;
            x += 1;            
        }
        y += 1;
    }
    
    return freqs;
}

void makegray(unsigned char* bitmap, BITMAPINFOHEADER* infoheader, int starty, int endy, unsigned char* bitmapimage){
    int x;
    int y;
    int blue;
    int red;
    int green;
    int gray;
    int index;
    int padding;

    padding = (4 - ((infoheader->biWidth * 3) % 4)) % 4;

    y = starty;
    while(y < endy){      /* y < infoheader->biHeight */
        x = 0;
        while(x < infoheader->biWidth){    
            get_color(bitmap, infoheader->biWidth, infoheader->biHeight, x, y, &blue, &green, &red);

            gray = (red + green + blue) / 3;
            
            index = y * (infoheader->biWidth * 3 + padding) + x * 3;

            bitmapimage[index] = gray;          /* Append colors onto image data */
            bitmapimage[index+1] = gray;
            bitmapimage[index+2] = gray;

            x += 1;            
        }
        y += 1;
    }
    return;
}

HuffmanNode create_huff_tree(int* freqs){ 
    /* Takes a list of frequencies and returns root huffman node */ 
    int freq;
    HuffmanNode **struct_array;
    HuffmanNode *new;
    int size;
    int i;

    size = 0;

    for(freq = 0; freq < 256; freq++){
        if(freqs[freq] != 0){
            size += 1;
        }
    }

    struct_array = (HuffmanNode**)calloc(1, size * sizeof(HuffmanNode)); /* freed */

    i = 0;
    for(freq = 0; freq < 256; freq++){
        if(freqs[freq] != 0){
            new = (HuffmanNode*)malloc(sizeof(HuffmanNode));
            new->data = freq;
            new->freq = freqs[freq];
            new->left = NULL;
            new->right = NULL;
            struct_array[i] = new; 
            i++;
        }
    }

    qsort(struct_array, size, sizeof(HuffmanNode*), comparator);
    /*for(i=0;i<size;i++){
        printf("%d ", struct_array[i]->freq);
    }
    printf("\n");*/

    while(size > 1){
        new = (HuffmanNode*)malloc(sizeof(HuffmanNode));
        new->freq = struct_array[0]->freq + struct_array[1]->freq;
        new->data = struct_array[0]->data;
        new->left = struct_array[0];
        new->right = struct_array[1];
        struct_array[0] = new;
        shift(struct_array, size);
        size--;
        qsort(struct_array, size, sizeof(HuffmanNode*), comparator);
        /*for(i=0;i<size;i++){
            printf("%d left: %x right %x, ", struct_array[i]->freq, struct_array[i]->left, struct_array[i]->right);
        }
        printf("\n");*/
    }
    free(struct_array);
    return *new;
}

int comparator(const void *ptr1, const void *ptr2){
    const HuffmanNode* n1 = *(const HuffmanNode **)ptr1;
    const HuffmanNode* n2 = *(const HuffmanNode **)ptr2;

    if(n1->freq < n2->freq){
        return -1;
    }
    else if(n1->freq == n2->freq){
        if(n1->data < n2->data){
            return -1;
        }
    }
    else{
        return 1;
    }
}

void shift(HuffmanNode** array, int size){
    int i;
    for(i=1;i<(size-1);i++){
        array[i] = array[i+1];
    }
}

unsigned char** create_code(HuffmanNode root, unsigned char* str){
    unsigned char** hcodes;

    hcodes = (unsigned char**)calloc(1, 256 * sizeof(unsigned char*)); /* freed */
    create_code_helper(root, str, hcodes);
    /*bumjoon*/
    /*for(i=0;i<256;i++){
        printf("%s ", hcodes[i]);
    }*/
    return hcodes;
}

void create_code_helper(HuffmanNode node, unsigned char* value, unsigned char** array){
    unsigned char* temp;
    if(node.left == NULL && node.right == NULL){
        /*printf("%d \n", node.data);*/
        array[node.data] = value;
        return;
    }
    if(node.left != NULL){
        temp = stringmaker(value, '0');
        create_code_helper(*node.left, temp, array);
    }
    if(node.right != NULL){
        temp = stringmaker(value, '1');
        create_code_helper(*node.right, temp, array);
    }
}

unsigned char* stringmaker(unsigned char* string, unsigned char value){
    unsigned char* str;
    int len;
    len = strlen(string);
    str = (unsigned char*)calloc(1, (len + 2)*sizeof(unsigned char));
    str = strcpy(str, string);
    str[len] = value;
    str[len+1] = '\0';
    return str;
}

/* write compressed file */
void writecompress(char* outfile, BITMAPFILEHEADER* fileheader, BITMAPINFOHEADER* infoheader, unsigned char* bitmapimage, unsigned char** codes, int* freqs){
    FILE *outptr;
    int rowSize;
    int padding;
    int y;
    int x;
    int index;
    int gray;

    int bitcount = 0;
    int slen;
    unsigned char byte = 0; 
    int i;
    int freqcount;

    outptr = fopen(outfile, "wb");

    fwrite(&(fileheader->bfType), sizeof(WORD), 1, outptr); 
    if(fileheader->bfType != 0x4D42){      /* Check if bmp file */
        fclose(outptr);
        printf("Output file not of type .BMP\n");
        exit(1); 
    }

    fwrite(&(fileheader->bfSize), sizeof(DWORD), 1, outptr); 
    fwrite(&(fileheader->bfReserved1), sizeof(WORD), 1, outptr); 
    fwrite(&(fileheader->bfReserved2), sizeof(WORD), 1, outptr); 
    fwrite(&(fileheader->bfOffBits), sizeof(DWORD), 1, outptr); 

    fwrite(&(infoheader->biSize), sizeof(DWORD), 1, outptr); /* start of infoheader */
    fwrite(&(infoheader->biWidth), sizeof(LONG), 1, outptr); 
    fwrite(&(infoheader->biHeight), sizeof(LONG), 1, outptr); 
    fwrite(&(infoheader->biPlanes), sizeof(WORD), 1, outptr); 
    fwrite(&(infoheader->biBitCount), sizeof(WORD), 1, outptr); 
    fwrite(&(infoheader->biCompression), sizeof(DWORD), 1, outptr); 
    fwrite(&(infoheader->biSizeImage), sizeof(DWORD), 1, outptr); 
    fwrite(&(infoheader->biXPelsPerMeter), sizeof(LONG), 1, outptr); 
    fwrite(&(infoheader->biYPelsPerMeter), sizeof(LONG), 1, outptr); 
    fwrite(&(infoheader->biClrUsed), sizeof(DWORD), 1, outptr); 
    fwrite(&(infoheader->biClrImportant), sizeof(DWORD), 1, outptr); 
    
    fwrite(freqs, sizeof(int), 256, outptr);

    rowSize = infoheader->biWidth * 3;
    padding = (4 - (rowSize % 4)) % 4;

    bitcount = 0;

    y = 0;
    while(y < infoheader->biHeight){      /* y < infoheader->biHeight */
        x = 0;
        while(x < infoheader->biWidth){    
            get_gray(bitmapimage, infoheader->biWidth, infoheader->biHeight, x, y, &gray);

            slen = strlen(codes[gray]);
            /*printf("%s\n", codes[gray]);*/
            for(i = 0; i < slen; i++){
                if(codes[gray][i] == '1'){
                    if(bitcount == 7){
                        byte |= 1;
                    }
                    else{
                        byte |= 1;
                        byte <<= 1; 
                    }
                }
                else{
                    if(bitcount == 7){
                        byte <<= 0;
                    }
                    else{
                    byte <<= 1;
                    }
                }
                bitcount++;
                /*for (int i = 7; i >= 0; i--) {
                            unsigned char mask = 1 << i;
                            unsigned char bit = (byte & mask) >> i;
                            printf("%u", bit);
                        }
                    printf("\n");*/
                if(bitcount == 8){
                    fwrite(&byte, 1, 1, outptr);
                    byte = 0;
                    bitcount= 0;
                }
            } 
            x++;            
        }
        y++;
    }
    if(bitcount != 0){
        for(bitcount; bitcount < 7; bitcount++){
            byte <<= 1;
        }
        fwrite(&byte, 1, 1, outptr);   
    }
    fclose(outptr);
    return;
}

void freetree(HuffmanNode* node){ /*prob need to fix*/
    if(node->left == NULL && node->right == NULL){
        free(node);
        return;
    }
    if(node->left != NULL){
        freetree(node->left);
    }
    if(node->right != NULL){
        freetree(node->right);
    }
}

int* readcompress(char* filename, BITMAPFILEHEADER* fileheader, BITMAPINFOHEADER* infoheader, int* freqs){
    FILE *fileptr;      /* File pointer */
    int rowSize;
    int padding;
    int readret;
    int i;
    int success;
    unsigned char byte;
    unsigned char mask;
    int index = 0;
    int* tmp = NULL;
    int* huffmancode;

    int init = 0;
    unsigned char bit;
    huffmancode = (int*)malloc(sizeof(int)*8);

    /* Open file in binary mode*/
    fileptr = fopen(filename, "rb");
    if (fileptr == NULL){
        printf("At least one inputfile is missing\n");
        exit(1);
    }
    
    readret = fread(&(fileheader->bfType), sizeof(WORD), 1, fileptr); /* read filetype */
    if(readret == 0){
        fclose(fileptr);
        printf("One or more input images are not of type .CBMP\n");
        exit(1); 
    }
    if(fileheader->bfType != 0x4D42){      /* Check if bmp file */
        fclose(fileptr);
        printf("One or more input images are not of type .CBMP\n");
        exit(1); 
    }   
    fread(&(fileheader->bfSize), sizeof(DWORD), 1, fileptr); 
    fread(&(fileheader->bfReserved1), sizeof(WORD), 1, fileptr); 
    fread(&(fileheader->bfReserved2), sizeof(WORD), 1, fileptr); 
    fread(&(fileheader->bfOffBits), sizeof(DWORD), 1, fileptr); 

    fread(&(infoheader->biSize), sizeof(DWORD), 1, fileptr); /* start of infoheader */
    fread(&(infoheader->biWidth), sizeof(LONG), 1, fileptr); 
    fread(&(infoheader->biHeight), sizeof(LONG), 1, fileptr); 
    fread(&(infoheader->biPlanes), sizeof(WORD), 1, fileptr); 
    fread(&(infoheader->biBitCount), sizeof(WORD), 1, fileptr); 
    fread(&(infoheader->biCompression), sizeof(DWORD), 1, fileptr); 
    fread(&(infoheader->biSizeImage), sizeof(DWORD), 1, fileptr); 
    fread(&(infoheader->biXPelsPerMeter), sizeof(LONG), 1, fileptr); 
    fread(&(infoheader->biYPelsPerMeter), sizeof(LONG), 1, fileptr); 
    fread(&(infoheader->biClrUsed), sizeof(DWORD), 1, fileptr); 
    fread(&(infoheader->biClrImportant), sizeof(DWORD), 1, fileptr); 

    rowSize = infoheader->biWidth * 3;
    padding = (4 - (rowSize % 4)) % 4;

    fread(freqs, sizeof(int), 256, fileptr);

    success = 1;
    while(success == 1){
        success = fread(&byte, 1, 1, fileptr);
        if(success == 0){

        }
        else{
            tmp = realloc(huffmancode, (index + 8) * sizeof(int));
            huffmancode = tmp;
            for (i = 7; i >= 0; i--) {
                mask = 1 << i;
                bit = (byte & mask) >> i;
                huffmancode[index] = bit;
                index++;
                /*printf("%d ", bit);*/
            }
        }
    }

    /*for(i=0;i<index;i++){
        printf("%d ", huffmancode[i]);
    }*/
  
    return huffmancode;
}

void decode(BITMAPINFOHEADER* infoheader, int* huffmancode, HuffmanNode root, unsigned char* bitmapimage, int starty, int endy){
    int x;
    int y;
    int gray;
    int index;
    int padding;

    int num;
    int codeindex;
    HuffmanNode curr;

    curr = root;
    codeindex = 0;
    padding = (4 - ((infoheader->biWidth * 3) % 4)) % 4;

    y = starty;
    while(y < endy){      /* y < infoheader->biHeight */
        x = 0;
        while(x < infoheader->biWidth){  
            while(curr.left != NULL && curr.right != NULL){ /* get to last huffman node */
                num = huffmancode[codeindex];
                if(num == 1){
                    curr = *curr.right;
                }
                else{
                    curr = *curr.left;
                    }
                codeindex++;
            }
            gray = curr.data;
            curr = root;

            index = y * (infoheader->biWidth * 3 + padding) + x * 3;

            bitmapimage[index] = gray;          /* Append colors onto image data */
            bitmapimage[index+1] = gray;
            bitmapimage[index+2] = gray;

            x += 1;            
        }
        y += 1;
    }
    return;
}
