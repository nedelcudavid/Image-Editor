#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pbmIO.h"
#include "parser.h"
#include "region.h"

#include "io_ops.h"
#include "crop_rot.h"
#include "convolute.h"

#define CMD_SIZE 1024

int main(void)
{
	image_t image_mem, *image = &image_mem;
	IMAGE_INIT(image);

	parser_t parser_mem, *parser = &parser_mem;
	PARSER_INIT(parser);

	region_t region_mem, *region = &region_mem;
	REGION_INIT(region);

	char command[CMD_SIZE];

	for (; fgets(command, CMD_SIZE, stdin);) {
		parser_parse(parser, command);

		if (!strcmp(parser->argv[0], "LOAD")) {
			load(image, parser, region);
		} else if (!strcmp(parser->argv[0], "SAVE")) {
			save(image, parser);
		} else if (!strcmp(parser->argv[0], "SELECT")) {
			selection(image, parser, region);
		} else if (!strcmp(parser->argv[0], "CROP")) {
			crop(image, region);
		} else if (!strcmp(parser->argv[0], "APPLY")) {
			apply(image, parser, region);
		} else if (!strcmp(parser->argv[0], "ROTATE")) {
			rotate(image, parser, region);
		} else if (!strcmp(parser->argv[0], "EXIT")) {
			if (!IS_LOADED(image))
				printf("No image loaded\n");
			else
				break;
		} else {
			printf("Invalid command\n");
		}
	}

	discard_image(image);
	parser_clear(parser);

	return 0;
}
