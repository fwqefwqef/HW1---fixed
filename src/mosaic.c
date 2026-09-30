#include "pngparser.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Average the block whose top-left corner is (bx, by). Boundary blocks only
 * use the pixels that fall inside the image. */
struct pixel average_block(int w, int h, struct pixel (*data)[w], int bx,
                           int by, int block) {
  uint8_t sum_r = 0, sum_g = 0, sum_b = 0, sum_a = 0;
  int count = 0;
  for (int dy = 0; dy < block && by + dy < h; dy++) {
    for (int dx = 0; dx < block && bx + dx < w; dx++) {
      sum_r += data[by + dy][bx + dx].red;
      sum_g += data[by + dy][bx + dx].green;
      sum_b += data[by + dy][bx + dx].blue;
      sum_a += data[by + dy][bx + dx].alpha;
      count++;
    }
  }
  struct pixel avg = {sum_r / count, sum_g / count, sum_b / count,
                      sum_a / count};
  return avg;
}

int main(int argc, char *argv[]) {
  struct image *img = NULL;
  struct image *img_out = NULL;

  /* mosaic input_image output_image block_size */
  if (argc != 4) {
    goto error_usage;
  }

  /* Rename the arguments for easier reference */
  char *input = argv[1];
  char *output = argv[2];

  /* Size in pixels of one mosaic block */
  uint8_t block = atoi(argv[3]);
  if (block == 0) {
    goto error_usage;
  }

  if (load_png(input, &img)) {
    return 1;
  }

  /* Memory allocation and error handling */
  img_out = malloc(sizeof(struct image));
  img_out->size_x = img->size_x;
  img_out->size_y = img->size_y;
  img_out->px =
      malloc((size_t)img->size_x * img->size_y * sizeof(struct pixel));

  if (img->size_x > 0 && img->size_y > 0) {
    /* Cast both pixel arrays into 2D arrays */
    struct pixel(*data)[img->size_x] = (struct pixel(*)[img->size_x])img->px;
    struct pixel(*dst)[img->size_x] = (struct pixel(*)[img->size_x])img_out->px;

    /* We walk the image block by block */
    for (int by = 0; by < img->size_y; by += block) {
      for (int bx = 0; bx < img->size_x; bx += block) {
        /* Average the block and paint it back */
        struct pixel avg =
            average_block(img->size_x, img->size_y, data, bx, by, block);
        for (int dy = 0; dy < block; dy++) {
          for (int dx = 0; dx < block; dx++) {
            dst[by + dy][bx + dx] = avg;
          }
        }
      }
    }
  }

  if (store_png(output, img_out, NULL, 0)) {
    goto error_store;
  }

  free(img_out->px);
  free(img_out);
  free(img->px);
  free(img);
  return 0;

error_store:
  free(img_out->px);
  free(img_out);
  free(img->px);
  free(img);
  printf("Couldn't write output image\n");
  return 1;

error_usage:
  printf("Usage: %s input_image output_image block_size\n", argv[0]);
  return 1;
}
