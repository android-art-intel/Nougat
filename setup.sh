#!/bin/bash

# settings
ANDROID_VER="android-7.0.0_r14"
if [ -n "$1" ]
  then
    ANDROID_VER="$1"
fi
[ -z "${CURRENT_DIR}" ] && CURRENT_DIR="$0"
if [ ! -e "$CURRENT_DIR" ]; then
  export CURRENT_DIR=$(pwd)
else
  [ ! -d "$CURRENT_DIR" ] && CURRENT_DIR=$(dirname $CURRENT_DIR)
  export CURRENT_DIR=$(cd $CURRENT_DIR; pwd)
fi
WS_TOP=${CURRENT_DIR}
WS_OUT=${WS_TOP}/out

# Create out dir
[ -d "${WS_OUT}" ] || mkdir ${WS_OUT} || exit 1

# Download repo
([ -x "${WS_OUT}/repo" ] || curl ${CURL_PROXY} https://storage.googleapis.com/git-repo-downloads/repo > ${WS_OUT}/repo) || exit 1
chmod a+x ${WS_OUT}/repo

# Initalize Android android-7.0.0_r14 workspace
echo "cd ${WS_TOP} && ${WS_OUT}/repo init -u https://android.googlesource.com/platform/manifest -b ${ANDROID_VER})"
(cd ${WS_TOP} && ${WS_OUT}/repo init -u https://android.googlesource.com/platform/manifest -b ${ANDROID_VER}) || exit 1

# Download Android android-6.0.1_r46 sources
(cd ${WS_TOP} && ./.repo/repo/repo sync -c -j5) || exit 1

WS_CAPSTONE=${WS_TOP}/capstone
CAPSTONE_VER=c508c4a0270b566f14bb9f2a7ce6243df10c8fc4

# Download capstone disassembler (https://github.com/aquynh/capstone
echo "cd ${WS_TOP} && git clone https://github.com/aquynh/capstone.git"
(cd ${WS_TOP} && git clone https://github.com/aquynh/capstone.git) || exit 1

# Checkout a particular version of capstone tested
echo "cd ${WS_CAPSTONE} && git checkout c508c4a0270b566f14bb9f2a7ce6243df10c8fc4"
(cd ${WS_CAPSTONE} && git checkout ${CAPSTONE_VER}) || exit 1

# apply extra-patches
cd ${WS_TOP}
${CURRENT_DIR}/apply_extra.sh

