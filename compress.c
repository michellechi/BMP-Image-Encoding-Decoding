#include <stdio.h>
#include <stdlib.h>
#include "header.h"
#include <string.h>
#include <math.h>

HuffNode *newNode(char data, unsigned freq)
{
    HuffNode *temp = (HuffNode *)malloc(sizeof(HuffNode));
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
    return temp;
}

PriorityQueue *createQueue(int capacity)
{
    PriorityQueue *queue = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    queue->size = 0;
    queue->capacity = capacity;
    queue->array = (HuffNode **)malloc(queue->capacity * sizeof(HuffNode *));
    return queue;
}

int isEmpty(PriorityQueue *queue)
{
    return queue->size == 0;
}

void freeTree(HuffNode *root)
{
    if (root == NULL)
    {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

void freePaths(char **paths)
{
    int i;
    for (i = 0; i < 256; i++)
    {
        if (paths[i] != NULL)
            free(paths[i]);
    }
    /* free(paths); */
}

void enQueue(PriorityQueue *queue, HuffNode *item)
{
    int i;
    if (queue->size == queue->capacity)
    {
        return;
    }
    i = queue->size;
    while (i > 0 && queue->array[(i - 1) / 2]->freq > item->freq)
    {
        queue->array[i] = queue->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    queue->array[i] = item;
    queue->size++;
}

HuffNode *deQueue(PriorityQueue *queue)
{
    int i, left, right, min;
    HuffNode *temp = NULL;
    if (isEmpty(queue))
    {
        return NULL;
    }
    temp = queue->array[0];
    queue->size--;
    if (queue->size > 0)
    {
        queue->array[0] = queue->array[queue->size];
        i = 0;
        while (2 * i + 1 < queue->size)
        {
            left = 2 * i + 1;
            right = 2 * i + 2;
            min = left;
            if (right < queue->size && queue->array[right]->freq < queue->array[left]->freq)
            {
                min = right;
            }
            if (queue->array[i]->freq > queue->array[min]->freq)
            {
                HuffNode *temp = queue->array[i];
                queue->array[i] = queue->array[min];
                queue->array[min] = temp;
                i = min;
            }
            else
            {
                break;
            }
        }
    }
    return temp;
}

int isLeaf(HuffNode *root)
{
    return !(root->left) && !(root->right);
}

HuffNode *buildHuffmanTree(unsigned int freq[], int variety)
{
    HuffNode *left, *right, *top, *root;
    int i;
    unsigned char data;
    PriorityQueue *queue = createQueue(variety);

    for (i = 0; i < 256; i++)
    {
        if (freq[i] != 0)
        {
            enQueue(queue, newNode(i, freq[i]));
        }
    }

    while (queue->size > 1)
    {
        left = deQueue(queue);
        right = deQueue(queue);

        data = left->data < right->data ? left->data : right->data;

        top = newNode(data, left->freq + right->freq);
        top->left = left;
        top->right = right;

        enQueue(queue, top);
    }

    root = deQueue(queue);
    free(queue->array);
    free(queue);
    return root;
}

/* Generate an array of paths 0s representing traversing left and 1 to the right */
char **generatePaths(unsigned int *freqs, HuffNode *root)
{
    int i;
    char **paths = (char **)malloc(256 * sizeof(char *));
    int arr[MAX_HT], depth = 0;
    for (i = 0; i < 256; i++)
    {
        if (freqs[i] != 0)
        {
            paths[i] = (char *)malloc(MAX_HT * sizeof(char));
        }
    }
    generatePathsHelper(root, arr, depth, paths);
    return paths;
}

void generatePathsHelper(HuffNode *root, int arr[], int depth, char **paths)
{
    int i;
    if (root->left)
    {
        arr[depth] = 0;
        generatePathsHelper(root->left, arr, depth + 1, paths);
    }

    if (root->right)
    {
        arr[depth] = 1;
        generatePathsHelper(root->right, arr, depth + 1, paths);
    }

    if (isLeaf(root))
    {
        for (i = 0; i < depth; i++)
        {
            paths[root->data][i] = arr[i] + '0';
        }
        paths[root->data][depth] = '\0';
    }
}

/* Compress the original imageData given the paths to each character */
tuple *compress(char **codes, unsigned char *original, BITMAPINFOHEADER *infoHeader)
{
    int size, padding = 0, i, j, bitIndex = 0;
    unsigned int byteIndex = 0;
    unsigned char *compressedData;
    char *path;
    char currentChar;
    tuple *res = malloc(sizeof(tuple));
    if (infoHeader->biWidth * 3 % 4 != 0)
    {
        padding = 4 - (infoHeader->biWidth * 3 % 4);
    }
    else
    {
        padding = 0;
    }
    size = (infoHeader->biWidth + padding) * infoHeader->biHeight * 3;
    compressedData = (unsigned char *)malloc(size * sizeof(unsigned char) * 256);
    res->data = compressedData;
    /* Replace the original chars with the path while compressing the strings down to unsigned chars*/
    for (i = 0; i < size; i++)
    {
        path = codes[original[i]];
        for (j = 0; j < strlen(path); j++)
        {
            currentChar = path[j];
            if (currentChar == '1')
            {
                compressedData[byteIndex] |= (1 << (7 - bitIndex));
            }
            bitIndex++;
            if (bitIndex == 8)
            {
                bitIndex = 0;
                byteIndex++;
            }
        }
    }

    res->size = byteIndex;
    return res;
}
