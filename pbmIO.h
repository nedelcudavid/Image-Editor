#ifndef _PBMIO_H
#define _PBMIO_H

#define IS_COLOR(img) ((img)->RGB ? 1 : 0)
#define IS_LOADED(image) ((image)->maxval ? 1 : 0)

typedef unsigned char byte_t;

typedef struct rgb_t {
	double r, g, b;
} rgb_t;

typedef struct image_t {
	rgb_t **RGB;
	double **Y;
	int width, height;
	int maxval;
} image_t;

#define IMAGE_INIT(img) ((void)memset((img), 0, sizeof(image_t)))

void discard_image(image_t *image);

int load_image(image_t *image, char *path);

void save_image(image_t *image, char *path, int ascii);

#endif
