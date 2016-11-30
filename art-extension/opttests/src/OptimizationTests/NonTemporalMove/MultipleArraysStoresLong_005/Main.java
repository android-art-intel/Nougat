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

package OptimizationTests.NonTemporalMove.MultipleArraysStoresLong_005;

// M: no non temporal move expected because of BoundsCheck
// N: non temporal move is applied because BoundsCheck is eliminated

public class Main {                                                                                                                                                   

    final int iterations = 0x40000;

    class A {
        public long[] arr1 = new long [iterations];
        public long[] arr2 = new long [iterations];
    
        public long[] testLoop(long[] tab) {
        for (int i = 0; i < iterations; i++) {
            tab[i] = (long)(i + 1);
            arr1[i] = (long)(i * 2);
            tab[i] = (long)i * 3;
            arr1[i] = (long)i * 4;
            arr2[i] = (long)i * 5;
        }
        arr1[iterations/2] = tab[10];
        tab[11] = arr1[5] + arr2[5];
        return tab;
    }

    }

    public void test()
    {
        long[] tab1 = new long [iterations];
        A a = new A();
        System.out.println(a.testLoop(a.arr1)[11]);
        System.out.println(a.arr1[11]);
    }

    public static void main(String[] args)
    {
        new Main().test();
    }

}  

