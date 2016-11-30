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

package OptimizationTests.LoopInformation.LoopFloatLT;

import OptimizationTests.LoopInformation.shared.*;
import static OptimizationTests.LoopInformation.shared.TestUtils.hash;

public class Main {
    public int test1(float x) {
        float f = x;
        int res = (int) x;
        for (float i = 0f; i < 1000f; i++) {
            f *= 1.003f;
            res <<= 3;
            res += res;
            res += i;
        }
        return hash(f, res);
    }

    public int test2(float x) {
        float f = x;
        int res = (int) x;
        for (float i = 0.05f; i < 1111.5f; i += 0.5f) {
            f *= 1.003f;
            res <<= 3;
            res += res;
            res += i;
        }
        return hash(f, res);
    }

    public int test3(float x) {
        float f = x;
        int res = (int) x;
        for (float i = 0.5f; i < 2000.5f; i += 0.25f) {
            f *= 1.003f;
            res <<= 3;
            res += res;
            res += i;
        }
        return hash(f, res);
    }

    public int test4(float x) {
        float f = x;
        int res = (int) x;
        for (float i = 0.25f; i < 2000.5f; i += 0.125f) {
            f *= 1.003f;
            res <<= 3;
            res += res;
            res += i;
        }
        return hash(f, res);
    }


    public int test5(float x) {
        float f = x;
        int res = (int) x;
        for (float i = 0.1f; i < 1111.1f; i += 0.1f) {
            f *= 1.03f;
            res <<= 3;
            res += res;
            res += i;
        }
        return hash(f, res);
    }

    public int test6(float x) {
        float f = x;
        int res = (int) x;
        for (float i = 0.01f; i < 1111.11f; i += 0.01f) {
            f *= 1.01f;
            res <<= 3;
            res += res;
            res += i;
        }
        return hash(f, res);
    }

    public int test7(float x) {
        float f = x;
        int res = (int) x;
        for (float i = 0.001f; i < 1111.111f; i += 0.001f) {
            f *= 1.03f;
            res <<= 3;
            res += res;
            res += i;
        }
        return hash(f, res);
    }

    public int test8(float x) {
        float f = x;
        int res = (int) x;
        for (float i = 31.5f; i < 2425.5f; i += 10.5f) {
            f *= 1.03f;
            res <<= 3;
            res += res;
            res += i;
        }
        return hash(f, res);
    }

    public int test9(float x) {
        float f = x;
        int res = (int) x;
        for (float i = 0f; i < 1126.125f; i += 125.125f) {
            f *= 1.03f;
            res <<= 3;
            res += res;
            res += i;
        }
        return hash(f, res);
    }

    public static void main(String[] args) {
        TestUtils.runTests(new Main(), 1.00001f);
    }
}
