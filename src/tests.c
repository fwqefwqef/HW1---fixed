/* Unit tests for the Y0L0 PNG filter library.
 *
 * Four tests are already written for you and serve as examples:
 * gray_weight_extremes, gray_zero_weights, sharpen_tiny_image and
 * sharpen_checkerboard. They are not graded.
 *
 * The eight tests marked "TODO: Implement" are graded, one point each:
 * gray_reference_images, invert_roundtrip, invert_empty_image,
 * blur_small_kernel, blur_radius_limits, alpha_overwrite,
 * alpha_null_argument and sharpen_reference_images.
 *
 * All of them are already registered in main(); you only have to fill in the
 * bodies (and the radius list that blur_radius_limits loops over).
 *
 * Build and run with:
 *   make tests && ./tests
 * Use CK_FORK=no ./tests when you want to run the suite under a debugger.
 * Note that CK_FORK=no makes a crashing test kill the whole suite, so
 * alpha_null_argument only works in the default (forking) mode.
 */
#include "filter.h"
#include <assert.h>
#include <check.h>
#include <float.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* Allocate an image of the requested size and fill it with random pixels.
 * Both sizes have to be larger than 0. */
struct image generate_rand_img_sized(uint16_t size_x, uint16_t size_y) {
  struct image img;

  img.size_x = size_x;
  img.size_y = size_y;
  img.px = malloc(img.size_x * img.size_y * sizeof(struct pixel));
  if (img.px == NULL)
    assert(0 && "Rerun test, malloc failed");
  for (long i = 0; i < img.size_y * img.size_x; i++) {
    img.px[i].red = rand();
    img.px[i].green = rand();
    img.px[i].blue = rand();
    img.px[i].alpha = rand();
  }

  return img;
}

struct image generate_rand_img() {
  uint16_t size_x, size_y;

  do {
    size_x = rand() % 128;
  } while (size_x == 0);
  do {
    size_y = rand() % 128;
  } while (size_y == 0);

  return generate_rand_img_sized(size_x, size_y);
}

struct image duplicate_img(struct image img) {
  struct image img_dup;

  img_dup.size_x = img.size_x;
  img_dup.size_y = img.size_y;
  img_dup.px = malloc(img.size_x * img.size_y * sizeof(struct pixel));
  if (img_dup.px == NULL)
    assert(0 && "Rerun test, malloc failed");
  for (long i = 0; i < img.size_y * img.size_x; i++) {
    img_dup.px[i].red = img.px[i].red;
    img_dup.px[i].green = img.px[i].green;
    img_dup.px[i].blue = img.px[i].blue;
    img_dup.px[i].alpha = img.px[i].alpha;
  }

  return img_dup;
}

/* Grayscale function should not crash when the weights are
 * at the limits of the double data size */
START_TEST(gray_weight_extremes) {
  srand(time(NULL) ^ getpid());

  /* Generate random png image */
  struct image img = generate_rand_img();

  /* Limiting cases of double */
  double weight_limits[] = {DBL_MIN, DBL_TRUE_MIN, DBL_MAX, DBL_EPSILON,
                            DBL_MIN_EXP};
  int n_limits = sizeof(weight_limits) / sizeof(weight_limits[0]);

  /* grayscale requires array of 3 weights */
  for (unsigned i0 = 0; i0 < n_limits; i0++)
    for (unsigned i1 = 0; i1 < n_limits; i1++)
      for (unsigned i2 = 0; i2 < n_limits; i2++) {
        double weights[3] = {weight_limits[i0], weight_limits[i1],
                             weight_limits[i2]};
        /* Merely checking that the function does not crash */
        filter_grayscale(&img, weights);
      }

  /* Not strictly necessary unless using CK_FORK=no */
  free(img.px);
}
END_TEST

/* With all three weights set to 0 every color channel has to become 0,
 * while the alpha channel keeps its value */
