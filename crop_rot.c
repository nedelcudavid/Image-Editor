#include "crop_rot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "io_ops.h"

// Intoarce o matrice de double, alocata intr-un singur bloc
static double **__double_mtx(int height, int row_size)
{
	// Aloca vectorul ce tine referinte catre inceputurile de rand
	double **mtx = (double **)calloc(height, sizeof(double *));
	assert(mtx);

	// Aloca blocul de memorie pentru matrice
	mtx[0] = calloc(height * row_size, sizeof(double));
	assert(mtx[0]);

	// Seteaza referintele
	for (int i = 1; i < height; i++)
		mtx[i] = mtx[i - 1] + row_size;

	return (void *)mtx;
}

// Verifica daca este selectata intreaga imagine
static int __is_selected_all(image_t *image, region_t *region)
{
	if (region->x1 != 0 || region->y1 != 0)
		return 0;
	if (region->x2 != image->width || region->y2 != image->height)
		return 0;
	return 1;
}

void crop(image_t *image, region_t *region)
{
	if (!IS_LOADED(image)) {
		printf("No image loaded\n");
		return;
	}

	if (__is_selected_all(image, region)) {
		printf("Image cropped\n");
		return;
	}

	// Muta, rand cu rand, pixelii din selectie in coltul stanga-sus
	int x1 = region->x1;
	int y1 = region->y1;
	int x2 = region->x2;
	int y2 = region->y2;

	// Creaza o imagine noua in care se va muta selectia, rand cu rand
	if (IS_COLOR(image)) {
		int row_size = (x2 - x1) * 3;

		rgb_t **new_image = (rgb_t **)__double_mtx(y2 - y1, row_size);

		// Copiaza randurile din selectie in noua imagine
		for (int i = y1; i < y2; i++)
			memcpy(new_image[i - y1], &image->RGB[i][x1],
				   row_size * sizeof(double));

		// Pune aceasta imagine noua in locul celei vechi
		free(image->RGB[0]);
		free(image->RGB);
		image->RGB = new_image;
	} else {
		int row_size = x2 - x1;

		double **new_image = __double_mtx(y2 - y1, row_size);

		for (int i = y1; i < y2; i++)
			memcpy(new_image[i - y1], &image->Y[i][x1],
				   row_size * sizeof(double));

		free(image->Y[0]);
		free(image->Y);
		image->Y = new_image;
	}

	// Seteaza noile dimensiuni ale imaginii
	image->height = y2 - y1;
	image->width = x2 - x1;
	__select_all(image, region);
	printf("Image cropped\n");
}

// Roteste intreaga imagine in sens orar la 90 de grade
static void __rot_all_once(image_t *image, region_t *region)
{
	// Creaza o noua imagine cu dimensiunile potrivite
	int new_h = image->width;
	int new_w = image->height;

	// Formula pentru noile pozitii ale pixelilor:
	// i_n = new_w - j - 1
	// j_n = i
	if (IS_COLOR(image)) {
		rgb_t **new_img = (rgb_t **)__double_mtx(new_h, new_w * 3);

		for (int i = 0; i < new_h; i++)
			for (int j = 0; j < new_w; j++)
				new_img[i][j] = image->RGB[new_w - j - 1][i];

		// Sterge vechea imagine si seteaz-o pe cea noua
		free(image->RGB[0]);
		free(image->RGB);
		image->RGB = new_img;
	} else {
		double **new_img = __double_mtx(new_h, new_w);

		for (int i = 0; i < new_h; i++)
			for (int j = 0; j < new_w; j++)
				new_img[i][j] = image->Y[new_w - j - 1][i];

		// Sterge vechea imagine si seteaz-o pe cea noua
		free(image->Y[0]);
		free(image->Y);
		image->Y = new_img;
	}

	// Seteaza noile dimensiuni
	image->width = new_w;
	image->height = new_h;
	__select_all(image, region);
}

// Roteste o selectie din imagine la 90 de grade
static void __rot_once(image_t *image, region_t *region)
{
	// Calculeaza latimea selectiei
	int len = region->x2 - region->x1;

	// Formula pentru noile pozitii ale pixelilor:
	// i_n = len - j - 1 + y1
	// j_n = i + x1
	int x1 = region->x1;
	int y1 = region->y1;

	if (IS_COLOR(image)) {
		// Creaza o matrice noua in care se va stoca selectia rotita,
		// apoi muta aceasta matrice peste imagine, rand cu rand

		rgb_t **new_img = (rgb_t **)__double_mtx(len, len * 3);

		for (int i = 0; i < len; i++)
			for (int j = 0; j < len; j++)
				new_img[i][j] = image->RGB[len - j - 1 + y1][i + x1];

		// Muta matricea in imagine

		for (int i = 0; i < len; i++)
			memcpy(&image->RGB[i + y1][x1], new_img[i], len * sizeof(rgb_t));

		// Sterge matricea auxiliara
		free(new_img[0]);
		free(new_img);
	} else {
		// Creaza o matrice noua in care se va stoca selectia rotita,
		// apoi muta aceasta matrice peste imagine, rand cu rand

		double **new_img = __double_mtx(len, len);

		for (int i = 0; i < len; i++)
			for (int j = 0; j < len; j++)
				new_img[i][j] = image->Y[len - j - 1 + y1][i + x1];

		// Muta matricea in imagine

		for (int i = 0; i < len; i++)
			memcpy(&image->Y[i + y1][x1], new_img[i], len * sizeof(double));

		// Sterge matricea auxiliara
		free(new_img[0]);
		free(new_img);
	}
}

void rotate(image_t *image, parser_t *parser, region_t *region)
{
	if (!IS_LOADED(image)) {
		printf("No image loaded\n");
		return;
	} else if (parser->argc != 2) {
		printf("Invalid command\n");
		return;
	} else if (atoi(parser->argv[1]) % 90) {
		printf("Unsupported rotation angle\n");
		return;
	} else if (region->x2 - region->x1 != region->y2 - region->y1) {
		if (!__is_selected_all(image, region)) {
			printf("The selection must be square\n");
			return;
		}
	}

	// Imaginea se va roti de mai multe ori cu 90 de grade. Calculeaza numarul
	// de astfel de rotatii.
	int number_of_rotations = atoi(parser->argv[1]) / 90;
	while (number_of_rotations < 0)
		number_of_rotations += 4;
	number_of_rotations %= 4;

	// Roteste imaginea in sens orar cate 90 de grade de numarul necesar de ori
	void (*rot_func)(image_t *image, region_t *region);

	if (__is_selected_all(image, region))
		rot_func = __rot_all_once;
	else
		rot_func = __rot_once;

	for (int i = 0; i < number_of_rotations; i++)
		rot_func(image, region);

	printf("Rotated %d\n", atoi(parser->argv[1]));
}
