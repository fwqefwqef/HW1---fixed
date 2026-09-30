# COSE451 Software Security - HW 01 build and test environment
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Toolchain, libraries and debugging tools used by this assignment
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    make \
    build-essential \
    pkg-config \
    zlib1g-dev \
    check \
    clang-format \
    gdb \
    valgrind \
    python3 \
    xxd \
    file \
    zip \
    unzip \
    git \
    vim \
    nano \
    gosu \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /homework

# The PNG library leaks memory by design; leak reports would drown the
# sanitizer output students care about.
ENV ASAN_OPTIONS=detect_leaks=0

# NOTE: CK_FORK is deliberately NOT set here. Check runs every test in its own
# process by default, so one crashing test cannot take the whole suite down.
# Set CK_FORK=no only while debugging a single test under gdb (see SETTINGS.md).

# Run as a user whose UID/GID match the host, so files created in the mounted
# src/ and reports/ directories stay editable on the host.
RUN useradd -m -s /bin/bash student
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]

CMD ["/bin/bash"]
