#
# Copyright (C) 2015 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

OUTPUT_PROFILE_DIR=/tmp
RUN_APCT=$HOME/apct-2.7.2/run.sh
RUN_BENCHMARK=$HOME/src/benchmarks/run.sh

function recompile_boot() {
  echo "Force recompilation of boot.oat"
  adb shell stop
  adb shell rm -f /data/dalvik-cache/x*/*
  adb logcat -b system -c
  adb shell sleep 1
  adb shell logcat -c
  adb shell sleep 1
  adb shell logcat -c
  adb logcat > logcat.log &
  LOGCAT_PID=$!
  sleep 1
  adb shell start
  let ORIG=1000
  let CNT=$ORIG
  let N=0
  let ret=0
  OK=
  FAIL=
  let WAIT=5
  let boot=0
  while [ $CNT -gt 0 ]
  do
    sleep 1
    let CNT=CNT-1
    let N=N+1
    OK=`grep -a -m 1 'AlertReceiver: onReceive: a=android.intent.action.BOOT_COMPLETED Intent' < logcat.log`
    if [ ! "$OK" == "" ]
    then
      if [ $boot == 0 ]
      then
        let boot=1
        echo "BOOT COMPLETE!"
      fi
      # Wait some more time to check for late death.
      if [ $CNT -gt $WAIT ]
      then
        echo "Waiting $WAIT more second(s) just in case"
        CNT=$WAIT
      fi
    fi
    grep -q 'read: Unexpected EOF!' < logcat.log
    if [ $? == 0 ]
    then
      echo "restart LOGCAT"
      adb logcat > logcat.log &
      LOGCAT_PID=$!
    fi
  done
  kill -9 $LOGCAT_PID

  echo TIME: $N
}

function get_profiles() {
  mkdir -p $OUTPUT_PROFILE_DIR/profile.x86{,_64}
  echo "Fetching profiles to $OUTPUT_PROFILE_DIR/profile.x86{,_64}"
  PROF_DIR=/system/framework
  adb pull $PROF_DIR/x86/profile $OUTPUT_PROFILE_DIR/profile.x86
  mv -f $OUTPUT_PROFILE_DIR/profile.x86/profile/* $OUTPUT_PROFILE_DIR/profile.x86
  rmdir $OUTPUT_PROFILE_DIR/profile.x86/profile
  adb pull $PROF_DIR/x86_64/profile $OUTPUT_PROFILE_DIR/profile.x86_64/
  mv -f $OUTPUT_PROFILE_DIR/profile.x86_64/profile/* $OUTPUT_PROFILE_DIR/profile.x86_64
  rmdir $OUTPUT_PROFILE_DIR/profile.x86_64/profile
}

function run_apct() {
  echo "Not running APCT because it times out during rebuild of system"
  # echo "Run APCT"
  # $RUN_APCT APCT_EXCLUDE="*SETUP*"
  # get_profiles
}

function build_infrastructure() {
  echo "Ensure we have our automator script ready"
  make
  adb install -r bin/profilinginstrumentation-debug-free-unsigned.apk
}

adb root
build_infrastructure
if false
then
  # No need to to this if we are generating boot.oat on host.
  echo "Set property exact.profile to 'generate'"
  adb shell setprop exact.profiling generate
  if [[ "$1" != "--noboot" ]]
  then
    echo "recompile boot.oat"
    recompile_boot
  fi
fi
get_profiles
run_apct
APPNAME="com.rovio.angrybirdstransformers"
echo "Uninstalling $APPNAME (may fail)"
sleep 10
echo "Run Benchmarks"
$RUN_BENCHMARK RUNS=1 AnTuTu-6.0.1 CaffeineMarkRR jArtBand-1.03 IcyRocks-1.1 QuadrantPro-2.1.1
get_profiles
echo "unlock screen: power off"
adb shell input keyevent KEYCODE_POWER
sleep 2
echo "unlock screen: menu"
adb shell input keyevent KEYCODE_MENU
sleep 1
adb shell input keyevent KEYCODE_MENU
echo "Uninstalling $APPNAME (okay to fail uninstall)"
adb uninstall $APPNAME
get_profiles
echo "Run UX script"
adb shell am instrument -w com.intel.profilinginstrumentation/android.test.InstrumentationTestRunner
adb uninstall com.rovio.angrybirdstransformers
