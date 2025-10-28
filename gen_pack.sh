#!/usr/bin/env bash
# Version: 3.1
# Date: 2024-04-17
# This bash script generates a CMSIS Software Pack: CMSIS-Driver_Validation
#

set -o pipefail

# Set version of gen pack library
# For available versions see https://github.com/Open-CMSIS-Pack/gen-pack/tags.
# Use the tag name without the prefix "v", e.g., 0.7.0
REQUIRED_GEN_PACK_LIB="0.11.3"

# Set default command line arguments
DEFAULT_ARGS=(-c "")

# Pack warehouse directory - destination
# Default: ./output
#
# PACK_OUTPUT=./output

# Temporary pack build directory
# Default: ./build
#
# PACK_BUILD=./build

# Specify directory names to be added to pack base directory
# An empty list defaults to all folders next to this script.
# Default: empty (all folders)
#
PACK_DIRS="
  Boards
  Config
  Documentation
  Include
  Source
  Template
  Tools
"

# Specify file names to be added to pack base directory
# Default: empty
#
PACK_BASE_FILES="
  LICENSE.txt
  README.md
"

# Specify file names to be deleted from pack build directory
# Default: empty
#
PACK_DELETE_FILES="
  Documentation/Doxygen
"

# Specify patches to be applied
# Default: empty
#
# PACK_PATCH_FILES="
#     <list patches here>
# "

# Specify addition argument to packchk
# Default: empty
#
# PACKCHK_ARGS=()

# Specify additional dependencies for packchk
# Default: empty
#
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
#
PACK_CHANGELOG_MODE="tag"
# Specify file patterns to be excluded from the checksum file
# Default: <empty>
# Values:
# - empty          All files packaged are included in the checksum file
# - glob pattern   One glob pattern per line. Files matching a given pattern are excluded
#                  from the checksum file
# - "*"            The * (match all pattern) can be used to skip checksum file creating completely.
# 
# PACK_CHECKSUM_EXCLUDE="
#   <list file patterns here>
# "

#
# custom pre-processing steps
#
# usage: preprocess <build>
#   <build>  The build folder
#
function preprocess() {
  # add custom steps here to be executed
  # before populating the pack build folder
  pushd ./Documentation/Doxygen/ > /dev/null
  echo "Changing working directory to $(pwd)"

  echo "Executing ./gen_doc.sh"
  ./gen_doc.sh

  popd > /dev/null
  echo "Changing working directory to $(pwd)"
  return 0
}

#
# custom post-processing steps
#
# usage: postprocess <build>
#   <build>  The build folder
#
function postprocess() {
  # add custom steps here to be executed
  # after populating the pack build folder
  # but before archiving the pack into output folder
  return 0
}

############ DO NOT EDIT BELOW ###########

# Set GEN_PACK_LIB_PATH to use a specific gen-pack library root
# ... instead of bootstrap based on REQUIRED_GEN_PACK_LIB
if [[ -n "${GEN_PACK_LIB_PATH}" ]] && [[ -f "${GEN_PACK_LIB_PATH}/gen-pack" ]]; then
  . "${GEN_PACK_LIB_PATH}/gen-pack"
else
  . <(curl -sL "https://raw.githubusercontent.com/Open-CMSIS-Pack/gen-pack/main/bootstrap")
fi

gen_pack "${DEFAULT_ARGS[@]}" "$@"

exit 0
