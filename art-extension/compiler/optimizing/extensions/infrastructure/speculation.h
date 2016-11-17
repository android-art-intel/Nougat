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

#ifndef ART_COMPILER_OPTIMIZING_EXTENSIONS_INFRASTRUCTURE_SPECULATION_H_
#define ART_COMPILER_OPTIMIZING_EXTENSIONS_INFRASTRUCTURE_SPECULATION_H_

namespace art {

enum SpeculationRecoveryApproach {
  kRecoveryAny,                         // Pick any recovery method.
  kRecoveryNotNeeded,                   // No recovery needed - prediction is always right.
  kRecoveryDeopt,                       // Deoptimize to interpreter - this also ensures
                                        // profiling happens.
  kRecoveryCodeVersioning,              // Version the code by creating separate control flow.
  kRecoveryFault,                       // Fault on mis-speculation. This is useful for experiments.
  kRecoveryCodeVersioningWithCounting,  // Version the code by creating separate control flow but
                                        // count mispredict rate.
};
std::ostream& operator<<(std::ostream& os, const SpeculationRecoveryApproach& rhs);

enum VersioningApproach {
  kVersioningAny,    // Pick any versioning approach.
  kVersioningLocal,  // Apply versioning locally just for instruction in question.
  kVersioningRange,  // Apply versioning to part of graph up to last similar candidate.
  kVersioningLoop,   // Apply loop versioning.
};
std::ostream& operator<<(std::ostream& os, const VersioningApproach& rhs);

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_EXTENSIONS_INFRASTRUCTURE_SPECULATION_H_
