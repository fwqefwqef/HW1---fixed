# BUG-7
## Category
Command Injection

## Description
In crop.c Lines 80-82, user-controlled input arg "output" is directly converted into shell program text, allowing command injection.

## Affected Lines in the original program
Lines 80-82

## Expected vs Observed
Expected: user input is converted into the filename
Observed: user input is converted into shell program text

## Steps to Reproduce
### Command
./crop_asan test_imgs/desert.png 'a; touch pwned.txt #' 0 0 100 100

### Output
cropped a
(pwned.txt is created in the src directory)


### Proof-of-Concept Input (if needed)
desert.png is contained in test_imgs directory.

## Suggested Fix Description
Replace Lines 79-82 with:
FILE *log = fopen("crop_history.log", "a");
if (log) {
  fprintf(log, "cropped %s\n", output);
  fclose(log);
}