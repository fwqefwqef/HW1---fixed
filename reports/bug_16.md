# BUG-16
## Category
Temporal safety violation

## Description
In resize.c Lines 80-81, img is read after already being freed in Lines 77-78, which can lead to undefined behavior.

## Affected Lines in the original program
Lines 80-81

## Expected vs Observed
Expected: clean exit after resizing
Observed: heap-use-after-free error

## Steps to Reproduce
### Command
./resize_asan test_imgs/desert.png desert2.png 0.5

### Output
==59==ERROR: AddressSanitizer: heap-use-after-free on address 0x502000000032 at pc 0x5b0615f2d87f bp 0x7ffd3fdecc90 sp 0x7ffd3fdecc80
READ of size 2 at 0x502000000032 thread T0

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Relocate Lines 80-81 to Line 76.