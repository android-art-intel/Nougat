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

package OptimizationTests.FormBottomLoops.TryCatchConditions_01;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 *
 * Simplest cases for loops that are already bottom tested
 *
 **/

public class Main {
    final int iterations = 1100;

    /* Positive:  if inside loop */
    final int test1(int n) {
        int testVar = 0;

        for (int i = 0; i < iterations; i++) {
            testVar += 5 -  n/2 + i/2;
            if (testVar > 50) {
                n +=  i + testVar;
            } else {
                n +=  i - testVar;
            }

        }

        return testVar + n;
    } 

    /* Positive:  if inside loop, i is modified inside if */
    final int test2(int n) {
        int testVar = 0;
        int additionalVar = 1;

        for (int i = 0; i < iterations; i++) {
            if (i > n) {
                n += 10;
                i = i + additionalVar;
                additionalVar += 3;
            } else {
                additionalVar += 1;
            }
            testVar +=  additionalVar;
        }

        return testVar + n;
    }


    /* positive, Loop gate passed, no more limitations on basic blocks number */
    final int test3(int n, boolean[] bb) {
        int testVar = 0;

        int i = 0; 
        while (i < iterations + n) {
                if (i > 10) {
                    bb[i%bb.length] = true;
                } else {
                        n++;
                        bb[i%bb.length] = false;
                    }
                }


        return testVar;
    }


    /* Positive:  if outside loop */
    final int test4(int n, boolean[] bb) {
        int testVar = 0;
        int additionalVar1 = 1;

        if (n + testVar > 9 ) {
            for (int i = 0; i < iterations; i++) {
                i = i + additionalVar1;
                bb[i%bb.length] = (i % 2 == 1);

            }
        }

        return testVar + additionalVar1;
    }


/* Negative:  if inside loop, more than one back edge */
    final int test5(int n) {
        int testVar = 0;
        int additionalVar1 = 1;


        int i = 0; 
        while (i < iterations) {
            i++;
            testVar += 5 -  n/2 + i/2 + additionalVar1;
            if (i > n) {
                i++;
                testVar += 3;
            } else {
                i = i + additionalVar1;
            }

        }

        return testVar + additionalVar1;
    }

    /* Positive:  switch inside loop */
    final int test6(int n) {
        int testVar = 0;

        for (int i = 0; i < iterations; i++) {
            testVar += 5 -  n/2 + i/2 ;
            switch (i) {
            case 0:
                testVar +=  n;
                break;
            }

        }

        return testVar + n;
    }


    final int $noinline$test1() {
        int res = 0;
        int n = 10;

        res += test1(n) ;

        try {
            if (test1(n) > 0) // 1
                throw new Exception("...");
        } catch (Exception e) {
            res ^= test1(n) ; // 0
        } finally {
            res -= test1(n) ; // 2!
        }

        res *= test1(n) ; // 1

        return res;
    }

    final int $noinline$test2() {
        int res = 0;
        int n = 10;

        res += test2(n);

        try {
            if (test2(n) > 0)
                throw new Exception("...");
        } catch (Exception e) {
            res ^= test2(n);
        } finally {
            res -= test2(n);
        }

        res *= test2 (n);

        return res;
    }

    final int $noinline$test3() {
        int res = 0;
        int n = 10;
        boolean[] bb = new boolean[n];

        res += test3(n, bb);

        try {
            if (test3(n, bb) > 0)
                throw new Exception("...");
        } catch (Exception e) {
            res ^= test3(n, bb);
        } finally {
            res -= test3(n, bb);
        }

        res *= test3 (n, bb);

        return res;
    }

    final int $noinline$test4() {
        int res = 0;
        int n = 10;
        boolean[] bb = new boolean[n];

        res += test4(n, bb);

        try {
            if (test4(n, bb) > 0)
                throw new Exception("...");
        } catch (Exception e) {
            res ^= test4(n, bb);
        } finally {
            res -= test4(n, bb);
        }

        res *= test4 (n, bb);

        return res;
    }

    final int $noinline$test5() {
        int res = 0;
        int n = 10;

        res += test5(n);

        try {
            if (test5(n) > 0)
                throw new Exception("...");
        } catch (Exception e) {
            res ^= test5(n);
        } finally {
            res -= test5(n);
        }

        res *= test5 (n);

        return res;
    }

    final int $noinline$test6() {
        int res = 0;
        int n = 10;

        res += test6(n) ;

        try {
            if (test6(n) > 0) // 1
                throw new Exception("...");
        } catch (Exception e) {
            res ^= test6(n) ; // 0
        } finally {
            res -= test6(n) ; // 2!
        }

        res *= test6(n) ; // 1

        return res;
    }


    public void $noinline$runTests() {
        System.out.println($noinline$test1());
        System.out.println($noinline$test2());
        System.out.println($noinline$test3());
        System.out.println($noinline$test4());
        System.out.println($noinline$test5());
        System.out.println($noinline$test6());
    }

    public static void main(String[] args)
    {
        new Main().$noinline$runTests();
    }


}