START_TEST(gray_zero_weights) {
  srand(time(NULL) ^ getpid());

  /* Generate random png image */
  struct image img = generate_rand_img();
  uint8_t rand_alpha = rand();
  double weights[] = {0, 0, 0};
  uint16_t sz_x = img.size_x, sz_y = img.size_y;
  for (long i = 0; i < sz_y; i++)
    for (long j = 0; j < sz_x; j++)
      img.px[i * sz_x + j].alpha = rand_alpha;

  filter_grayscale(&img, weights);

  ck_assert_uint_eq(img.size_x, sz_x);
  ck_assert_uint_eq(img.size_y, sz_y);
  ck_assert_ptr_ne(img.px, NULL);
  for (long i = 0; i < sz_y; i++)
    for (long j = 0; j < sz_x; j++) {
      long idx = i * sz_x + j;
      ck_assert_uint_eq(img.px[idx].red, 0);
      ck_assert_uint_eq(img.px[idx].green, 0);
      ck_assert_uint_eq(img.px[idx].blue, 0);
      ck_assert_uint_eq(img.px[idx].alpha, rand_alpha);
    }

  free(img.px);
}
END_TEST

/* A 1x1 image is a fixed point of the sharpen kernel: every neighbour is
 * clamped back onto the only pixel there is, and the nine kernel weights add
 * up to 1. So the filter must leave this image untouched, and must not read
 * or write outside of it.
 *
 * The pixel lives on the heap, not on the stack: filter_sharpen is allowed to
 * replace img.px with a buffer of its own, so we also re-read img.px after
 * the call instead of looking at the pointer we passed in. */
START_TEST(sharpen_tiny_image) {
  struct pixel *px = malloc(sizeof(struct pixel));
  if (px == NULL)
    assert(0 && "Rerun test, malloc failed");
  px->red = 0x12;
  px->green = 0x34;
  px->blue = 0x56;
  px->alpha = 0x78;
  struct image img = {1, 1, px};

  filter_sharpen(&img, NULL);

  ck_assert_uint_eq(img.size_x, 1);
  ck_assert_uint_eq(img.size_y, 1);
  ck_assert_ptr_ne(img.px, NULL);
  ck_assert_uint_eq(img.px[0].red, 0x12);
  ck_assert_uint_eq(img.px[0].green, 0x34);
  ck_assert_uint_eq(img.px[0].blue, 0x56);
  ck_assert_uint_eq(img.px[0].alpha, 0x78);

  free(img.px);
}
END_TEST

/* Sharpening the checkerboard image has to reproduce the reference image
 * exactly, while every alpha channel keeps the value of the source image */
START_TEST(sharpen_checkerboard) {
  struct image *img, *img_sharp, img_dup;

  ck_assert_int_eq(load_png("test_imgs/ck.png", &img), 0);
  img_dup = duplicate_img(*img);
  filter_sharpen(img, NULL);

  /* Compare to known good image */
  ck_assert_int_eq(load_png("test_imgs/ck_sharpen.png", &img_sharp), 0);

  ck_assert_uint_eq(img_sharp->size_x, img->size_x);
  ck_assert_uint_eq(img_sharp->size_y, img->size_y);
  ck_assert_ptr_ne(img->px, NULL);
  for (long j = 0; j < (long)img->size_x * img->size_y; j++) {
    ck_assert_uint_eq(img_sharp->px[j].red, img->px[j].red);
    ck_assert_uint_eq(img_sharp->px[j].green, img->px[j].green);
    ck_assert_uint_eq(img_sharp->px[j].blue, img->px[j].blue);
    ck_assert_uint_eq(img_dup.px[j].alpha, img->px[j].alpha);
  }
  free(img_dup.px);
  free(img_sharp->px);
  free(img->px);
  free(img_sharp);
  free(img);
}
END_TEST

char *gray_sources[] = {"test_imgs/desert.png", "test_imgs/summer.png"};
char *gray_expected[] = {"test_imgs/desert_gray.png",
                         "test_imgs/summer_gray.png"};

