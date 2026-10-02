# BUG-11
## Category
String vulnerability

## Description
In filter.c Line 250, output variable is printed directly as printf(output) instead of using a format specifier like printf("%s", output). Output argument is fully attacker-controlled, so the attacker can name the output into format specifiers like %d and %x to leak stack/register value.

## Affected Lines in the original program
Source of the bug: Line 250

## Expected vs Observed
Expected: Confirmation of writing file, Wrote %d.png
Observed: prints a leaked stack/register value instead of the literal filename

## Steps to Reproduce
### Command
./filter_asan test_imgs/desert.png %d.png alpha 00

### Output
Wrote 128.png
%d.png: done

### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
change Line 250 to:
  printf("%s",output);