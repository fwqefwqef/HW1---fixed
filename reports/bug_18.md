# BUG-18
## Category
Wrong operators

## Description
In crop.c Line 39, the program checks whether the width is a negative value AND whether the height is a negative value, meaning one of the two can be negative, and then later in line 57, the program fails to allocate memory due to unsigned size_t interpreting the negative value as a huge number.

## Affected Lines in the original program
Source of the bug: Line 39
Where bug triggers: Line 57

## Expected vs Observed
Expected: goto error_usage; and normal exit with return value 1
Observed: the negative height passes validation and is reported as Couldn't allocate memory, instead of being rejected with usage error.

## Steps to Reproduce
### Command
./crop test_imgs/desert.png desert2.png 0 0 100 -100

### Output
Couldn't allocate memory

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Change Line 39:
  if (width <= 0 || height <= 0) {