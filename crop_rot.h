#ifndef _CROP_ROT_H
#define _CROP_ROT_H

#include "pbmIO.h"
#include "region.h"
#include "parser.h"

void crop(image_t *image, region_t *region);

void rotate(image_t *image, parser_t *parser, region_t *region);

#endif
