# BUG-1
## Category
Local persisting pointers

## Description
In filter.c Line 22, gray_pixel() returns a pointer to p, where p is a local struct. The lifetime of p ends when gray_pixel() returns, so the returned pointer in Line 45 is invalid. Line 46 then tries to read "red" value from this invalid pointer, which is undefined behavior. 

## Affected Lines in the original program
Source of the bug: Line 22
Where bug triggers: Line 45-46

## Expected vs Observed
Expected: A grayscale image
Observed: No image returned

## Steps to Reproduce
### Command
./filter_asan test_imgs/desert.png gray.png grayscale

### Output
filter.c:46:31: runtime error: member access within null pointer of type 'struct pixel'

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Return the pixel struct itself instead of a pointer to the pixel. Use that pixel for the red/blue/green overwrites instead of the pointer. 
Line 22: return p
Line 45: struct pixel g = gray_pixel(luminosity);
Line 46-48: image_data[i][j].red = g.red; image_data[i][j].green = g.green; image_data[i][j].blue = g.blue;