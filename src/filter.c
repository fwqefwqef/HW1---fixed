#include "filter.h"
#include "pngparser.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARG_SIZE 255

/* Build the gray version of one pixel */
struct pixel *gray_pixel(double luminosity) {
  struct pixel p;

  /* Keep the value inside the range of a color channel */
  if (!(luminosity > 0)) {
    luminosity = 0;
  } else if (luminosity > 255) {
    luminosity = 255;
  }

  p.red = p.green = p.blue = (uint8_t)luminosity;
  return &p;
}

/* This filter iterates over the image and calculates the weighted luminosity of
 * the color channels for every pixel. This value is then written to all the
 * color channels to get the grayscale representation of the image. */
void filter_grayscale(struct image *img, void *weight_arr) {
  if (img->size_x == 0 || img->size_y == 0) {
    return;
  }

  struct pixel(*image_data)[img->size_x] =
      (struct pixel(*)[img->size_x])img->px;
  double *weights = (double *)weight_arr;

  for (unsigned short i = 0; i < img->size_y; i++) {
    for (unsigned short j = 0; j < img->size_x; j++) {
      double luminosity = 0;

      luminosity += weights[0] * image_data[i][j].red;
      luminosity += weights[1] * image_data[i][j].green;
      luminosity += weights[2] * image_data[i][j].blue;

      struct pixel *g = gray_pixel(luminosity);
      image_data[i][j].red = g->red;
      image_data[i][j].green = g->green;
      image_data[i][j].blue = g->blue;
      /* The alpha channel is left unchanged */
    }
  }
}

/* This filter blurs an image. For every pixel we define a square of side
 * 2*radius+1 centered around it. The new value of the pixel is the average
 * value of all pixels in the square. Pixels of the square which fall outside
 * the image do not count towards the average. */
void filter_blur(struct image *img, void *r) {
  if (img->size_x == 0 || img->size_y == 0) {
    return;
  }

  struct pixel(*image_data)[img->size_x] =
      (struct pixel(*)[img->size_x])img->px;
  int radius = *((int *)r);
  if (radius < 0) {
    radius = 0;
  }

  struct pixel(*new_data)[img->size_x] =
      malloc(sizeof(struct pixel) * img->size_x * img->size_y);
  if (!new_data) {
    return;
  }

  for (long i = 0; i < img->size_y; i++) {
    for (long j = 0; j < img->size_x; j++) {

      unsigned long red = 0, green = 0, blue = 0, alpha = 0;
      unsigned long count = 0;

      /* Rows and columns covered by the square centered on (i, j) */
      long y_min = i - radius;
      long y_max = i + radius;
      long x_min = j - radius;
      long x_max = j + radius;

      /* BUG! This bug is an example and is NOT graded.
       * FIX: clip y_min/y_max to [0, size_y - 1] and x_min/x_max to
       * [0, size_x - 1] before the loops, so that only pixels inside the
       * image are read (and counted).
       */
      for (long y = y_min; y <= y_max; y++) {
        for (long x = x_min; x <= x_max; x++) {
          struct pixel current = image_data[y][x];

          red += current.red;
          blue += current.blue;
          green += current.green;
          alpha += current.alpha;
          count++;
        }
      }

      new_data[i][j].red = red / count;
      new_data[i][j].green = green / count;
      new_data[i][j].blue = blue / count;
      new_data[i][j].alpha = alpha / count;
    }
  }

  memcpy(img->px, new_data, sizeof(struct pixel) * img->size_x * img->size_y);
  free(new_data);
}

/* This filter just negates every color in the image */
void filter_negative(struct image *img, void *noarg) {
  if (img->size_x == 0 || img->size_y == 0) {
    return;
  }

  struct pixel(*image_data)[img->size_x] =
      (struct pixel(*)[img->size_x])img->px;

  for (long i = 0; i < img->size_y; i++) {
    for (long j = 0; j < img->size_x; j++) {
      struct pixel current = image_data[i][j];
      struct pixel neg;
      neg.red = 255 - current.red;
      neg.green = 255 - current.green;
      neg.blue = 255 - current.blue;
      neg.alpha = current.alpha;
      image_data[i][j] = neg;
    }
  }
}

