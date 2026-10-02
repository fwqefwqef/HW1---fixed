# BUG-17
## Category
Stack buffer overflow / underflow

## Description
In solid.c Lines 35-43, final_name size is 32 chars, and error handling properly handles names that are above 32 chars, but for names that do not include .png and are 28-31 chars long, the program will append .png (4 chars) to the name which results in stack buffer overflow.

## Affected Lines in the original program
Lines 35-43

## Expected vs Observed
Expected: Proper error handling accounting for the .png addition
Observed: Stack buffer overflow

## Steps to Reproduce
### Command
./solid_asan aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa 100 100 ff0000

### Output
==35==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7ffd59426010 at pc 0x7c2ac7c702c3 bp 0x7ffd59425f00 sp 0x7ffd594256a8
WRITE of size 5 at 0x7ffd59426010 thread T0

### Proof-of-Concept Input (if needed)
None required.

## Suggested Fix Description
Add between Line 40 and 41
  if (strlen(final_name) + 4 >= sizeof(final_name))
    goto error;    