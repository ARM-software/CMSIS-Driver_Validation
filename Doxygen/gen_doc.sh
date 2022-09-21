#!/bin/bash
# Version: 1.0
# Date: 2022-07-15
# This bash script generates CSMSIS Driver Verification documentation
#
# Pre-requisites:
# - bash shell (for Windows: install git for Windows)
# - doxygen


set -o pipefail

DIRNAME=$(dirname $(readlink -f $0))
DOXYGEN=$(which doxygen 2>/dev/null)
REQ_DXY_VERSION="1.9.2"
REGEN=1
ALLPARTS=($(find ${DIRNAME} -mindepth 1 -maxdepth 1 -type d -exec basename {} \;))
PARTS=()

if [[ -z "$*" ]]; then
    REGEN=1
else
    for part in "$*"; do
        if [[ " ${ALLPARTS[@]} " =~ " $part " ]]; then
            PARTS+=($part)
        fi
    done
fi

if [[ ! -f "${DOXYGEN}" ]]; then
    echo "Doxygen not found!" >&2
    echo "Did you miss to add it to PATH?"
    exit 1
else
    version=$("${DOXYGEN}" --version | sed -E 's/.*([0-9]+\.[0-9]+\.[0-9]+).*/\1/')
    echo "Doxygen is ${DOXYGEN} at version ${version}"
    if [[ "${version}" != "${REQ_DXY_VERSION}" ]]; then
        echo "Doxygen required to be at version ${REQ_DXY_VERSION}!" >&2
        exit 1
    fi
fi

function doxygen {
    partname=$(basename $(dirname $1))
    if [[ $REGEN != 0 ]] || [[ " ${PARTS[@]} " =~ " ${partname} " ]]; then
        pushd "$(dirname $1)" > /dev/null
        echo "${DOXYGEN} $1"
        "${DOXYGEN}" $(basename "$1")
        popd > /dev/null
        
        if [[ $2 != 0 ]]; then
            # use with $partname in path for complex multi-folder projects
            #mkdir -p "${DIRNAME}/../Documentation/html/search/"
            #cp -f "${DIRNAME}/templates/search.css" "${DIRNAME}/../Documentation/html/search/"
            mkdir -p "${DIRNAME}/../Documentation/html/search/"
            cp -f "${DIRNAME}/templates/search.css" "${DIRNAME}/../Documentation/html/search/"
        fi
        
        projectName=$(grep -E "PROJECT_NAME\s+=" $1 | sed -r -e 's/[^"]*"([^"]+)".*/\1/')
        projectNumber=$(grep -E "PROJECT_NUMBER\s+=" $1 | sed -r -e 's/[^"]*"([^"]+)".*/\1/')
        datetime=$(date -u +'%a %b %e %Y %H:%M:%S')
        sed -e "s/{datetime}/${datetime}/" "${DIRNAME}/templates/footer.js" \
          | sed -e "s/{projectName}/${projectName}/" \
          | sed -e "s/{projectNumber}/${projectNumber}/" \
          > "${DIRNAME}/../Documentation/html/footer.js"
    fi
}

if [[ $REGEN != 0 ]]; then
    echo "Cleaning existing documentation ..."
    find "${DIRNAME}/../Documentation/" -mindepth 1 -maxdepth 1 -type d -exec rm -rf {} +
fi

echo "Generating documentation ..."
doxygen "${DIRNAME}/CMSIS_DV.dxy" 1
exit 0