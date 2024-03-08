#!/bin/bash
# Version: 2.3
# Date: 2022-09-28
# This bash script generates a CMSIS-Driver Software Pack
#

set -o pipefail

# Set version of gen pack library
REQUIRED_GEN_PACK_LIB="0.6.1"

# Set default command line arguments
DEFAULT_ARGS=(-c "")

# Pack warehouse directory - destination
PACK_OUTPUT=./output

# Temporary pack build directory
PACK_BUILD=./build

# Specify directory names to be added to pack base directory
PACK_DIRS="
  Boards
  Config
  Documentation
  Include
  Scripts
  Source
  Tools
"

# Specify file names to be added to pack base directory
PACK_BASE_FILES="
  LICENSE.txt
  README.md
"

# Specify file names to be deleted from pack build directory
PACK_DELETE_FILES=""

# Specify patches to be applied
PACK_PATCH_FILES=""

# Specify addition argument to packchk
PACKCHK_ARGS=()

# Specify additional dependencies for packchk
PACKCHK_DEPS="
  ARM.CMSIS.pdsc
  ARM.CMSIS-RTX.pdsc
  Keil.STM32F2xx_DFP.pdsc
  Keil.STM32F4xx_DFP.pdsc
  Keil.STM32F7xx_DFP.pdsc
  Keil.B-L475E-IOT01A_BSP.pdsc
  Infineon.XMC4000_DFP.pdsc
  NXP.LPCXpresso55S69_BSP.pdsc
  NXP.EVK-MIMXRT1064_BSP.pdsc
"

# Optional: restrict fallback modes for changelog generation
# Default: full
# Values:
# - full      Tag annotations, release descriptions, or commit messages (in order)
# - release   Tag annotations, or release descriptions (in order)
# - tag       Tag annotations only
PACK_CHANGELOG_MODE="tag"

# custom preprocessing steps
function preprocess() {
  ./DoxyGen/gen_doc.sh
}

# custom post-processing steps
# function postprocess() {
# }

############ DO NOT EDIT BELOW ###########

function install_lib() {
  local URL="https://github.com/Open-CMSIS-Pack/gen-pack/archive/refs/tags/v$1.tar.gz"
  echo "Downloading gen-pack lib to '$2'"
  mkdir -p "$2"
  curl -L "${URL}" -s | tar -xzf - --strip-components 1 -C "$2" || exit 1
}

function load_lib() {
  if [[ -d ${GEN_PACK_LIB} ]]; then
    . "${GEN_PACK_LIB}/gen-pack"
    return 0
  fi
  local GLOBAL_LIB="/usr/local/share/gen-pack/${REQUIRED_GEN_PACK_LIB}"
  local USER_LIB="${HOME}/.local/share/gen-pack/${REQUIRED_GEN_PACK_LIB}"
  if [[ ! -d "${GLOBAL_LIB}" && ! -d "${USER_LIB}" ]]; then
    echo "Required gen_pack lib not found!" >&2
    install_lib "${REQUIRED_GEN_PACK_LIB}" "${USER_LIB}"
  fi

  if [[ -d "${GLOBAL_LIB}" ]]; then
    . "${GLOBAL_LIB}/gen-pack"
  elif [[ -d "${USER_LIB}" ]]; then
    . "${USER_LIB}/gen-pack"
  else
    echo "Required gen-pack lib is not installed!" >&2
    exit 1
  fi
}

load_lib
gen_pack "${DEFAULT_ARGS[@]}" "$@"

exit 0
