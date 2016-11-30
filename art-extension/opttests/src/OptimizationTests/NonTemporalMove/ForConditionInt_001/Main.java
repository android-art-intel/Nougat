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

package OptimizationTests.NonTemporalMove.ForConditionInt_001;

// N: 2 non-temporal move expected: 1 if condition in the loop
// M: no non-temporal move expected

public class Main {
    final int iterations = 133000;

    public int $noinline$checkSum(int[] tab, int n) {
        int s = 0;
        for (int i = 0; i < n ; i++) {
            s = s + tab[i];
        }
        return s;
    }

    // Formally, NTM should not apply here, because the loop contains the array get. In this particular case, the compiler can prove that tab[j] is actually equal to j (because it was set just above).
    public int $noinline$testLoop(int[] tab) {
        for (int j = 0; j < iterations; j++) {
            tab[j] = j;
            if (j > 4)
                tab[j] = tab[j] + 1; // Array get!!!

        }
        return $noinline$checkSum(tab, iterations);
    }

    public void $noinline$test()
    {
        int[] tabInt = new int[iterations];
        System.out.println($noinline$testLoop(tabInt));
    }

    public static void main(String[] args)
    {
        new Main().$noinline$test();
    }

}

