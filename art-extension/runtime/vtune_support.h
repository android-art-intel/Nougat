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

#ifndef VTUNE_SUPPORT_H_
#define VTUNE_SUPPORT_H_

#include "oat_file.h"

namespace art {

/*
 * @brief Send compiled method's code to VTune.
 */
void SendMethodToVTune(const char* method_name,
                       const void* code,
                       size_t code_size,
                       const char* class_file_name = nullptr,
                       const char* source_file_name = nullptr,
                       const void* optimized_code_info_ptr = nullptr,
                       const DexFile* dex_file = nullptr,
                       const DexFile::CodeItem* code_item = nullptr);

}  // namespace art

#endif // VTUNE_SUPPORT_H_
