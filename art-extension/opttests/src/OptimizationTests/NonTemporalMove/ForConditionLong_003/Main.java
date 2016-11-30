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

package OptimizationTests.NonTemporalMove.ForConditionLong_003;

// 2 non-temporal move expected: 1 if condition in the loop

public class Main {
    final int iterations = 133000;

    public long $noinline$checkSum(long[] tab, int n) {
        long s = 0;
        for (int i = 0; i < n ; i++) {
            s = s + tab[i];
        }
        return s;
    }

    public long $noinline$testLoop(long[] tab) {
        for (int j = 0; j < iterations; j++) {
            tab[j] = j;
            if (j > 4)
                tab[j] = j + 1;
        }
        return $noinline$checkSum(tab, iterations);
    }

    public void $noinline$test()
    {
        long[] tab = new long[iterations];
        System.out.println($noinline$testLoop(tab));
    }

    public static void main(String[] args)
    {
        new Main().$noinline$test();
    }

}

