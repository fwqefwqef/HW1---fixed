# BUG-8
## Category
Type Error

## Description
In mosaic.c Line 11, sum_r, sum_g, sum_b, sum_a can only be max 255 because it is an uint8_t. However, they are intended to sum aggregates of RGB values that are 0-255. These values overflow very quickly within a few pixels, especially the alpha channel because pixels often have an alpha value of 255. If the block size is 10, there are 100 pixels in the block, meaning a value between 0-255 is divided by 100. This results in an average RGB and alpha value of 0-2, which means it will be almost completely transparent, like the below example.

## Affected Lines in the original program
Source of the bug: Line 11
Where the bug triggers: Line 15-18

## Expected vs Observed
Expected: Proper averages of pixels within block_size calculated
Observed: Incorrect averages approaching 0 for RGB, Alpha values

## Steps to Reproduce
### Command
./mosaic test_imgs/desert.png mosaic.png 10

### Output
A completely empty image in the dimensions of desert.png

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Change the type of the sum values in Line 11
long sum_r = 0, sum_g = 0, sum_b = 0, sum_a = 0;