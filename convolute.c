#include "convolute.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define INVALID 0
#define BLUR 1
#define GAUSSIAN_BLUR 2
#define SHARPEN 3
#define DET_EDGE 4

#define KER_SIZE 3

static int __filter_type(char *arg)
{
	if (!strcmp(arg, "BLUR"))
		return BLUR;
	else if (!strcmp(arg, "GAUSSIAN_BLUR"))
		return GAUSSIAN_BLUR;
	else if (!strcmp(arg, "SHARPEN"))
		return SHARPEN;
	else if (!strcmp(arg, "EDGE"))
		return DET_EDGE;

	return INVALID;
}

// Obtine nucleul de convolutare a filtrului specificat
static double __set_kernel(int kernel[KER_SIZE][KER_SIZE], int filter)
{
	switch (filter) {
	case BLUR:
		for (int i = 0; i < KER_SIZE; i++)
			for (int j = 0; j < KER_SIZE; j++)
				kernel[i][j] = 1;
		return 9;

	case GAUSSIAN_BLUR:
		// Seteaza marginile
		for (int i = 0; i < KER_SIZE; i++)
			for (int j = 0; j < KER_SIZE; j++)
				kernel[i][j] = ((i + j) % 2) ? 2 : 1;
		kernel[1][1] = 4;

		return 16;

	case SHARPEN:
		// Seteaza marginile
		for (int i = 0; i < KER_SIZE; i++)
			for (int j = 0; j < KER_SIZE; j++)
				kernel[i][j] = ((i + j) % 2) ? -1 : 0;
		kernel[1][1] = 5;

		return 1;

	case DET_EDGE:
		// Seteaza marginile
		for (int i = 0; i < KER_SIZE; i++)
			for (int j = 0; j < KER_SIZE; j++)
				kernel[i][j] = -1;
		kernel[1][1] = 8;

		return 1;

	default:
		return 0;
	}
}

// Creaza o plansa cu 3 canale de dimensiuni width x height
static rgb_t **__create_canvas(int width, int height)
{
	// Aloca spatiu pentru referintele catre randuri
	rgb_t **canvas = (rgb_t **)calloc(height, sizeof(rgb_t **));
	assert(canvas);

	// Spatiul pentru pixeli
	canvas[0] = (rgb_t *)calloc(height * width, sizeof(rgb_t));
	assert(canvas[0]);

	// Seteaza referintele pt randuri
	for (int i = 1; i < height; i++)
		canvas[i] = canvas[i - 1] + width;

	return canvas;
}

static int __on_edge(image_t *image, int i, int j)
{
	// Determina daca pixelul are suficienti vecini pentru a realiza
	// convolutarea
	if (i == 0 || j == 0)
		return 1;
	if (i == image->height - 1 || j == image->width - 1)
		return 1;
	return 0;
}

static double __clamp(image_t *image, double val)
{
	// Incadreaza valoarea intre limite
	if (val >= image->maxval)
		return (double)image->maxval;
	else if (val <= 0)
		return 0;
	return val;
}

static void __convolute(image_t *image, region_t *region, int kernel[][3],
						double factor)
{
	// Creaza o "plansa" unde se va crea regiunea convolutata
	int r_width = region->x2 - region->x1;
	int r_height = region->y2 - region->y1;
	rgb_t **canvas = __create_canvas(r_width, r_height);

	for (int i = 0; i < r_height; i++)
		for (int j = 0; j < r_width; j++) {
			// Verifica daca pixelul este pe margine
			int y = region->y1 + i;
			int x = region->x1 + j;

			if (__on_edge(image, y, x)) {
				// Copiaza pixelul pe plansa, acesta nu are suficienti vecini
				canvas[i][j] = image->RGB[y][x];
				continue;
			}

			// Calculeaza valoarea intensitatii fiecarui canal
			double r = 0, g = 0, b = 0;

			for (int p = 0; p < KER_SIZE; p++)
				for (int q = 0; q < KER_SIZE; q++) {
					r += kernel[p][q] * image->RGB[y + p - 1][x + q - 1].r;
					g += kernel[p][q] * image->RGB[y + p - 1][x + q - 1].g;
					b += kernel[p][q] * image->RGB[y + p - 1][x + q - 1].b;
				}

			// Seteaza noile valori
			canvas[i][j].r = __clamp(image, r / factor);
			canvas[i][j].g = __clamp(image, g / factor);
			canvas[i][j].b = __clamp(image, b / factor);
		}

	// Copiaza continutul plansei peste imagine
	int x1 = region->x1;
	int y1 = region->y1;

	int row_size = r_width * sizeof(rgb_t);
	for (int i = 0; i < r_height; i++)
		memcpy(&image->RGB[i + y1][x1], canvas[i], row_size);

	// Sterge plansa
	free(canvas[0]);
	free(canvas);
}

void apply(image_t *image, parser_t *parser, region_t *region)
{
	if (!IS_LOADED(image)) {
		printf("No image loaded\n");
		return;
	} else if (parser->argc != 2) {
		printf("Invalid command\n");
		return;
	} else if (!IS_COLOR(image)) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}

	// Verifica tipul de filtru
	int filter = __filter_type(parser->argv[1]);

	if (filter == INVALID) {
		printf("APPLY parameter invalid\n");
		return;
	}

	// Calculeaza nucleul si realizeaza convolutarea
	int kernel[KER_SIZE][KER_SIZE];
	double denominator = __set_kernel(kernel, filter);

	__convolute(image, region, kernel, denominator);

	printf("APPLY %s done\n", parser->argv[1]);
}
