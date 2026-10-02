# BUG-15
## Category
Type error

## Description
In rect.c Lines 24-28, the coordinates of the rectangle are stored as unsigned chars, meaning they can only store values up to 255. Any value above that will overflow, resulting in the drawn rectangle to be much smaller than intended or to not exist at all.

## Affected Lines in the original program
Line 24-28

## Expected vs Observed
Expected: Rectangle draw is clamped to the upper bounds of the image.
Observed: Rectangle is drawn smaller than intended or not drawn at all.

## Steps to Reproduce
### Command
./rect_asan test_imgs/desert.png desert2.png 100 100 300 300 ff0000

### Output
The same image as desert.png, with no rectangle drawn over it.

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Change Lines 24-28:
  int top_left_x = atoi(argv[3]);
  int top_left_y = atoi(argv[4]);

  int bottom_right_x = atoi(argv[5]);
  int bottom_right_y = atoi(argv[6]);