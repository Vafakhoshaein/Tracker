#ifndef FLAGIMAGE_H
#define FLAGIMAGE_H

typedef struct image_t* image_handle;

/*for creating/resizing/destorying image*/
image_handle CreateImage(int width, int height);
void DestroyImage(image_handle* img);
image_handle resizeImage(image_handle img, int width, int height);



/*setting/removing/reseting flags*/
void setFlag(image_handle img, int col, int row);
void removeFlag(image_handle img, int col, int row);
void resetImage(image_handle img);

/*flag query*/
/*this function returns 1 if col/row is out of image range*/
int isFlaged(image_handle img, int col, int row);

/*dimension query*/
int imageWidth(image_handle img);
int imageHeight(image_handle img);

#endif // FLAGIMAGE_H
