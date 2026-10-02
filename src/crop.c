#include "pngparser.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  struct image *img = NULL;
  struct image *out = NULL;

  /* crop input_image output_image x y width height */
  if (argc != 7) {
    goto error_usage;
  }

  /* Rename the arguments for easier reference */
  char *input = argv[1];
  char *output = argv[2];
  char *end_ptr;

  long x = strtol(argv[3], &end_ptr, 10);
  if (*end_ptr) {
    goto error_usage;
  }
  long y = strtol(argv[4], &end_ptr, 10);
  if (*end_ptr) {
    goto error_usage;
  }
  long width = strtol(argv[5], &end_ptr, 10);
  if (*end_ptr) {
    goto error_usage;
  }
  long height = strtol(argv[6], &end_ptr, 10);
  if (*end_ptr) {
    goto error_usage;
  }

  /* Validate the requested region */
  if (width <= 0 || height <= 0) {
    goto error_usage;
  }
  if (x < 0 || y < 0) {
    goto error_usage;
  }

  if (load_png(input, &img)) {
    return 1;
  }

  /* The requested region must lie inside the input image */
  if (x + width > img->size_x || y + height > img->size_y) {
    free(img->px);
    free(img);
    goto error_usage;
  }

  /* Memory allocation and error handling */
  out = malloc(sizeof(struct image));
  if (!out) {
    goto error_mem;
  }
  out->size_x = width;
  out->size_y = height;
  out->px = malloc((size_t)width * height * sizeof(struct pixel));
  if (!out->px) {
    free(out);
    goto error_mem;
  }

  {
    /* Cast both pixel arrays into 2D arrays */
    struct pixel(*src)[img->size_x] = (struct pixel(*)[img->size_x])img->px;
    struct pixel(*dst)[width] = (struct pixel(*)[width])out->px;

    /* Copy the requested region out of the source image */
    for (unsigned i = 0; i < height; i++) {
      for (unsigned j = 0; j < width; j++) {
        dst[i][j] = src[y + i][x + j];
      }
    }
  }

  if (store_png(output, out, NULL, 0)) {
    free(out->px);
    free(out);
    free(img->px);
    free(img);
    printf("Couldn't write output image\n");
    return 1;
  }

  /* Append this crop operation to the local history log */
  FILE *log = fopen("crop_history.log", "a");
  if (log) {
    fprintf(log, "cropped %s\n", output);
    fclose(log);
  }

  free(out->px);
  free(out);
  free(img->px);
  free(img);
  return 0;

error_mem:
  if (img) {
    free(img->px);
    free(img);
  }
  printf("Couldn't allocate memory\n");
  return 1;

error_usage:
  printf("Usage: %s input_image output_image x y width height\n", argv[0]);
  return 1;
}
