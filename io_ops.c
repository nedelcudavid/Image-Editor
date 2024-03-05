#include "io_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Selecteaza intreaga imagine
void __select_all(image_t *image, region_t *region)
{
	region->x1 = 0;
	region->y1 = 0;
	region->x2 = image->width;
	region->y2 = image->height;
}

void load(image_t *image, parser_t *parser, region_t *region)
{
	if (parser->argc != 2) {
		printf("Invalid command\n");
		return;
	}

	char *path = parser->argv[1];

	// Incearca incarcarea imaginii cu numele dat ca parametru
	int status = load_image(image, path);

	if (status)
		printf("Failed to load %s\n", path);
	else
		printf("Loaded %s\n", path);
	__select_all(image, region);
}

void save(image_t *image, parser_t *parser)
{
	if (!IS_LOADED(image)) {
		printf("No image loaded\n");
		return;
	}

	// Afla formatul de fisier cerut (ascii / binar)
	if (parser->argc != 2) {
		if (parser->argc != 3 || strcmp(parser->argv[2], "ascii")) {
			printf("Invalid command\n");
			return;
		}
	}

	char *path = parser->argv[1];
	int ascii = parser->argc == 3 && !strcmp(parser->argv[2], "ascii");

	// Salveaza imaginea in formatul cerut
	save_image(image, path, ascii);

	printf("Saved %s\n", path);
}

// Determina daca stringul este un numar
static int __is_number(char *str)
{
	// Atoi intoarce 0 pentru stringuri care nu sunt numere, sau pt numarul 0
	return atoi(str) != 0 || str[0] == '0';
}

// Verifica daca coordonatele selectiei sunt inauntrul imaginii
// si definesc o arie nenula
// Coordonatele sunt in ordinea x1, y1, x2, y2
static int __check_coords(image_t *image, int *coords)
{
	if (coords[0] < 0 || coords[0] > image->width)
		return 0;
	if (coords[2] < 0 || coords[2] > image->width)
		return 0;
	if (coords[1] < 0 || coords[1] > image->height)
		return 0;
	if (coords[3] < 0 || coords[3] > image->height)
		return 0;
	if (coords[0] == coords[2] || coords[1] == coords[3])
		return 0;
	return 1;
}

void selection(image_t *image, parser_t *parser, region_t *region)
{
	// Verifica daca imaginea e incarcata, daca se doreste selectarea intregii
	// imagini si daca numarul de argumente e corect.
	if (!IS_LOADED(image)) {
		printf("No image loaded\n");
		return;
	} else if (parser->argc == 2 && !strcmp(parser->argv[1], "ALL")) {
		__select_all(image, region);
		printf("Selected ALL\n");
		return;
	} else if (parser->argc != 5) {
		printf("Invalid command\n");
		return;
	}

	// Verifica daca toate cele 4 argumente sunt numere
	for (int i = 1; i <= 4; i++) {
		if (!__is_number(parser->argv[i])) {
			printf("Invalid command\n");
			return;
		}
	}

	int coords[4] = {0};

	for (int i = 0; i < 4; i++)
		coords[i] = atoi(parser->argv[i + 1]);

	// Verifica daca coordonatele sunt valide
	if (!__check_coords(image, coords)) {
		printf("Invalid set of coordinates\n");
		return;
	}

	region->x1 = atoi(parser->argv[1]);
	region->y1 = atoi(parser->argv[2]);
	region->x2 = atoi(parser->argv[3]);
	region->y2 = atoi(parser->argv[4]);

	// Ordoneaza crescator coordonatele
	if (region->x2 < region->x1) {
		int aux = region->x1;
		region->x1 = region->x2;
		region->x2 = aux;
	}
	if (region->y2 < region->y1) {
		int aux = region->y2;
		region->y2 = region->y1;
		region->y1 = aux;
	}

	printf("Selected %d %d ", region->x1, region->y1);
	printf("%d %d\n", region->x2, region->y2);
}
