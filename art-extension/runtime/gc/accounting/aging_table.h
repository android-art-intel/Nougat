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

#ifndef ART_RUNTIME_GC_ACCOUNTING_AGING_TABLE_H_
#define ART_RUNTIME_GC_ACCOUNTING_AGING_TABLE_H_

#include <memory>
#include <string>

namespace art {
class MemMap;

namespace mirror {
  class Object;
}  // namespace mirror

namespace gc {
namespace accounting {

class AgingTable {
 public:
  // Create and initialize an age_table with capacity. Storage is allocated with a MemMap.
  static AgingTable* Create(const std::string& name, size_t capacity,
                            uint8_t* space_begin, uint8_t* space_end);

  // Initialize a space bitmap using the provided mem_map as the live bits. Takes ownership of the
  // mem map. The address range covered starts at space_begin and is of size equal to space_capacity.
  // Objects are kAlignment-aligned.
  static AgingTable* CreateFromMemMap(const std::string& name, MemMap* mem_map, size_t capacity,
                                      uint8_t* space_begin, uint8_t* space_end);

  ~AgingTable();

  // Return age of an object.
  uint8_t GetObjectAge(mirror::Object* obj);
  // Increase age of an object by 1.
  void IncreaseObjectAge(mirror::Object* obj, uint8_t old_age);
  // Zero aging table.
  void Reset();

 private:
  AgingTable(MemMap* mem_map, size_t table_size, uint8_t* space_begin,
             uint8_t* space_end, const std::string& name);
  // Backing storage of aging table.
  std::unique_ptr<MemMap> mem_map_;
  // The aging table itself.
  uint8_t* begin_;
  // Size of the aging table.
  size_t size_;
  // The base address of the space, which corresponds to the first byte in this aging table.
  uint8_t* space_begin_;
  uint8_t* space_end_;
  // Name of this aging table.
  std::string name_;

  // Convert to address in age table from an object.
  size_t OffsetFromObject(mirror::Object* obj);
};

}  // namespace accounting
}  // namespace gc
}  // namespace art
#endif  // ART_RUNTIME_GC_ACCOUNTING_AGING_TABLE_H_
