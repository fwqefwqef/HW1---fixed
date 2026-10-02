# BUG-4
## Category
Heap overflow/underflow

## Description
In circle.c line 10, draw_pixel() only checks the upper bounds of the image to determine whether the pixel it will draw is valid, and does not check for negative coordinates. Therefore, when x or y is negative, the program can access out-of-bounds values, resulting in undefined behavior.

## Affected Lines in the original program
Source of the bug: Line 10
Where bug triggers: Line 13

## Expected vs Observed
Expected: Ignore circle pixels that are out of the image bounds.
Observed: the sanitizer reports an out-of-bounds (negative index) access at line 13. (On the plain build, segmentation fault occurs.)

## Steps to Reproduce
### Command
./circle_asan test_imgs/desert.png desert2.png 100 4 100 000000

### Output
circle.c:13:18: runtime error: index -96 out of bounds for type 'pixel [*]'

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Add lower-bound checks to line 10: if (x >= 0 && x < img->size_x && y >= 0 && y < img->size_y)