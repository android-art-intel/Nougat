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

package OptimizationTests.ShortMethodsInliningNonVirtualInvokes.InvokeDirectIChar_001;
class Test {
    char thingies = 'a';

    private char getThingies() {
        return thingies;
    }

    private void setThingies(char newThingies) {
        thingies = newThingies;
    }

    public char gimme() {
       return (char)(getThingies() + 1);
    }

    public void hereyouare(char newThingies) {
        setThingies((char)(newThingies - 1));
    }
}
