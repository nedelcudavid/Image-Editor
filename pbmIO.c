#include "pbmIO.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define LINE_SIZE 32

// Aloca o matrice de bytes de dimensiuni specificate
static byte_t **__get_byte_matrix(int height, int width)
{
	// aloca memorie pentru referinte
	byte_t **matrix = (byte_t **)calloc(sizeof(byte_t *), height);
	assert(matrix);

	// Aloca memorie pt elemente
	matrix[0] = (byte_t *)calloc(sizeof(byte_t), height * width);
	assert(matrix[0]);

	// Seteaza referintele catre randuri

	for (int i = 1; i < height; i++)
		matrix[i] = matrix[i - 1] + width;
	return matrix;
}

// Elibereaza din memorie o matrice de octeti
static void __destroy_matrix(byte_t **matrix)
{
	free(matrix[0]);
	free(matrix);
}

// Elibereaza memoria utilizata de pixelii imaginii
void discard_image(image_t *image)
{
	// Verifica daca se afla o imagine in memorie
	if (!IS_LOADED(image))
		return;

	// Si daca aceasta e color sau in tonuri de gri
	if (IS_COLOR(image)) {
		free(image->RGB[0]);
		free(image->RGB);
		image->RGB = NULL;
	} else {
		free(image->Y[0]);
		free(image->Y);
		image->Y = NULL;
	}

	image->maxval = 0;
}

// Aloca memorie pentru pixelii imaginii. Dimensiunile sunt plasate in
// cadrul campurilor structurii image.
static void __allocate_img(image_t *image, int is_color)
{
	int number_of_pixels = image->width * image->height;

	// Se realizeaza doua alocari: una pentru referinte catre inceputurile
	// de rand, si un bloc continuu de memorie pentru toti pixelii imaginii
	if (is_color) {
		image->RGB = (rgb_t **)calloc(image->height, sizeof(rgb_t *));
		assert(image->RGB);

		image->RGB[0] = (rgb_t *)calloc(number_of_pixels, sizeof(rgb_t));
		assert(image->RGB[0]);

		// Se seteaza referintele catre inceputurile de rand
		for (int i = 1; i < image->height; i++)
			image->RGB[i] = image->RGB[i - 1] + image->width;
	} else {
		image->Y = (double **)calloc(image->height, sizeof(double *));
		assert(image->Y);

		image->Y[0] = (double *)calloc(number_of_pixels, sizeof(double));
		assert(image->Y[0]);

		// Se seteaza referintele catre inceputurile de rand
		for (int i = 1; i < image->height; i++)
			image->Y[i] = image->Y[i - 1] + image->width;
	}
}

// Citeste valorile pixelilor imaginii
static void __read_img(image_t *image, int is_color, int is_binary, FILE *in)
{
	if (is_binary) {
		// creaza o matrice de octeti unde se vor citi valorile pixelilor
		int pixel_size = is_color ? 3 : 1;
		int number_of_pixels = image->width * image->height;

		// citeste continutul imaginii printr un singur apel la fread
		byte_t **byte_matrix = __get_byte_matrix(image->height,
												 image->width * pixel_size);
		fread(byte_matrix[0], pixel_size, number_of_pixels, in);

		// seteaza pixelii
		if (is_color) {
			for (int i = 0; i < image->height; i++)
				for (int j = 0; j < image->width; j++) {
					image->RGB[i][j].r = (double)byte_matrix[i][3 * j];
					image->RGB[i][j].g = (double)byte_matrix[i][3 * j + 1];
					image->RGB[i][j].b = (double)byte_matrix[i][3 * j + 2];
				}
		} else {
			for (int i = 0; i < image->height; i++)
				for (int j = 0; j < image->width; j++)
					image->Y[i][j] = (double)byte_matrix[i][j];
		}

		// Elibereaza matricea
		__destroy_matrix(byte_matrix);
	} else {
		// Citeste, element cu element, valorile pixelilor

		for (int i = 0; i < image->height; i++)
			for (int j = 0; j < image->width; j++)
				if (is_color) {
					fscanf(in, "%lf", &image->RGB[i][j].r);
					fscanf(in, "%lf", &image->RGB[i][j].g);
					fscanf(in, "%lf", &image->RGB[i][j].b);
				} else {
					fscanf(in, "%lf", &image->Y[i][j]);
				}
	}
}

