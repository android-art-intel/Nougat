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

function build_infrastructure() {
  echo "Ensure we have our automator script ready"
  make
  adb install -r bin/initialization-debug-free-unsigned.apk
}

adb root
build_infrastructure
adb shell am instrument -w com.intel.initialization/android.test.InstrumentationTestRunner
