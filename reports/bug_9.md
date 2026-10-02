# BUG-9
## Category
Stack buffer overflow / underflow

## Description
In filter.c Line 254, the short summary after writing the output holds only 64 characters. Therefore, a long "output" arg can result in strcpy triggering stack buffer overflow. The output name needs to be truncated.

## Affected Lines in the original program
Source of the bug: Line 254
Where bug triggers: Line 255-256

## Expected vs Observed
Expected: Truncated output file name
Observed: Stack buffer overflow

## Steps to Reproduce
### Command
./filter_asan test_imgs/desert.png aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.png negative

### Output
Wrote aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.png
==137==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7ffed42027b0 at pc 0x756b5ebeb4bf bp 0x7ffed4202630 sp 0x7ffed4201dd8
WRITE of size 69 at 0x7ffed42027b0 thread T0


### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Change Lines 255-256 to:
  snprintf(summary, sizeof(summary), "%s: done", output);