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

package OptimizationTests.LoopInformation.Regr01;

import OptimizationTests.LoopInformation.shared.*;

public class Main {

    // CAR-3170
    public int test1(int x) {
        int i = x;
        for (float f = 5f; f <= 10f; f += 1f) {
            i += 4;
        }
        return i / 4;
    }

    public int test2(int x) {
        int i = x;
        for (float f = 15f; f >= 10f; f -= 1f) {
            i += 4;
        }
        return i / 4;
    }

    public static void main(String[] args) {
        TestUtils.runTests(new Main(), 100);
    }
}
