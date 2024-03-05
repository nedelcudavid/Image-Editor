#ifndef _IO_OPS_H
#define _IO_OPS_H

#include "pbmIO.h"
#include "parser.h"
#include "region.h"

void load(image_t *image, parser_t *parser, region_t *region);

void save(image_t *image, parser_t *parser);

void selection(image_t *image, parser_t *parser, region_t *region);

void __select_all(image_t *image, region_t *region);

#endif
