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

package OptimizationTests.FormBottomLoops.TryCatchFloatIterator_01;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;


public class Main {
    final int iterations = 1100;

    final float test1(int n) {
        float testVar = 0;

        for (float i = 1.2345f; i < Float.MAX_VALUE; i *= Float.MAX_VALUE/3 + 0.12345f) {
            testVar += 5 -  n/2 + i/2;

        }
        return testVar;

    } 

    final double test2(int n) {
        double testVar = 0;

        float i = 0.5f; 
        while (i < Float.MAX_VALUE ) {
            i++;
            i *= 1 + n/2 + (++i)/2;
            testVar += i;
        }

        return testVar;
    }


    final float test3(int n) {
        float testVar = 0.123f;

        for (float i = 0; i < iterations; i += n) {
            testVar += 5 -  n/(i - 11) + i/2;
        }

        return testVar + n;
    }

    final float $noinline$test1() {
        float res = 0;
        int n = 10;

        res += test1(n) ;

        try {
            if (test1(n) > 0)
                throw new Exception("...");
        } catch (Exception e) {
            res /= test1(n) ;
        } finally {
            res -= test1(n) ;
        }

        res *= test1(n) ;

        return res;
    }

    final float $noinline$test2() {
        float res = 0;
        int n = 10;

        res += test2(n);

        try {
            if (test2(n) > 0)
                throw new Exception("...");
        } catch (Exception e) {
            res /= test2(n);
        } finally {
            res -= test2(n);
        }

        res *= test2 (n);

        return res;
    }

    final float $noinline$test3() {
        float res = 0;
        int n = 10;

        res += test3(n);

        try {
            if (test3(n) > 0)
                throw new Exception("...");
        } catch (Exception e) {
            res /= test3(n);
        } finally {
            res -= test3(n);
        }

        res *= test3 (n);

        return res;
    }

    public void $noinline$runTests() {
        System.out.println($noinline$test1());
        System.out.println($noinline$test2());
        System.out.println($noinline$test3());
    }

    public static void main(String[] args)
    {
        new Main().$noinline$runTests();
    }


}