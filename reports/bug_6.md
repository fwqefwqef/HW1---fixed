# BUG-6
## Category
Heap overflow/underflow

## Description
In crop.c lines 38-44, When validating the crop region, the program does not check the upper bounds for whether x+width or y+height would exceed the dimensions of the input image. Therefore, src[y+i][x+j] can read out-of-bounds values, resulting in unintended behavior.

## Affected Lines in the original program
Source of the bug: region validation (lines 38-44). The guard belongs right after load_png (line 48).
Where bug triggers: Line 71

## Expected vs Observed
Expected: Crop fails and returns error message if the crop region goes beyond the image bounds
Observed: The source image is copied multiple times across the crop region

## Steps to Reproduce
### Command
./crop test_imgs/desert.png desert2.png 0 0 3000 100

### Output
The top part of desert, repeated 12 times across a 3000x100 image.

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Add after line 48:
if (x + width > img->size_x || y + height > img->size_y) {
  free(img->px);
  free(img);
  goto error_usage;
}