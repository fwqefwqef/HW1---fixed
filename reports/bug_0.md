# BUG-0
## Category
Heap overflow/underflow

## Description

`filter_blur` builds the square of side `2 * radius + 1` around the pixel it is
averaging, but it never clips that square to the image before walking it. For
any pixel close to a border, `y_min`/`x_min` are negative and `y_max`/`x_max`
run past the last row/column, so `image_data[y][x]` reads pixels outside the
heap buffer that holds the image.

## Affected Lines in the original program
In `filter.c:83-86` (the unclipped bounds) and `filter.c:93-95`, where
`image_data[y][x]` performs the out-of-bounds read.

## Expected vs Observed
The comment above the function says that the pixels of the square which fall
outside the image do not count towards the average, so we expect the loops to
visit only the pixels that are really inside the image, and `count` to hold
only that many pixels.

What happens instead is that the loops run over the full square. For the pixel
at (0, 0) with radius 1 the very first iteration reads `image_data[-1][-1]`,
which is 1 row and 1 column before the start of the allocation. The values that
come back are whatever else lives on the heap, so every border pixel of the
output is computed from foreign memory and the divisor is wrong too (9 instead
of 4 in that corner). Reading far enough outside the mapping also makes the
program crash, which is what happens for a large radius.

## Steps to Reproduce

### Command

```
export ASAN_OPTIONS=detect_leaks=0
make clean && make asan
./solid_asan poc 8 8 ff8000
./filter_asan poc.png out.png blur 1
```

(`ASAN_OPTIONS=detect_leaks=0` is already set inside the Docker image; export it
yourself if you build somewhere else, otherwise the library's own leaks bury
the interesting output.)

The sanitizer build stops at the first bad access:

```
filter.c:95:47: runtime error: index -1 out of bounds for type 'pixel [*]'
```

(the exact wording and column depend on the compiler)

That single line is all you get: the `asan` target builds with
`-fno-sanitize-recover=all`, so the process stops right there. Rebuilding with
`-fsanitize-recover=undefined` lets the run continue, and AddressSanitizer then
reports the read itself as a `heap-buffer-overflow` inside `filter_blur`,
against the buffer allocated at `filter.c:71`.

### Proof-of-Concept Input (if needed)
None to attach: the `solid` utility in the same directory generates `poc.png`,
an 8x8 solid-color image. Any PNG reproduces the bug.

## Suggested Fix Description
Clip the square to the image before iterating over it. After computing
`y_min`, `y_max`, `x_min` and `x_max`, raise the two minima to 0 and lower the
two maxima to `img->size_y - 1` and `img->size_x - 1`. The loops then visit
only pixels that are inside the buffer, and because `count` is incremented
inside the loops it automatically ends up holding exactly the number of pixels
that were summed, which is the average the documentation describes.
