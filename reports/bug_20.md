# BUG-20
## Category
String vulnerability

## Description
In filter.c Line 202, strncat limits the size of the input to size(input). However, C strings have a null terminated character '\0' at the end, which means strncat needs to account for this and limit the size of the input write to 1 less character.

## Affected Lines in the original program
Line 202

## Expected vs Observed
Expected: Proper string parsing
Observed: stack buffer overflow

## Steps to Reproduce
### Command
./filter_asan aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa out.png negative

### Output
==40==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7ffdd404cacf at pc 0x745f42bd18c1 bp 0x7ffdd404c830 sp 0x7ffdd404bfd8
WRITE of size 256 at 0x7ffdd404cacf thread T0

### Proof-of-Concept Input (if needed)
None required.

## Suggested Fix Description
Change Line 202:
  strncat(input, argv[1], sizeof(input)-1);