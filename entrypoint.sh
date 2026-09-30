#!/bin/bash
# Match the container user to the host user so that files created in the
# mounted directories are owned by you on the host.
MY_HOST_UID=${MY_UID:-${HOST_UID:-1000}}
MY_HOST_GID=${MY_GID:-${HOST_GID:-1000}}

if [ "$MY_HOST_UID" != "0" ]; then
    groupmod -g "$MY_HOST_GID" student 2>/dev/null || groupadd -g "$MY_HOST_GID" student 2>/dev/null
    usermod -u "$MY_HOST_UID" -g "$MY_HOST_GID" student 2>/dev/null
    chown student:student /homework 2>/dev/null
    exec gosu student "$@"
else
    exec "$@"
fi
