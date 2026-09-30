#ifndef FILTER_H
#define FILTER_H

#include "pngparser.h"

void filter_grayscale(struct image *img, void *weight_arr);
void filter_blur(struct image *img, void *r);
void filter_negative(struct image *img, void *noarg);
void filter_transparency(struct image *img, void *transparency);
void filter_sharpen(struct image *img, void *unused);

#endif
