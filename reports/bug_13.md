# BUG-13
## Category
Unchecked system call return code

## Description
In mosaic.c Line 51 and 54, after malloc, the program does not check if the memory was allocated successfully. Therefore, if a dynamic memory is larger than the program's allocated memory is assigned, the program will crash instead of exiting normally.

## Affected Lines in the original program
Lines 51, 54

## Expected vs Observed
Expected: Program exits cleanly with return value 1
Observed: Crash occurs

## Steps to Reproduce
### Command
bash -c 'ulimit -v 120000; ./mosaic test_imgs/big.png big2.png 10'

### Output
Segmentation fault (core dumped)

### Proof-of-Concept Input (if needed)
big is a 4000x4000 image that is contained in the test_imgs directory, created by the following command:
./solid test_imgs/big.png 4000 4000 ff0000

## Suggested Fix Description
Add after Line 51:
  if (!img_out) {
    free(img->px);
    free(img);
    return 1;
  }

Add after Line 55:
  if (!img_out->px) {
    free(img_out);
    free(img->px);
    free(img);
    return 1;
  }