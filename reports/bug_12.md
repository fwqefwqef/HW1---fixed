# BUG-12
## Category
Heap overflow / underflow

## Description
In mosaic.c Line 70, When writing pixel averages over the image, the program does not consider edge cases where blocks go outside of the boundaries of the image, resulting in writing pixel values to out-of-bounds locations and undefined behavior.

## Affected Lines in the original program
Line 70

## Expected vs Observed
Expected: Program does not write pixels that are out of bounds
Observed: Out of bounds write

## Steps to Reproduce
### Command
./mosaic_asan test_imgs/desert.png mosaic.png 10

### Output
mosaic.c:70:25: runtime error: index 255 out of bounds for type 'pixel [*]'

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Change Line 70:
        if (by + dy < img->size_y && bx + dx < img->size_x)
            dst[by + dy][bx + dx] = avg;