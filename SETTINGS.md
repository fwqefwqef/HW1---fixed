# COSE451 Software Security - HW 01 Environment Setup Guide

This document explains how to set up the environment for HW1. You can either use
the provided Docker image (recommended) or install the tools locally.

The grading environment is Ubuntu 22.04 with gcc 11 and `check` 0.15.2, which is
exactly what the Docker image gives you. If you work locally, make sure your code
still builds and behaves the same way inside the container before submitting.

## Docker environment (recommended)

### Prerequisites

- Docker (and optionally Docker Compose, which ships with Docker Desktop)
- Docker must be running before you start

Installation instructions: https://docs.docker.com/get-started/get-docker/

### Method 1: Docker Compose

```bash
# From the root directory of this assignment (the directory holding src/)

# Build the image and start the container in the background.
# MY_UID/MY_GID make files you create inside the container belong to you on the host.
export MY_UID=$(id -u) MY_GID=$(id -g)
docker compose up -d

# Open a shell in the container
docker compose exec -u student hw1-environment bash

# Inside the container: build and run
# (most tests fail until you do the assignment; HW1.pdf describes that
#  starting state)
cd src && make && ./tests

# Leave the shell
exit

# Stop the container when you are done
docker compose down
```

### Method 2: Plain Docker

```bash
# Build the image
docker build -t cose451-hw1 .

# Start an interactive container with this directory mounted
docker run -it --rm \
  -e MY_UID=$(id -u) -e MY_GID=$(id -g) \
  -v "$(pwd)":/homework \
  cose451-hw1
```

To keep a container around instead of removing it on exit:

```bash
docker run -d --name hw1-dev -e MY_UID=$(id -u) -e MY_GID=$(id -g) \
  -v "$(pwd)":/homework cose451-hw1 tail -f /dev/null
docker exec -it -u student hw1-dev bash
# when finished
docker stop hw1-dev && docker rm hw1-dev
```

### Troubleshooting

**Files created in the container are owned by root on the host.** You attached to
the container without `-u student` (`docker exec` and `docker compose exec` skip
the entrypoint that switches users). Use `docker exec -it -u student ...`, or fix
the ownership afterwards on the host:

```bash
sudo chown -R "$(id -u):$(id -g)" src reports
```

**You need a root shell inside the container** (for example to install an extra
package): pass `MY_UID=0`.

```bash
docker run -it --rm -e MY_UID=0 -v "$(pwd)":/homework cose451-hw1
```

**Rebuilding the image after changing the Dockerfile:**

```bash
docker compose down
docker compose build --no-cache
docker compose up -d
```

**`docker compose` vs `docker-compose`:** newer Docker versions use the
`docker compose` subcommand. If your installation only has the old standalone
binary, replace `docker compose` with `docker-compose` in the commands above.

## Local environment

### Ubuntu / Debian

```bash
sudo apt-get update
sudo apt-get install -y gcc make build-essential pkg-config \
    zlib1g-dev check clang-format gdb valgrind
```

### Fedora

```bash
sudo dnf install -y gcc make pkgconf-pkg-config zlib-devel check-devel \
    clang-tools-extra gdb valgrind
```

### CentOS / RHEL 8+

```bash
sudo dnf install -y epel-release
sudo dnf install -y gcc make pkgconf-pkg-config zlib-devel check-devel \
    clang-tools-extra gdb valgrind
```

### macOS

Apple Silicon and Intel Macs can build the programs, but they are not the
grading environment. `gdb` installs on Apple Silicon but cannot debug native
arm64 binaries, so use `lldb` or the container. `stat`, `sed` and other tools
differ from the GNU versions. Apple clang turns on hardening that changes how
some bugs behave: one that crashes under the container's gcc can look harmless
here. Use Docker whenever you want to be sure.

```bash
brew install make check pkg-config clang-format
```

Homebrew installs GNU make as `gmake`. On macOS `gcc` runs Apple clang unless
you install a real gcc and put it first in your `PATH`.

### Verifying your installation

```bash
gcc --version
make --version
pkg-config --libs check || echo "check not found - install the 'check' package"
clang-format --version
```

## Sanitizers

The `Makefile` has an `asan` target that rebuilds the programs with
AddressSanitizer and UndefinedBehaviorSanitizer. These report memory errors and
undefined behaviour that a normal build does not report. Use them to confirm a
bug you suspect, and to confirm your fix.

```bash
cd src
make asan                   # produces solid_asan, filter_asan, ..., tests_asan
./filter_asan input.png out.png blur 1
```

The sanitizer builds are named `<program>_asan`, so they never overwrite the
normal binaries and you can keep both around at the same time.

The PNG library allocates memory it never frees, so leak reports are turned off
in the container (`ASAN_OPTIONS=detect_leaks=0`). Set the same variable if you
run sanitizer builds outside Docker.

## Notes

- Your work belongs in `src/` and `reports/`. Both are mounted into the
  container, so it survives restarts.
- `CK_FORK` is not set in the image. Check runs each test in its own process,
  so a crashing test does not kill the rest of the suite. While debugging a
  single test under `gdb`, set `CK_FORK=no` for that run only:
  `CK_FORK=no gdb ./tests`.
- Before submitting, format your sources: `clang-format -i --style=LLVM *.c *.h`
  inside the container, so you get the same clang-format version as the graders.
