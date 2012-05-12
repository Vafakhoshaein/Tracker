/**
 * FlagImage is a binary matrix that is created using CreateImage
 * The matrix entries are binary meaning that they could only be 0 or 1
 * This is an optimized version of a binary matrix in terms of memory and
 * performace.
 *
 * After using the matrix, destroy the image using DestroyImage() function
 *
 * Author: Vafa Khoshaein
 * Date:   Aug 9th, 2011
 * Ver no: 1.0
 **/

#include <FlagImage.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct image_t {
    int i_cols;
    int i_rows;
    int width_step; /*number of bytes in each row*/
    int size;       /*size of the data buffer*/
    unsigned char* data;
} image_t;

/*for creating/destorying images*/
image_handle CreateImage(int width, int height)
{
    image_handle img = (image_handle)malloc(sizeof(image_t));
    img->i_cols = width;
    img->i_rows = height;

    img->width_step = (width >> 3) + 1;
    img->size = img->width_step*height;
    img->data = malloc(img->size);
    resetImage(img);
    return img;
}

image_handle resizeImage(image_handle img, int width, int height)
{
    if (!img)
        return NULL;

    unsigned int _width_step = (width >> 3) + 1;
    unsigned int _size = _width_step*height;

    unsigned char* data = realloc(img->data, _size);
    if (data)
    {
        img->width_step = _width_step;
        img->size = _size;
        img->i_cols = width;
        img->i_rows = height;
        img->data = data;
        resetImage(img);
    }
    return img;
}

void DestroyImage(image_handle* img)
{
    if (img && *img)
    {
        free((*img)->data);
        free(*img);
        *img = NULL;
    }
}

/*setting/removing/reseting flags*/
void setFlag(image_handle img, int col, int row)
{
    if (col < 0 || col >= img->i_cols || row < 0 || row >= img->i_rows)
        return;

    int index = img->width_step * row + (col >> 3);
    unsigned char bitFlag = 0x80;
    bitFlag = bitFlag >> (col % 8);
    img->data[index] |= bitFlag;
}

void removeFlag(image_handle img, int col, int row)
{
    if (col < 0 || col >= img->i_cols || row < 0 || row >= img->i_rows)
        return;

    int index = img->width_step * row + (col >> 3);
    unsigned char bitFlag = (0x80 >> (col % 8)) ^ 0xFF;
    img->data[index] &= bitFlag;
}

void resetImage(image_handle img)
{
    memset(img->data, 0, img->size);
}

/*flag query*/
int isFlaged(image_handle img, int col, int row)
{
    if (col < 0 || col >= img->i_cols || row < 0 || row >= img->i_rows)
        return 1;

    int index = img->width_step * row + (col >> 3);
    unsigned char bitFlag = 0x80 >> (col % 8);
    return bitFlag & img->data[index];
}


int imageWidth(image_handle img)
{
    if (!img)
        return -1;
    return img->i_cols;
}

int imageHeight(image_handle img)
{
    if (!img)
        return -1;
    return img->i_rows;
}
