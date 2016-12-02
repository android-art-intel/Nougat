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

package OptimizationTests.ShortMethodsInliningNonVirtualInvokes.InvokeSuperRangeIByte_001;
class Test extends SuperTest {

    public byte getThingies(byte c1, byte c2, byte c3, byte c4, byte c5, byte c6) {
        return super.getThingies(c1, c2, c3, c4, c5, c6);
    }

    public void setThingies(byte newThingies, byte c1, byte c2, byte c3, byte c4, byte c5, byte c6) {
        super.setThingies(newThingies, c1, c2, c3, c4, c5, c6);
    } 
}