/* The reference images were generated with the same weights that filter.c
 * passes to the filter, the ITU-R BT.601 luma coefficients
 * {0.299, 0.587, 0.114}.
 *
 * TODO: Implement
 * For the image pair with index _i:
 *   - load gray_sources[_i] and keep a copy of it (duplicate_img),
 *   - apply filter_grayscale with the weights above,
 *   - load gray_expected[_i] and check that the dimensions are unchanged,
 *     that every red/green/blue channel matches the reference image, and
 *     that every alpha channel still holds the value of the source image,
 *   - free every image and every pixel buffer you allocated or loaded. */
START_TEST(gray_reference_images) {
  double weights[] = {0.299, 0.587, 0.114};

  struct image *img, *expected;
  ck_assert_int_eq(load_png(gray_sources[_i], &img), 0);
  struct image copy = duplicate_img(*img);

  filter_grayscale(img, weights);

  ck_assert_int_eq(load_png(gray_expected[_i], &expected), 0);
  ck_assert_uint_eq(img->size_x, expected->size_x);
  ck_assert_uint_eq(img->size_y, expected->size_y);
  ck_assert_ptr_ne(img->px, NULL);
  for (long j = 0; j < (long)img->size_x * img->size_y; j++) {
    ck_assert_uint_eq(img->px[j].red, expected->px[j].red);
    ck_assert_uint_eq(img->px[j].green, expected->px[j].green);
    ck_assert_uint_eq(img->px[j].blue, expected->px[j].blue);
    ck_assert_uint_eq(img->px[j].alpha, copy.px[j].alpha);
  }

  free(copy.px);
  free(img->px);
  free(img);
  free(expected->px);
  free(expected);
}
END_TEST

/* Verify that the black image is inverted properly to a white image.
 * Then invert the result again and verify that you get a black image back.
 * The alpha channel needs to be intact in both cases.
 *
 * TODO: Implement
 * Build a random-sized image whose pixels are all {0, 0, 0, alpha} for one
 * fixed random alpha, call filter_negative twice, and check the dimensions,
 * the color channels and the alpha channel after each call. */
START_TEST(invert_roundtrip) {
  srand(time(NULL) ^ getpid());

  /* Random-sized image, every pixel black with one fixed random alpha */
  struct image img = generate_rand_img();
  uint8_t alpha = rand();
  uint16_t sz_x = img.size_x, sz_y = img.size_y;
  for (long i = 0; i < (long)sz_x * sz_y; i++) {
    img.px[i].red = 0;
    img.px[i].green = 0;
    img.px[i].blue = 0;
    img.px[i].alpha = alpha;
  }

  /* First inversion: black -> white */
  filter_negative(&img, NULL);
  ck_assert_uint_eq(img.size_x, sz_x);
  ck_assert_uint_eq(img.size_y, sz_y);
  ck_assert_ptr_ne(img.px, NULL);
  for (long i = 0; i < (long)sz_x * sz_y; i++) {
    ck_assert_uint_eq(img.px[i].red, 255);
    ck_assert_uint_eq(img.px[i].green, 255);
    ck_assert_uint_eq(img.px[i].blue, 255);
    ck_assert_uint_eq(img.px[i].alpha, alpha);
  }

  /* Second inversion: white -> black */
  filter_negative(&img, NULL);
  ck_assert_uint_eq(img.size_x, sz_x);
  ck_assert_uint_eq(img.size_y, sz_y);
  for (long i = 0; i < (long)sz_x * sz_y; i++) {
    ck_assert_uint_eq(img.px[i].red, 0);
    ck_assert_uint_eq(img.px[i].green, 0);
    ck_assert_uint_eq(img.px[i].blue, 0);
    ck_assert_uint_eq(img.px[i].alpha, alpha);
  }

  free(img.px);
}
END_TEST

/* Check if the filter doesn't crash when we pass a 0x0 image
 *
 * TODO: Implement
 * Pass an image with size_x == 0 and size_y == 0 (with a valid, but unused,
 * pixel buffer) to filter_negative and check that it returns without
 * touching the image. */