// Incarca o imagine in memorie. Intoarce 0 daca operatia nu a intampinat
// dificultati.
int load_image(image_t *image, char *path)
{
	// Elibereaza din memorie vechea imagine
	discard_image(image);

	FILE *in = fopen(path, "rb");

	if (!in)
		return 1;

	char buffer[LINE_SIZE] = {0};
	// Verifica magic number-ul

	fgets(buffer, LINE_SIZE, in);
	int magic_no = buffer[1] - '0';
	int is_color = magic_no == 3 || magic_no == 6;
	int is_binary = magic_no > 3;

	// Citeste dimensiunile
	fgets(buffer, LINE_SIZE, in);
	assert(sscanf(buffer, "%d%d", &image->width, &image->height) == 2);

	// Citeste valoarea maxima a unui pixel
	fgets(buffer, LINE_SIZE, in);
	assert(sscanf(buffer, "%d", &image->maxval) == 1);

	// Aloca memoria pentru pixelii imaginii
	__allocate_img(image, is_color);

	// Realizeaza citirea pixelilor;
	__read_img(image, is_color, is_binary, in);
	fclose(in);
	return 0;
}

// Scrie valorile pixelilor din imaginea incarcata in fisierul
// pasat ca parametru, in formatul cerut
static void __write_pixels(image_t *image, int ascii, FILE *out)
{
	if (ascii) {
		if (IS_COLOR(image)) {
			// Scrie in format ASCII, pentru fiecare pixel,
			// valoarea fiecarui canal
			for (int i = 0; i < image->height; i++) {
				for (int j = 0; j < image->width; j++) {
					rgb_t pixel = image->RGB[i][j];
					fprintf(out, "%hhu %hhu %hhu ", (byte_t)lround(pixel.r),
							(byte_t)lround(pixel.g), (byte_t)lround(pixel.b));
				}

				// Separa liniile
				fprintf(out, "\n");
			}
		} else {
			// Scrie luminozitatea fiecarui pixel in format ascii
			for (int i = 0; i < image->height; i++) {
				for (int j = 0; j < image->width; j++)
					fprintf(out, "%hhu ", (byte_t)lround(image->Y[i][j]));

				// Separator pt linii
				fprintf(out, "\n");
			}
		}
	} else {
		int pix_size = IS_COLOR(image) ? 3 : 1;

		// Stocheaza valorile pixelilor intr-o matrice pt a efectua un singur
		// apel la fwrite
		byte_t **byte_matrix = __get_byte_matrix(image->height,
												 image->width * pix_size);
		if (IS_COLOR(image)) {
			for (int i = 0; i < image->height; i++)
				for (int j = 0; j < image->width; j++) {
					byte_matrix[i][3 * j] = lround(image->RGB[i][j].r);
					byte_matrix[i][3 * j + 1] = lround(image->RGB[i][j].g);
					byte_matrix[i][3 * j + 2] = lround(image->RGB[i][j].b);
				}
		} else {
			for (int i = 0; i < image->height; i++)
				for (int j = 0; j < image->width; j++)
					byte_matrix[i][j] = lround(image->Y[i][j]);
		}

		// Scrie continutul imaginii in fisierul de iesire
		fwrite(byte_matrix[0], pix_size, image->width * image->height, out);
		__destroy_matrix(byte_matrix);
	}
}

// Salveaza imaginea din memorie pe disc, cu numele si formatul precizat
void save_image(image_t *image, char *path, int ascii)
{
	FILE *out = fopen(path, "wb");
	assert(out);

	int is_color = IS_COLOR(image);

	// Calculeaza numarul magic
	int magic_no;

	if (ascii && is_color)
		magic_no = 3;
	else if (ascii && !is_color)
		magic_no = 2;
	else if (!ascii && is_color)
		magic_no = 6;
	else
		magic_no = 5;

	// Scrie randul cu numarul magic
	fprintf(out, "P%d\n", magic_no);

	// Scrie dimensiunile
	fprintf(out, "%d %d\n", image->width, image->height);

	// Scrie valoarea maxima a unui pixel
	fprintf(out, "%d\n", image->maxval);

	// Scrie valorile pixelilor
	__write_pixels(image, ascii, out);
	fclose(out);
}
