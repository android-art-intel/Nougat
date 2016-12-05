// Copyright (c) 2013 Adobe Systems Incorporated. All rights reserved.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "json.h"
#include "gcview_space.h"
#include "utils.h"

namespace art {
namespace gc {
namespace gcview {

#define ITERATE_DATA(__cmd__) \
  GCVIEW_ARRAY_ITERATE(&_data, Data*, the_data, __cmd__)

void GCviewSpace::updateModifiedFlags() {
  bool modified = false;
  ITERATE_DATA({
    the_data->updateModifiedFlag();
    if (the_data->isModified()) {
      modified = true;
    }
  });
  _modified = modified;
}

void GCviewSpace::updateModifiedFlags(bool modified) {
  ITERATE_DATA({
    the_data->updateModifiedFlag(modified);
  });
  _modified = modified;
}

void GCviewSpace::updatePrevValues() {
  ITERATE_DATA({
    the_data->updatePrevValue();
  });
}

Data* GCviewSpace::addData(Data* data) {
  GCVIEW_GUARANTEE(findData(data->getName(), false /* should succeed */) == nullptr,
                   "data with that name alredy exist");
  unsigned id = _data.add(data);
  data->setID(id);
  return data;
}

Data* GCviewSpace::findData(const char* name,
                      bool should_succeed) const {
  ITERATE_DATA({
    if (!strcmp(name, the_data->getName())) return the_data;
  });
  GCVIEW_GUARANTEE(!should_succeed, "data not found");
  return nullptr;
}

void GCviewSpace::writeJSONMetadata(JSONWriter *writer) const {
  {
    JSONObjectWriter y(writer);
    y.writePair("ID", _id);
    y.writePair("Name", _name);
    y.startPair("Data");
    {
      JSONArrayWriter z(writer, true /* add_newlines */);
      ITERATE_DATA({
        z.startElem();
        the_data->writeJSONMetadata(writer);
      });
    }
  }
}

void GCviewSpace::writeJSONData(JSONWriter *writer) const {
  if (_modified) {
    JSONArrayWriter y(writer, true /* add_newlines */);
    ITERATE_DATA({
        y.startElem();
        the_data->writeJSONData(writer);
      });
  } else {
    writer->writeNull();
  }
}

void GCviewSpace::validate() const {
  ITERATE_DATA({ the_data->validate(); });
}

GCviewSpace::GCviewSpace(const char* name)
    : _name(Utils::cloneStr(name)), _modified(false) { }

GCviewSpace::~GCviewSpace() {
  delete[] _name;
  if (_manipulator != nullptr) {
    delete _manipulator;
    _manipulator = nullptr;
  }
  ITERATE_DATA({ delete the_data; });
}

}    // namespace gcview
}    // namespace gc
}    // namespace art