START_TEST(invert_empty_image) {
  /* Valid but unused pixel buffer, 0x0 dimensions */
  struct pixel *px = malloc(sizeof(struct pixel));
  if (px == NULL)
    assert(0 && "Rerun test, malloc failed");
  struct image img = {0, 0, px};

  filter_negative(&img, NULL);

  /* The filter must return without touching the image */
  ck_assert_uint_eq(img.size_x, 0);
  ck_assert_uint_eq(img.size_y, 0);
  ck_assert_ptr_eq(img.px, px);

  free(img.px);
}
END_TEST

/* Check for the simple, non-uniform, 3x3 test image that the blur filter
 * gives the correct output for the radii 0, 1, 2 and 3.
 *
 * The image is one white (180) pixel in the middle of a black 3x3 image, and
 * every alpha channel is 255. The blur filter averages the pixels of a
 * (2*radius+1) square around every pixel, and the parts of that square which
 * fall outside the image neither contribute to the sum nor to the divisor.
 * The expected color channels are therefore:
 *   radius 0 -> the image is unchanged (each square holds one pixel),
 *   radius 1 -> 45 in the four corners (180/4), 30 in the middle of the four
 *               edges (180/6) and 20 in the centre (180/9),
 *   radius 2 -> 20 everywhere (the square always covers the whole image),
 *   radius 3 -> 20 everywhere, same as radius 2.
 * The alpha channel stays 255 everywhere.
 *
 * The pixels live on the heap because filter_blur may replace img.px with a
 * buffer of its own; re-read img.px after every call.
 *
 * TODO: Implement
 * Run the four radii, resetting the image before each one, and check the
 * dimensions, the three color channels and the alpha channel of all 9 pixels
 * against the values above. */
START_TEST(blur_small_kernel) {
  const struct pixel black = {0, 0, 0, 255};
  const struct pixel white = {180, 180, 180, 255};
  struct pixel *px = malloc(9 * sizeof(struct pixel));
  if (px == NULL)
    assert(0 && "Rerun test, malloc failed");
  for (int k = 0; k < 9; k++)
    px[k] = black;
  px[4] = white;
  struct image img = {3, 3, px};

  /* Expected color value (identical for r/g/b) of the 9 pixels, per radius. */
  uint8_t expected[4][9] = {
      /* radius 0: unchanged (each square holds one pixel) */
      {0, 0, 0, 0, 180, 0, 0, 0, 0},
      /* radius 1: corners 45, edge-midpoints 30, centre 20 */
      {45, 30, 45, 30, 20, 30, 45, 30, 45},
      /* radius 2: the square always covers the whole image -> 20 everywhere */
      {20, 20, 20, 20, 20, 20, 20, 20, 20},
      /* radius 3: same as radius 2 */
      {20, 20, 20, 20, 20, 20, 20, 20, 20},
  };

  for (int radius = 0; radius <= 3; radius++) {
    /* Reset the image before each radius (filter_blur may replace img.px). */
    for (int k = 0; k < 9; k++)
      img.px[k] = black;
    img.px[4] = white;

    filter_blur(&img, &radius);

    ck_assert_uint_eq(img.size_x, 3);
    ck_assert_uint_eq(img.size_y, 3);
    ck_assert_ptr_ne(img.px, NULL);
    for (int k = 0; k < 9; k++) {
      ck_assert_uint_eq(img.px[k].red, expected[radius][k]);
      ck_assert_uint_eq(img.px[k].green, expected[radius][k]);
      ck_assert_uint_eq(img.px[k].blue, expected[radius][k]);
      ck_assert_uint_eq(img.px[k].alpha, 255);
    }
  }

  free(img.px);
}
END_TEST

/* Verify that the filter doesn't crash for the extreme radius values listed
 * in main(), and that it leaves the image dimensions alone.
 *
 * TODO: Implement
 * Work on a copy of blur_radius_img (duplicate_img) so that the other
 * iterations of the loop test are not affected, call filter_blur with
 * blur_radii[_i], check the dimensions and that img.px is not NULL, and free
 * the copy. */
