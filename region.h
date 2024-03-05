#ifndef _REGION_H
#define _REGION_H

typedef struct region_t {
	int x1, x2, y1, y2;
} region_t;

#define REGION_INIT(region) ((void)memset((region), 0, sizeof(region_t)))

#endif
