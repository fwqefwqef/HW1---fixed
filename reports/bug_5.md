# BUG-5
## Category
Wrong operators

## Description
In circle.c Line 54, the operator is | instead of &, always causing the value of color.red to be ff, regardless of input.

## Affected Lines in the original program
Line 54

## Expected vs Observed
Expected: A circle whose color is fully customizable with hex input
Observed: A circle whose red value is always forced to ff

## Steps to Reproduce
### Command
./circle_asan test_imgs/desert.png desert2.png 50 100 100 000000 

### Output
Red circle instead of black circle

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Change Line 54 to:
color.red = (hex_color & 0xff0000) >> 16;