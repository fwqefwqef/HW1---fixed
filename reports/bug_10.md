# BUG-10
## Category
Iteration error

## Description
In filter.c Line 149, since i iterates over 1 less row than necessary, the bottom row will not have the alpha filter applied.

## Affected Lines in the original program
Source of the bug: Line 149
Where bug triggers: Line 151

## Expected vs Observed
Expected: Alpha filter applied across the entire image
Observed: Alpha filter skips the bottom row

## Steps to Reproduce
### Command
./filter_asan test_imgs/ck.png ck2.png alpha 00

### Output
An empty image except the bottom row which still has the checkerboard pattern, in the dimensions of ck.png

### Proof-of-Concept Input (if needed)
ck.png is contained in test_imgs directory.

## Suggested Fix Description
Replace Line 149 with:
  for (long i = 0; i < img->size_y; i++) {