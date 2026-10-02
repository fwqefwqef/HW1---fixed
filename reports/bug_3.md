# BUG-3
## Category
Arithmetic overflow/underflow

## Description
In checkerboard.c lines 91-92, If (height + square_width - 1) or (width + square_width - 1) exceeds INT_MAX, a signed integer overflow occurs, resulting in a runtime error.

## Affected Lines in the original program
Lines 91-92

## Expected vs Observed
Expected: The program should be creating a very large checkerboard that is clipped at INT_MAX-1.
Observed: Runtime error

## Steps to Reproduce
### Command
./checkerboard_asan out.png 100 100 2147483647 ff0000 0000ff

### Output
checkerboard.c:91:38: runtime error: signed integer overflow: 2147483647 + 100 cannot be represented in type 'int'

### Proof-of-Concept Input (if needed)
None required.

## Suggested Fix Description
(img->size_y + square_width - 1) / square_width ===> (img->size_y - 1) / square_width + 1    // line 91
(img->size_x + square_width - 1) / square_width ===> (img->size_x - 1) / square_width + 1    // line 92