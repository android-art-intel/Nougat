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

package OptimizationTests.BBSimplifier.SimpleIfElse;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    /*Simple test*/
    private int $noinline$testSimpleIfElse(int p) {
        int[] a = new int[100];
        int res = a[p];
        for (int i = 0; i < 1000000; i++) {
            double x = 0;
            if (res * 70 < 10) {
                x += Math.cos(res);
            } else {
                x -= Math.cos(res);
            }
            res += i;
        }
        return res;
    }

    public void RunTests()
    {
        System.out.println($noinline$testSimpleIfElse(1));
    }

    public static void main(String[] args)
    {
        new OptimizationTests.BBSimplifier.SimpleIfElse.Main().RunTests();
    }


}

