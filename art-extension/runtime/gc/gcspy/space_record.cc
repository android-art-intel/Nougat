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


#include "space_record.h"

#include <cstring>

#include "gc/space/space-inl.h"
#include "gc/space/bump_pointer_space-inl.h"

namespace art {

namespace gc {

namespace gcspy {

static char *AllocStats(size_t tileNum, size_t statsSize) {
  size_t len = tileNum * statsSize;
  char *tiles = (char *)malloc(len);
  if (tiles == nullptr) {
    LOG(ERROR) << "M&C Driver: could not allocate %d bytes for tile data";
  }
  memset(tiles, 0, len);
  return tiles;
}


SpaceRecord::SpaceRecord(space::Space *space, int blockSize, int statsStructSize)
    : space_(space) {
  //TODO: handle large object spaces, maybe one large object chunk with one gcspy_area_t?
  // gcspy handles only continuous spaces now
  uintptr_t start = 0u;
  uintptr_t end = 0u;

  CHECK(space->IsContinuousSpace());

  space::ContinuousSpace *cont_space = space->AsContinuousSpace();
  start = (uintptr_t)cont_space->Begin();
  end = (uintptr_t)cont_space->Limit();

  Init(start, end, blockSize, statsStructSize);
}

void SpaceRecord::Init(uintptr_t start, uintptr_t end, int blockSize, int statsStructSize) {
  if (start != end) {
    int tileNum = gcspy_dUtilsTileNum((char *)start, (char *)end, blockSize);
    char *tiles = AllocStats(tileNum, statsStructSize);
    gcspy_dUtilsInit(&area_, (char *)start, (char *)end, 0, blockSize,
                     tileNum, tiles, statsStructSize);
  }
}

void SpaceRecord::Reset() {
  uintptr_t end;
  if (space_->IsBumpPointerSpace()) {
    space::BumpPointerSpace *bpspace = space_->AsBumpPointerSpace();
    end = (uintptr_t)bpspace->Limit();
  } else {
    space::ContinuousSpace *cont_space = space_->AsContinuousSpace();
    end = (uintptr_t)cont_space->Limit();
  }
  if (end != (uintptr_t)area_.end) {
    int tileNum = gcspy_dUtilsTileNum(area_.start, (char *)end, area_.blockSize);
    area_.end = (char *)end;
    area_.blockNum = tileNum;
    if (area_.stats) {
      free(area_.stats);
    }
    area_.stats = AllocStats(tileNum, area_.statsStructSize);
  } else {
    memset(area_.stats, 0, area_.blockNum * area_.statsStructSize);
  }
}

} // namespace gcspy
} // namespace gc
} // namespace art