struct image blur_radius_img;
int blur_radii[20];
START_TEST(blur_radius_limits) {
  /* Work on a copy so other iterations see the original image. */
  struct image copy = duplicate_img(blur_radius_img);
  int radius = blur_radii[_i];

  filter_blur(&copy, &radius);

  ck_assert_uint_eq(copy.size_x, blur_radius_img.size_x);
  ck_assert_uint_eq(copy.size_y, blur_radius_img.size_y);
  ck_assert_ptr_ne(copy.px, NULL);

  free(copy.px);
}
END_TEST

/* Verify for a random image that the transparency filter works properly
 *
 * TODO: Implement
 * Keep a copy of a random image (duplicate_img), apply filter_transparency
 * with a random alpha value, and check for every single pixel of the image
 * that the alpha channel now holds that value and that the three color
 * channels are unchanged. */
START_TEST(alpha_overwrite) {
  srand(time(NULL) ^ getpid());

  struct image img = generate_rand_img();
  struct image copy = duplicate_img(img);
  uint8_t alpha = rand();
  uint16_t sz_x = img.size_x, sz_y = img.size_y;

  filter_transparency(&img, &alpha);

  ck_assert_uint_eq(img.size_x, sz_x);
  ck_assert_uint_eq(img.size_y, sz_y);
  ck_assert_ptr_ne(img.px, NULL);
  for (long i = 0; i < (long)sz_x * sz_y; i++) {
    ck_assert_uint_eq(img.px[i].alpha, alpha);
    ck_assert_uint_eq(img.px[i].red, copy.px[i].red);
    ck_assert_uint_eq(img.px[i].green, copy.px[i].green);
    ck_assert_uint_eq(img.px[i].blue, copy.px[i].blue);
  }

  free(img.px);
  free(copy.px);
}
END_TEST

/* Check if the function crashes when we pass NULL as the argument.
 *
 * filter_transparency dereferences its argument, so the process is expected
 * to die with SIGSEGV. main() registers this test with
 * tcase_add_test_raise_signal(), which makes Check treat that signal as the
 * expected outcome, so the test body only has to make the call happen.
 * Remember that the filter returns immediately for an empty image, so the
 * image you pass has to contain at least one pixel.
 *
 * Note: run this one with the plain ./tests binary. In ./tests_asan the
 * sanitizer intercepts the fault and exits instead of letting the signal
 * through, and Check then reports the test as failed.
 *
 * TODO: Implement */
START_TEST(alpha_null_argument) {
  /* At least one pixel, so the filter does not return early before it
   * dereferences the (NULL) argument. */
  struct pixel *px = malloc(sizeof(struct pixel));
  if (px == NULL)
    assert(0 && "Rerun test, malloc failed");
  struct image img = {1, 1, px};

  /* Expected to die with SIGSEGV (registered via tcase_add_test_raise_signal)
   */
  filter_transparency(&img, NULL);

  free(img.px);
}
END_TEST

char *sharpen_sources[] = {"test_imgs/desert.png", "test_imgs/summer.png",
                           "test_imgs/sharpen_small_in.png"};
char *sharpen_expected[] = {"test_imgs/desert_sharpen.png",
                            "test_imgs/summer_sharpen.png",
                            "test_imgs/sharpen_small_out.png"};

/* Sharpening every source image has to reproduce the matching reference
 * image, exactly like sharpen_checkerboard does for the checkerboard.
 * test_imgs/sharpen_small_in.png is a 5x4 image, which is small enough to
 * work out by hand when you are debugging your filter_sharpen.
 *
 * TODO: Implement
 * For the image pair with index _i: load sharpen_sources[_i], keep a copy of
 * it, call filter_sharpen, load sharpen_expected[_i] and check the
 * dimensions, the three color channels against the reference image and the
 * alpha channel against the copy. Free everything you allocated or loaded. */
