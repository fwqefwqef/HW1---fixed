# BUG-19
## Category
Unchecked system call return code

## Description
In crop.c Line 76, the program does not check whether the store_png succeeded, like every other implementation of store_png (for example, in rect.c). This means that writing to an invalid location or a directory such as "/" still returns success with return 0, even though the write has failed.

## Affected Lines in the original program
Line 76

## Expected vs Observed
Expected: return value of 1 with error message
Observed: No image returned

## Steps to Reproduce
### Command
./crop test_imgs/desert.png / 0 0 100 100; echo $?

### Output
0 (Image is not created)

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Replace Line 76:
if (store_png(output, out, NULL, 0)) {
    free(out->px);
    free(out);
    free(img->px);
    free(img);
    printf("Couldn't write output image\n");
    return 1;
}