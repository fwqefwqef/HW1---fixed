# BUG-14
## Category
Iteration error

## Description
In rect.c Line 56, the iteration starts at i=1, meaning the rectangle draw will ignore the top row of the image.

## Affected Lines in the original program
Line 56

## Expected vs Observed
Expected: A complete rectangle draw
Observed: The top row of the image will never be affected

## Steps to Reproduce
### Command
./rect_asan test_imgs/ck.png ck2.png 0 0 100 100 000000

### Output
ck.png with the entire image covered in black except the top row.

### Proof-of-Concept Input (if needed)
ck.png is contained in test_imgs directory.

## Suggested Fix Description
Change Line 56:
    for (int i = 0; i < height; i++) {