/* Set the transparency of the picture to the value (0-255) passed as
 * argument */
void filter_transparency(struct image *img, void *transparency) {
  if (img->size_x == 0 || img->size_y == 0) {
    return;
  }

  struct pixel(*image_data)[img->size_x] =
      (struct pixel(*)[img->size_x])img->px;
  uint8_t local_alpha = *((uint8_t *)transparency);

  for (long i = 0; i < img->size_y - 1; i++) {
    for (long j = 0; j < img->size_x; j++) {
      image_data[i][j].alpha = local_alpha;
    }
  }
}

/* This filter sharpens an image using a 3x3 convolution.
 *
 * See HW1.pdf for the exact specification. In short:
 *   - kernel {{0,-1,0},{-1,5,-1},{0,-1,0}}, applied to red, green and blue,
 *   - every neighbour is read from the original (unmodified) pixels,
 *   - a neighbour index outside the image is clamped to the nearest edge,
 *   - every result is clamped to [0,255]; alpha is copied unchanged,
 *   - an empty image is left untouched; when the function returns, img->px
 *     is a heap buffer holding the result.
 */
void filter_sharpen(struct image *img, void *unused) {
  (void)img;
  (void)unused;
  /* TODO: Implement */
}

/* The filter structure comprises the filter function, its arguments and the
 * image we want to process */
struct filter {
  void (*filter)(struct image *img, void *arg);
  void *arg;
  struct image *img;
};

void execute_filter(struct filter *fil) { fil->filter(fil->img, fil->arg); }

int __attribute__((weak)) main(int argc, char *argv[]) {
  struct filter fil;
  char input[ARG_SIZE] = "";
  int radius;
  uint8_t alpha;
  struct image *img = NULL;
  double weights[] = {0.299, 0.587, 0.114};

  /* Some filters take no arguments, while others have 1 */
  if (argc != 4 && argc != 5) {
    goto error_usage;
  }

  fil.filter = NULL;
  fil.arg = NULL;

  char *output = argv[2];
  char *command = argv[3];

  /* Copy the input filename for easier reference */
  strncat(input, argv[1], sizeof(input));

  /* Error when loading a png image */
  if (load_png(input, &img)) {
    printf("%s PNG file cannot be loaded\n", input);
    exit(1);
  }

  fil.img = img;

  /* Decode the filter */
  if (!strcmp(command, "grayscale")) {
    fil.filter = filter_grayscale;
    fil.arg = weights;
  } else if (!strcmp(command, "negative")) {
    fil.arg = NULL;
    fil.filter = filter_negative;
  } else if (!strcmp(command, "blur")) {
    /* Bad filter radius will just be interpretted as 0 */
    radius = argv[4] ? atoi(argv[4]) : 0;
    fil.filter = filter_blur;
    fil.arg = &radius;
  } else if (!strcmp(command, "alpha")) {
    char *end_ptr;
    long tmp_alpha = argv[4] ? strtol(argv[4], &end_ptr, 16) : -1;
    if (!argv[4] || end_ptr == argv[4] || *end_ptr || tmp_alpha < 0 ||
        tmp_alpha > 255) {
      goto error_filter;
    }
    alpha = tmp_alpha;
    fil.filter = filter_transparency;
    fil.arg = &alpha;
  } else if (!strcmp(command, "sharpen")) {
    fil.filter = filter_sharpen;
    fil.arg = NULL;
  }

  if (fil.filter) {
    execute_filter(&fil);
  } else {
    goto error_filter;
  }

  if (store_png(output, img, NULL, 0)) {
    goto error_filter;
  }

  printf("Wrote ");
  printf(output);
  printf("\n");

  /* Print a short summary line after writing the output */
  char summary[64];
  strcpy(summary, output);
  strcat(summary, ": done");
  puts(summary);

  free(img->px);
  free(img);
  return 0;

error_filter:
  free(img->px);
  free(img);
error_usage:
  printf("Usage: %s input_image output_image filter_name [filter_arg]\n",
         argv[0]);
  printf("Filters:\n");
  printf("grayscale\n");
  printf("negative\n");
  printf("blur radius_arg\n");
  printf("alpha hex_alpha\n");
  printf("sharpen\n");
  return 1;
}
