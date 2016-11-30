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

package OptimizationTests.OSR.shared;

public class Cls2 {
    public boolean[] arr;
    public int count = 0;
    public Cls2(int n) {
        int arr_length = 0;
        if (n > 0) {
            arr_length = n;
        } else {
            arr_length = 0;
        }
        arr = new boolean[arr_length];
        for (int i = 0; i < arr.length; i++) {
            arr[i] = false;
        }
    }
    public int bar (int x) {
        return arr[x] ? x : 0;
    }

    // inlined
    public boolean[] getArr1() {
        return arr;
    }

    // was: not inlined 
    public boolean[] getArrNotInlined(int x) {
        arr[x] = false;
        System.out.println("Calling " + this.getClass().getName() + ".getArrNotInlined(" + x + ")");
        return arr;
    }


    // inlined 
    public boolean[] getArrInlined(int x) {
        count = count/x;
        return arr;
    }
}


