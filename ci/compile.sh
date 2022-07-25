#!/bin/bash

################################################################################
# Parse Arguments
################################################################################

CURRENT_ARCH=$(uname -m)

GIT_BRANCH=master
GIT_COMMIT=

CLEAN=T

function usage() {
    cat <<EOF
Usage: $0 [OPTION]...

Build Docker images for software compilation.

OPTIONS
   -b, --branch NAME
       Download source code for the specified git branch. Defaults to
       ${GIT_BRANCH}.
   -c, --commit HASH
       Download source code for the specified git commit.
   -h, --help
       Display this usage.
   -n, --no-clean
       Compile in existing build directory without deleting previous build
       results.
EOF
}

options=$(getopt -o b:c:hn -l branch:,commit:,help,no-clean -- "$@")
eval set -- "$options"

while true ; do
    case "$1" in
    -b|--branch)
        shift
        GIT_BRANCH="$1"
        ;;
    -c|--commit)
        shift
        GIT_COMMIT="$1"
        ;;
    -h|--help)
        usage
        exit 1
        ;;
    -n|--no-clean)
        CLEAN=
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

set -e

# Download OSRProducer example source to /home/pointone. If source code already
# exists, use that instead. To mount source code, run with:
#   docker run -v /path/to/source:/home/pointone ...
if [ ! -f README.md ]; then
    if [[ -n "${GIT_COMMIT}" ]]; then
        GIT_REF="${GIT_COMMIT}"
        GIT_STR="commit '${GIT_COMMIT}'"
    else
        GIT_REF="refs/heads/${GIT_BRANCH}"
        GIT_STR="branch '${GIT_BRANCH}'"
    fi

    echo "External source code not found. Downloading ${GIT_STR} from github..."

    wget https://github.com/PointOneNav/osr-producer/archive/${GIT_REF}.zip -O repo.zip
    unzip repo.zip

    pwd
    ls osr-producer-*
    mv osr-producer-*/* .
    rm -rf osr-producer-*
    ls
fi

BUILD_DIR="build-${CURRENT_ARCH}"
echo "Building in ${BUILD_DIR}..."

set -x

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

if [[ "${CLEAN}" == "T" ]]; then
    rm -rf *
fi

cmake ..
make -j
