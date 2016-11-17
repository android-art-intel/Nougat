/*
 * Copyright (C) 2016 Intel Corporation
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

#ifndef VENDOR_INTEL_ART_EXTENSION_OPT_INFRASTRUCTURE_PASS_OPTION_H_
#define VENDOR_INTEL_ART_EXTENSION_OPT_INFRASTRUCTURE_PASS_OPTION_H_

#include "dex/pass_manager.h"
#include "driver/compiler_driver.h"
#include "driver/compiler_options.h"
#include "optimization.h"
#include "scoped_thread_state_change.h"

namespace art {

class Compiler;
class CompilerDriver;
class HOptimization;

/**
 * Structure abstracting pass option values.
 */
template <typename T>
class PassOption {
 public:
  PassOption(const HOptimization* optimization,
             const CompilerDriver* driver,
             const std::string& option_name,
             const T default_value) {
    DCHECK(driver != nullptr);
    ScopedObjectAccess soa(Thread::Current());

    bool result = ReadUserDefinedOption(driver, optimization, option_name, value);
    if (!result) {
      value = default_value;
    }
  }

  /**
   * @brief Returns option value.
   */
  T GetValue() const {
    return value;
  }

 private:
  static bool ReadUserDefinedOption(const CompilerDriver* driver,
                                    const HOptimization* optimization,
                                    const std::string& option_name,
                                    bool& value) {
    std::string user_option;

    bool result = ReadUserDefinedOptionString(driver, optimization,
      option_name, user_option);

    if (result) {
      const char* option_ptr = user_option.c_str();
      DCHECK_EQ(*option_ptr, '\0');
      char* end_ptr = nullptr;
      int64_t int_value = strtoll(option_ptr, &end_ptr, 0);
      value = (int_value != 0);
      DCHECK(end_ptr != nullptr);
      return true;
    }
    return false;
  }

  static bool ReadUserDefinedOption(const CompilerDriver* driver,
                                    const HOptimization* optimization,
                                    const std::string& option_name,
                                    int64_t& value) {
    std::string user_option;

    bool result = ReadUserDefinedOptionString(driver, optimization,
      option_name, user_option);

    if (result) {
      const char* option_ptr = user_option.c_str();
      DCHECK_EQ(*option_ptr, '\0');
      char* end_ptr = nullptr;
      value = strtoll(option_ptr, &end_ptr, 0);
      DCHECK(end_ptr != nullptr);
      return true;
    }
    return false;
  }

  static bool ReadUserDefinedOptionString(const CompilerDriver* driver,
                                          const HOptimization* optimization,
                                          const std::string& option_name,
                                          std::string& option_value) {
    const PassManagerOptions* pass_options = driver->GetCompilerOptions().GetPassManagerOptions();
    const std::string& options = pass_options->GetOverriddenPassOptions();
    const size_t options_len = options.size();

    // Check whether any user options passed.
    if (options_len == 0) {
      return false;
    }

    const char* pass_name = optimization->GetPassName();
    const size_t pass_name_len = strlen(pass_name);
    const size_t min_option_size = 4;  // 2 delimiters, 1 option name, 1 option.
    size_t search_pos = 0;

    // If there is no room for pass options, exit early.
    if (options_len < pass_name_len + min_option_size) {
      return false;
    }

    do {
      search_pos = options.find(pass_name, search_pos);

      // Check if we found this pass name in rest of string.
      if (search_pos == std::string::npos) {
        // No more options for this pass.
        break;
      }

      // The string contains the pass name. Now check that there is
      // room for the options: at least one char for option name,
      // two chars for two delimiter, and at least one char for option.
      if (search_pos + pass_name_len + min_option_size >= options_len) {
        // No more options for this pass.
        break;
      }

      // Update the current search position to not include the pass name.
      search_pos += pass_name_len;

      // The format must be "PassName:SettingName:#" where # is the option.
      // Thus look for the first ":" which must exist.
      if (options[search_pos] != ':') {
        // Missing delimiter right after pass name.
        continue;
      } else {
        search_pos += 1;
      }

      // Now look for the actual option by finding the next ":" delimiter.
      const size_t option_name_pos = search_pos;
      size_t option_pos = options.find(':', option_name_pos);

      if (option_pos == std::string::npos) {
        // Missing a delimiter that would capture where option starts.
        continue;
      } else if (option_pos == option_name_pos) {
        // Missing option thus did not move from option name.
        continue;
      } else {
        // Skip the delimiter.
        option_pos += 1;
      }

      // Look for the terminating delimiter which must be a comma.
      size_t next_configuration_separator = options.find(',', option_pos);
      if (next_configuration_separator == std::string::npos) {
        next_configuration_separator = options_len;
      }

      // Prevent end of string errors.
      if (next_configuration_separator == option_pos) {
        continue;
      }

      std::string option_name_parsed =
        options.substr(option_name_pos, option_pos - option_name_pos - 1);

      if (option_name_parsed == option_name) {
        // Get the actual option value.
        option_value =
          options.substr(option_pos, next_configuration_separator - option_pos);
        return true;
      }
      search_pos = next_configuration_separator;
    } while (true);

    return false;
  }

  // Option value.
  T value;
};
}  // namespace art

#endif  // VENDOR_INTEL_ART_EXTENSION_OPT_INFRASTRUCTURE_PASS_OPTION_H_
