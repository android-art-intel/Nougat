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

package OptimizationTests.LoopInformation.LoopIntMinMax;

import OptimizationTests.LoopInformation.shared.*;

public class Main {

    public int test1(int x) {
        int res = x;
        for (int i = Integer.MAX_VALUE - 10000; i < Integer.MAX_VALUE; i++) {
            res <<= 3;
            res += res;
            res += i;
        }
        return res;
    }

    public int test2(int x) {
        int res = x;
        if (res > 0)
            return -1;
        for (int i = Integer.MAX_VALUE - 1000; i <= Integer.MAX_VALUE; i++) {
            res <<= 3;
            res += res;
            res += i;
        }
        return res;
    }

    public int test3(int x) {
        int res = x;
        for (int i = Integer.MIN_VALUE + 10000; i > Integer.MIN_VALUE; i--) {
            res <<= 3;
            res += res;
            res += i;
        }
        return res;
    }

    public int test4(int x) {
        int res = x;
        if (res > 0)
            return -1;
        for (int i = Integer.MIN_VALUE + 10000; i >= Integer.MIN_VALUE; i--) {
            res <<= 3;
            res += res;
            res += i;
        }
        return res;
    }

    public static void main(String[] args) {
        TestUtils.runTests(new Main(), 10);
    }
}
