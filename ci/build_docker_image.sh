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

REQUESTED_ARCH=
INSTALL=

function usage() {
    cat <<EOF
Usage: $0 [OPTION]...

Build Docker images for software compilation.

OPTIONS
   -a, --arch ARCH
       Build a container for the specified architecture. By default, only the
       host machine's architecture will be built (${CURRENT_ARCH}). Set to 'all'
       to build for all supported architectures.
   -h, --help
       Display this usage.
   -i, --install
       Install QEMU for cross-platform emulation.
EOF
}

options=$(getopt -o a:hi -l arch:,help,install -- "$@")
eval set -- "$options"

while true ; do
    case "$1" in
    -a|--arch)
        shift
        REQUESTED_ARCH="$1"
        ;;
    -h|--help)
        usage
        exit 1
        ;;
    -i|--install)
        INSTALL=T
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

function bar() {
        cat <<EOF
################################################################################
EOF
}

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

function build() {
    local ARCH=$1

    if [[ "${ARCH}" == "${CURRENT_ARCH}" ]]; then
        bar
        echo "Building for current architecture (${ARCH})..."
        docker build --rm -t ${IMAGE_NAME} .
        docker tag ${IMAGE_NAME} ${IMAGE_NAME}:${CURRENT_ARCH}
    else
        if [[ "${ARCH}" == "aarch64" ]]; then
            PLATFORM="linux/arm64"
            PREFIX="arm64v8/"
        elif [[ "${ARCH}" == "x86_64" ]]; then
            PLATFORM="linux/amd64"
            PREFIX="amd64/"
        else
            echo "Unsupported architecture ${ARCH}."
            exit 1
        fi

        bar
        echo "Building for target ${ARCH}..."
        install_qemu
        docker build --rm --platform ${PLATFORM} --build-arg ARCH_PREFIX=${PREFIX} -t ${IMAGE_NAME}:${ARCH} .
    fi
}

set -e

cd ${SCRIPT_DIR}

if [[ "${REQUESTED_ARCH}" == "all" ]]; then
    build aarch64
    build x86_64
else
    if [[ "${REQUESTED_ARCH}" == "" ]]; then
        REQUESTED_ARCH=${CURRENT_ARCH}
    fi

    build ${REQUESTED_ARCH}
fi
