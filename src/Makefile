CC = gcc
override CFLAGS += -O0 -g -w -ffp-contract=off
SANFLAGS = -fsanitize=address,undefined,float-cast-overflow \
           -fno-sanitize-recover=all -fno-omit-frame-pointer
LDLIBS = -lz -lm

# Check unit-test framework: ask pkg-config, and fall back to the libraries
# Ubuntu needs when pkg-config is not installed.
CHECK_CFLAGS := $(shell pkg-config --cflags check 2>/dev/null)
CHECK_LIBS := $(shell pkg-config --libs check 2>/dev/null || \
                echo -lcheck -lsubunit -lrt -lpthread -lm)

PROGS = solid checkerboard filter circle rect resize crop mosaic
ASAN_PROGS = $(PROGS:%=%_asan)

.PHONY: all utils asan clean
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

# "make -j clean all" must not run clean and the build at the same time
ifneq ($(filter clean,$(MAKECMDGOALS)),)
.NOTPARALLEL:
endif

# Plain build: ./solid, ./filter, ..., ./tests
all: utils tests
utils: $(PROGS)

# Sanitizer build (ASan + UBSan): ./solid_asan, ./filter_asan, ...,
# ./tests_asan. These are separate binaries, so they never mix with (or get
# skipped because of) a previous plain build.
asan: $(ASAN_PROGS) tests_asan

# --- PNG library (plain and sanitizer flavours) ---------------------------
libpngparser.a: pngparser.o crc.o
	ar rcs $@ $^
libpngparser_asan.a: pngparser_asan.o crc_asan.o
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
%_asan.o: %.c
	$(CC) $(CFLAGS) $(SANFLAGS) -c $< -o $@

pngparser.o pngparser_asan.o: pngparser.h crc.h
crc.o crc_asan.o: crc.h

# --- utilities -------------------------------------------------------------
$(PROGS): %: %.c pngparser.h libpngparser.a
	$(CC) $(CFLAGS) -o $@ $< libpngparser.a $(LDLIBS)
$(ASAN_PROGS): %_asan: %.c pngparser.h libpngparser_asan.a
	$(CC) $(CFLAGS) $(SANFLAGS) -o $@ $< libpngparser_asan.a $(LDLIBS)
filter filter_asan: filter.h

# --- unit tests ------------------------------------------------------------
tests.o: tests.c filter.h pngparser.h
	$(CC) $(CFLAGS) $(CHECK_CFLAGS) -c $< -o $@
tests_asan.o: tests.c filter.h pngparser.h
	$(CC) $(CFLAGS) $(SANFLAGS) $(CHECK_CFLAGS) -c $< -o $@
filter.o filter_asan.o: filter.h pngparser.h

tests: tests.o filter.o libpngparser.a
	$(CC) $(CFLAGS) -o $@ $^ $(CHECK_LIBS) $(LDLIBS)
tests_asan: tests_asan.o filter_asan.o libpngparser_asan.a
	$(CC) $(CFLAGS) $(SANFLAGS) -o $@ $^ $(CHECK_LIBS) $(LDLIBS)

clean:
	rm -f $(PROGS) $(ASAN_PROGS) tests tests_asan *.o *.a crop_history.log
	rm -rf *.dSYM
