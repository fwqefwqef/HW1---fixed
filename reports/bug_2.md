# BUG-2
## Category
Temporal safety violation

## Description
Line 122, if store_png fails, for example if the program is supplied with a directory for the "output_name" arg, it frees img->px in line 124 and then frees img->px again in line 128, making the program exit with Aborted (core dumped) instead of exiting cleanly.

## Affected Lines in the original program
Line 124, 128

## Expected vs Observed
Expected: print "Couldn't write output image" and exit cleanly with status 1.
Observed: Aborted (core dumped)

## Steps to Reproduce
### Command
./checkerboard / 100 100 10 ff0000 0000ff

### Output
Couldn't write output image
double free or corruption (!prev)
Aborted (core dumped)

### Proof-of-Concept Input (if needed)
None required.

## Suggested Fix Description
Remove free(img->px); from line 124