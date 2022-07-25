#!/bin/bash

################################################################################
# Set Directory Locations
################################################################################

# Find the directory of this file, following symlinks.
#
# Reference:
# - https://stackoverflow.com/questions/59895/how-to-get-the-source-directory-of-a-bash-script-from-within-the-script-itself
function get_parent_dir() {
    local SOURCE="${BASH_SOURCE[0]}"
    while [ -h "$SOURCE" ]; do
        local DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
        SOURCE="$(readlink "$SOURCE")"
        [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
    done

    local PARENT_DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
    echo "${PARENT_DIR}"
}

SCRIPT_DIR=$(get_parent_dir)
REPO_DIR=${SCRIPT_DIR}/..

################################################################################
# Parse Arguments
################################################################################

CURRENT_ARCH=$(uname -m)
IMAGE_NAME="osr"

ARCH=
INSTALL=
MOUNT_IN_PLACE=

function usage() {
    cat <<EOF
Usage: $0 [OPTION]... [COMMAND]

Run the software compilation in a target Docker container.

If COMMAND is specified, run the requested command instead of compiling.

OPTIONS
   -a, --arch ARCH
       Run on the specified target architecture (aarch64, x86_64, etc.). By
       default, run on the host machine's architecture (${CURRENT_ARCH}).
   -h, --help
       Display this usage.
   -i, --install
       Install QEMU for cross-platform emulation.
   -m, --mount
       Mount the current directory in the container and compile in-place.
       By default, download the latest 'master' branch and compile that.
EOF
}

options=$(getopt -o a:him -l arch:,help,install,mount -- "$@")
eval set -- "$options"

while true ; do
    case "$1" in
    -a|--arch)
        shift
        ARCH="$1"
        ;;
    -h|--help)
        usage
        exit 1
        ;;
    -i|--install)
        INSTALL=T
        ;;
    -m|--mount)
        MOUNT_IN_PLACE=T
        ;;
    --)
        shift
        break
        ;;
    *)
        break
        ;;
    esac
    shift
done

################################################################################
# Run
################################################################################

function install_qemu() {
    if ! ls /proc/sys/fs/binfmt_misc/qemu-* >/dev/null 2>&1; then
        if [[ "${INSTALL}" == "T" ]]; then
            bar
            echo "Installing QEMU support..."
            docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
        else
            echo "Warning: QEMU not found. Cross-compilation may fail."
        fi
    fi
}

if [[ "${MOUNT_IN_PLACE}" == "T" ]]; then
    VOLUME="-v ${REPO_DIR}:/home/pointone"
fi

if [[ "${ARCH}" == "" ]] || [[ "${ARCH}" == "${CURRENT_ARCH}" ]]; then
    PLATFORM=""
    IMAGE="${IMAGE_NAME}"
elif [[ "${ARCH}" == "aarch64" ]]; then
    PLATFORM="--platform linux/arm64"
    IMAGE="${IMAGE_NAME}:${ARCH}"
elif [[ "${ARCH}" == "x86_64" ]]; then
    PLATFORM="--platform linux/amd64"
    IMAGE="${IMAGE_NAME}:${ARCH}"
else
    echo "Unsupported architecture '${ARCH}'."
    exit 1
fi

if [[ "${ARCH}" != "${CURRENT_ARCH}" ]]; then
    install_qemu
fi

docker run --rm ${PLATFORM} -it ${VOLUME} ${IMAGE} $*
