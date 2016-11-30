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

package OptimizationTests.NonTemporalMove.VolatileLong_001;

// 2 non-temporal moves expected. Assign volatile variable to tab[i]

public class Main {
    final int iterations = 133000*200;
    private volatile long jack = 0;

    public long checkSum(long[] tab, int n) {
        long s = 0;
        for (int i = 0; i < n ; i++) {
            s = s + tab[i];
        }
        return s;
    }

    public boolean testLoop(long[] tab) {
        new Thread(new Runnable() {
            public void run() {
                for(int i = 0; i < iterations; ++i) {
                    jack = 1;
                }
            }
        }).start();

        long count = 0;
        for(int i = 0; i < iterations; ++i) {
            long k = jack;
            tab[i] = k;

            if (k == 1) {
                count++;
                jack = 0;
            }
        }

        return checkSum(tab, iterations) == count;
    }

    public void test()
    {
        long[] tab = new long[iterations];

        if (testLoop(tab) == true)
            System.out.println("ok");
        else
            System.out.println("not ok");
    }

    public static void main(String[] args)
    {
        new Main().test();
    }

}

