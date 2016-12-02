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

package OptimizationTests.ShortMethodsInliningNonVirtualInvokes.InvokeSuperRangeAObjectThrowNullSet_001;
// The test checks that stack after NullPointerException occurs is correct despite inlining
class SuperTest {
    static Foo[] thingiesArray;

    SuperTest(int iterations) {
        this.thingiesArray = new Foo[iterations];
    }

    public Foo getThingies(Foo[] arr, int i, Foo f1, Foo f2, Foo f3, Foo f4, Foo f5, Foo f6) {
        return arr[i];
    }

    public void setThingies(Foo[] arr, Foo newThingy, int i, Foo f1, Foo f2, Foo f3, Foo f4, Foo f5, Foo f6) {
        arr[i] = newThingy;
    }
}
