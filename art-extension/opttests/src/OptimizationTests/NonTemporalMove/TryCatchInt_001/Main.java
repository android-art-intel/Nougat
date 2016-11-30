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

package OptimizationTests.NonTemporalMove.TryCatchInt_001;

// 7 non temporal moves expected in test(): testLoop() is used 5 times, inlined. 
// NTM generated 2 times instead of 1 in try and finally blocks

public class Main {
    final int iterations = 133000;

    public int checkSum(int[] tab, int n) {
        int s = 0;
        for (int i = 0; i < n ; i++) {
            s = s + tab[i];
        }
        return s;
    }

    final int testLoop(int[] tab) {
        for (int j = 0; j < iterations; j++) {
            tab[j] = j;
        }
        return checkSum(tab, iterations);
    }


    final void test()
    {
        int[] tab = new int[iterations];

        int res = 0;

        res += testLoop(tab); // Before try-catch.

        try {
            if (testLoop(tab) > 0)
                throw new Exception("..."); // In Try section
        } catch (Exception e) {
            res ^= testLoop(tab); // In catch secton.
        } finally {
            res -= testLoop(tab); // in Finally section.
        }

        res *= testLoop(tab); // After try-catch.

        System.out.println(res);
    }

    public static void main(String[] args)
    {
        new Main().test();
    }

}