/*
 * Copyright (C) 2015 Intel Corporation
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
 */

#include "aging_table.h"

#include "base/stringprintf.h"
#include "mem_map.h"
#include "mirror/object-inl.h"
#include "base/logging.h"

namespace art {
namespace gc {
namespace accounting {

// Keep the same object alignment as bump pointer space.
static constexpr size_t kBumpPointerSpaceAlignment = 8;

AgingTable::AgingTable(MemMap* mem_map, size_t table_size, uint8_t* space_begin,
                       uint8_t* space_end, const std::string& name)
    : mem_map_(mem_map),
      size_(table_size),
      space_begin_(space_begin),
      space_end_(space_end),
      name_(name) {
  begin_ = reinterpret_cast<uint8_t*>(mem_map->Begin());
  CHECK(begin_ != nullptr);
  CHECK_NE(size_, 0U);
}

AgingTable::~AgingTable() {
  // Destroys MemMap via std::unique_ptr<>.
}

AgingTable* AgingTable::Create(const std::string& name, size_t capacity, uint8_t* space_begin,
                               uint8_t* space_end) {
  const size_t table_size = capacity / kBumpPointerSpaceAlignment;
  std::string error_msg;
  std::unique_ptr<MemMap> mem_map(MemMap::MapAnonymous(name.c_str(), nullptr, table_size,
                                                       PROT_READ | PROT_WRITE, false, false,
                                                       &error_msg));
  if (UNLIKELY(mem_map.get() == nullptr)) {
    LOG(ERROR) << "Failed to allocate aging table " << name << ": " << error_msg;
    return nullptr;
  }
  return CreateFromMemMap(name, mem_map.release(), capacity, space_begin, space_end);
}

AgingTable* AgingTable::CreateFromMemMap(const std::string& name, MemMap* mem_map,
                                    size_t capacity, uint8_t* space_begin, uint8_t* space_end) {
  CHECK(mem_map != nullptr);
  const size_t table_size = capacity / kBumpPointerSpaceAlignment;
  return new AgingTable(mem_map, table_size, space_begin, space_end, name);
}

uint8_t AgingTable::GetObjectAge(mirror::Object* obj) {
  if (UNLIKELY(obj == nullptr)) {
      return 0;
  }
  size_t offset = OffsetFromObject(obj);
  return *(begin_ + offset);
}

void AgingTable::IncreaseObjectAge(mirror::Object* obj, uint8_t old_age) {
  if (UNLIKELY(obj == nullptr)) {
      return;
  }
  size_t offset = OffsetFromObject(obj);
  *(begin_ + offset) = old_age + 1;
}

size_t AgingTable::OffsetFromObject(mirror::Object* obj) {
  uint8_t* obj_ptr = reinterpret_cast<uint8_t*>(obj);
  CHECK_LE(space_begin_, obj_ptr);
  CHECK_GT(space_end_, obj_ptr);
  size_t offset = (obj_ptr - space_begin_) / kBumpPointerSpaceAlignment;
  CHECK_LT(offset, size_) << "object: " << obj_ptr << ", space: " << space_begin_ << ", " << space_end_;
  return offset;
}

void AgingTable::Reset() {
  CHECK(mem_map_.get() != nullptr) << "backing storage for this aging table does not exist";
  mem_map_->MadviseDontNeedAndZero();
}

}  // namespace accounting
}  // namespace gc
}  // namespace art
