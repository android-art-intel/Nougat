/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef _SPACE_RECORD_H_
#define _SPACE_RECORD_H_

extern "C" {
#include "gcspy_d_utils.h"
}

#include "base/logging.h"

namespace art {

namespace gc {

namespace space {
class Space;
class ContinuousSpace;
class BumpPointerSpace;
} // namespace space

namespace gcspy {

class SpaceRecord {
 public:
  SpaceRecord(space::Space *space, int blockSize, int statsStructSize);

  ~SpaceRecord() {
    free(area_.stats);
  }

  void Init(uintptr_t start, uintptr_t end, int blockSize, int statsStructSize);

  space::Space *GetSpace() { return space_; }

  uintptr_t Begin() {
    return (uintptr_t)area_.start;
  }

  uintptr_t End() {
    return (uintptr_t)area_.end;
  }

  size_t GetTileNum() {
    return area_.blockNum;
  }

  size_t GetSize() {
    return area_.end - area_.start;
  }

  void *GetTile(int index) {
    return (void *)gcspy_d_utils_get_stats_struct(&area_, index, 0);
  }

  bool AddrInRange(char *addr) {
    return (gcspy_d_utils_addr_in_range(&area_, addr) != 0) ? true : false;
  }

  int GetTileIndex(char *addr) {
    return gcspy_d_utils_get_index(&area_, addr);
  }

  char *GetTileAddr(int index) {
    return gcspy_d_utils_get_addr(&area_, index);
  }

  void Reset();

  std::string GetRangeString(int index)  {
    char tmp[256];
    gcspy_dUtilsRangeString(&area_, index, tmp);
    return std::string(tmp);
  }

  void AddSingle(uintptr_t addr, int offset) {
    gcspy_dUtilsAddSingle(&area_, (char *)addr, offset);
  }

  void SetPercVal(uintptr_t start, uintptr_t end, int offset, int val) {
    gcspy_dUtilsSetPercVal(&area_, (char *)start, (char *)end, offset, val);
  }

  void SetPerc(uintptr_t start, uintptr_t end, int offset) {
    gcspy_dUtilsSetPerc(&area_, (char *)start, (char *)end, offset);
  }

  void SetPerc(uintptr_t end, int offset) {
    gcspy_dUtilsSetPerc(&area_, area_.start, (char *)end, offset);
  }

  void SetPerc(int offset) {
    gcspy_dUtilsSetPerc(&area_, area_.start, area_.end, offset);
  }

  void SetValue(uintptr_t start, uintptr_t end, int offset, int value) {
    gcspy_dUtilsSet(&area_, (char *)start, (char *)end, offset, value);
  }

  void UpdateEnumDesc(uintptr_t start, uintptr_t end, int offset, int value) {
    gcspy_dUtilsUpdateEnumDesc(&area_, (char *)start, (char *)end, offset, value);
  }

 private:
  space::Space *space_;
  gcspy_d_utils_area_t area_;
};


} // namespace gcspy
} // namespace gc
} // namespace art

#endif // _SPACE_RECORD_H_
