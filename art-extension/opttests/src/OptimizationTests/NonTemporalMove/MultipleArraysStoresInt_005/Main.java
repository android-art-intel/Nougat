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

package OptimizationTests.NonTemporalMove.MultipleArraysStoresInt_005;

// M: no non temporal move expected because of BoundsCheck
// N: non temporal move is applied because BoundsCheck is eliminated

public class Main {                                                                                                                                                   


    class A {
        public int[] arr1 = new int [iterations];
        public int[] arr2 = new int [iterations];
    
        public int[] testLoop(int[] tab) {
        for (int i = 0; i < iterations; i++) {
            tab[i] = i + 1;
            arr1[i] = i * 2;
            tab[i] = i * 3;
            arr1[i] = i * 4;
            arr2[i] = i * 5;
        }
        arr1[iterations/2] = tab[10];
        tab[11] = arr1[5] + arr2[5];
        return tab;
    }

    }


    final int iterations = 0x40000;

    public long checkSum(int[] tab, int n) {
        long s = 0;
        for (int i = 0; i < n ; i++) {
            s = s + tab[i];
        }
        return s;
    }


    public void test()
    {
        int[] tab1 = new int [iterations];
        A a = new A();
        System.out.println(a.testLoop(a.arr1)[11]);
        System.out.println(a.arr1[11]);
    }

    public static void main(String[] args)
    {
        new Main().test();
    }

}  

