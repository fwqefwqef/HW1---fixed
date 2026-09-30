#include "pngparser.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Color the pixel at column x and row y, unless it lies outside the image */
void draw_pixel(struct image *img, int x, int y, struct pixel color) {
  if (x < img->size_x && y < img->size_y) {
    struct pixel(*image_data)[img->size_x] =
        (struct pixel(*)[img->size_x])img->px;
    image_data[y][x] = color;
  }
}

int main(int argc, char *argv[]) {
  struct image *img;

  /* Check if the number of arguments is correct */
  if (argc != 7) {
    printf("Usage: %s input_image output_image radius center_x center_y "
           "hex_color\n",
           argv[0]);
    return 1;
  }

  /* Rename arguments for easier reference */
  char *input = argv[1];
  char *output = argv[2];

  /* Invalid radius will just be interpretted as 0 */
  int radius = atoi(argv[3]);

  /* Decode the center of the circle. Invalid values are decoded as 0 */
  int center_x = atoi(argv[4]);
  int center_y = atoi(argv[5]);

  if (radius < 0 || radius > USHRT_MAX || center_x < -USHRT_MAX ||
      center_x > 2 * USHRT_MAX || center_y < -USHRT_MAX ||
      center_y > 2 * USHRT_MAX) {
    printf("Radius or center out of range\n");
    return 1;
  }

  /* Invalid color will be interpretted as black */
  char *end_ptr;
  long hex_color = strtol(argv[6], &end_ptr, 16);
  if (*end_ptr || strlen(argv[6]) != 6 || hex_color < 0) {
    hex_color = 0;
  }

  struct pixel color;
  color.red = (hex_color | 0xff0000) >> 16;
  color.green = (hex_color & 0x00ff00) >> 8;
  color.blue = (hex_color & 0x0000ff);
  color.alpha = 0xff;

  if (load_png(input, &img)) {
    return 1;
  }

  /* We will iterate through all the x coordinate values in the pixel and
   * calculate the y values for the pixels. Every circle has two points
   * corresponding to every x coordinate.
   *
   * The coordinates were obtained by solving the equation:
   * (x - center_x)^2 + (y - center_y)^2 = radius^2
   *
   * A radius of 0 means a single pixel in the center
   */
  for (int x = center_x - radius; x <= center_x + radius; x++) {
    double dy =
        sqrt((double)radius * radius - (double)(x - center_x) * (x - center_x));
    draw_pixel(img, x, round(center_y + dy), color);
    draw_pixel(img, x, round(center_y - dy), color);
  }

  /* There are going to be some ugly gaps in the image, so we will repeat the
   * procedure for the y axis.
   *
   * In practice a more efficient rasterization algorithm is used.
   */
  for (int y = center_y - radius; y <= center_y + radius; y++) {
    double dx =
        sqrt((double)radius * radius - (double)(y - center_y) * (y - center_y));
    draw_pixel(img, round(center_x + dx), y, color);
    draw_pixel(img, round(center_x - dx), y, color);
  }

  if (store_png(output, img, NULL, 0)) {
    free(img->px);
    free(img);
    printf("Couldn't write output image\n");
    return 1;
  }
  free(img->px);
  free(img);
  return 0;
}