START_TEST(sharpen_reference_images) {
  struct image *img, *img_sharp, img_dup;

  ck_assert_int_eq(load_png(sharpen_sources[_i], &img), 0);
  img_dup = duplicate_img(*img);
  filter_sharpen(img, NULL);

  ck_assert_int_eq(load_png(sharpen_expected[_i], &img_sharp), 0);
  ck_assert_uint_eq(img_sharp->size_x, img->size_x);
  ck_assert_uint_eq(img_sharp->size_y, img->size_y);
  ck_assert_ptr_ne(img->px, NULL);
  for (long j = 0; j < (long)img->size_x * img->size_y; j++) {
    ck_assert_uint_eq(img_sharp->px[j].red, img->px[j].red);
    ck_assert_uint_eq(img_sharp->px[j].green, img->px[j].green);
    ck_assert_uint_eq(img_sharp->px[j].blue, img->px[j].blue);
    ck_assert_uint_eq(img_dup.px[j].alpha, img->px[j].alpha);
  }
  free(img_dup.px);
  free(img_sharp->px);
  free(img->px);
  free(img_sharp);
  free(img);
}
END_TEST

int main() {
  Suite *s = suite_create("lib-Y0l0 tests");
  TCase *tc1 = tcase_create("edge case tests");
  suite_add_tcase(s, tc1);
  TCase *tc2 = tcase_create("basic functionality tests");
  suite_add_tcase(s, tc2);

  /* All twelve tests are registered below; you only fill in the bodies marked
   * TODO above, plus the radius list further down. */

  /* Tests for limits*/
  tcase_add_test(tc1, gray_weight_extremes);
  tcase_add_test(tc1, invert_empty_image);
  tcase_add_test(tc1, sharpen_tiny_image);
  /* This test is expected to die with SIGSEGV instead of returning */
  tcase_add_test_raise_signal(tc1, alpha_null_argument, SIGSEGV);

  srand(time(NULL) ^ getpid());
  /* Keep this image small: filter_blur costs O(size_x * size_y) per pixel */
  blur_radius_img = generate_rand_img_sized(1 + rand() % 32, 1 + rand() % 32);
  /* TODO: Fill in the 20 radii the loop test has to cover, with w and h the
   * width and the height of blur_radius_img:
   *   INT_MIN, INT_MIN + 1, INT_MIN / 2, -1, 0, 1,
   *   INT_MAX - 1, INT_MAX / 2, INT_MAX,
   *   w, h, w - 1, h - 1, w + 1, h + 1, w / 2, h / 2, 2 * w, 2 * h, -w
   * Careful: INT_MAX + 1 and INT_MIN - 1 are signed overflow, which is
   * undefined behaviour, so they are not in the list. */
  int w = blur_radius_img.size_x;
  int h = blur_radius_img.size_y;
  int radii[20] = {INT_MIN, INT_MIN + 1, INT_MIN / 2, -1,      0,
                   1,       INT_MAX - 1, INT_MAX / 2, INT_MAX, w,
                   h,       w - 1,       h - 1,       w + 1,   h + 1,
                   w / 2,   h / 2,       2 * w,       2 * h,   -w};
  memcpy(blur_radii, radii, sizeof(blur_radii));
  tcase_add_loop_test(tc1, blur_radius_limits, 0,
                      sizeof(blur_radii) / sizeof(blur_radii[0]));

  /* Tests for functionality */
  tcase_add_test(tc2, gray_zero_weights);
  tcase_add_loop_test(tc2, gray_reference_images, 0,
                      sizeof(gray_sources) / sizeof(gray_sources[0]));
  tcase_add_test(tc2, invert_roundtrip);
  tcase_add_test(tc2, blur_small_kernel);
  tcase_add_test(tc2, alpha_overwrite);
  tcase_add_loop_test(tc2, sharpen_reference_images, 0,
                      sizeof(sharpen_sources) / sizeof(sharpen_sources[0]));
  tcase_add_test(tc2, sharpen_checkerboard);

  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_VERBOSE);

  int number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  /* blur_radius_img has to stay alive until every test has run */
  free(blur_radius_img.px);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
