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

package OptimizationTests.FormBottomLoops.TryCatchDiv_01;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    private final static int LIMIT = 1200;

    final int test1(int n) {
        int a = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            a = i;
        }

        return a + i + n;
    }

    final int test2(int n) {
        int a = 0;
        int i = 0;
        int res = 0;

        while (true) {
            res += a;
            a += i + 6;
            i = i + 1;
            a += i + 6;
            if (i >= LIMIT) break;
            res += a;
        }

        return a + i + res + n;
    }

    final int test3(int n) {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a += i + 6;
            if (i >= LIMIT) break;
            b = a;
        }

        return a + b + i + n;
    }


    final int test4(int n) {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b = a;
            if (i >= LIMIT) break;
            a += i + 6;
            b = a;
        }

        return a + b + i + n;
    }

    final int test5(int n) {
        int i = 0;
        int a = 0;
        int b = 0;
        int res = 0;

        while (true) {
            res += a;
            if (i >= LIMIT) break;
            b = a;
            a = i++;
            res += a;
        }

        return a + b + i + res + n;
    }

    /* Positive:  switch inside loop */
    final int test6(int n) {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b = a;
            if (i >= LIMIT) break;
            a += i + 6;
            b = a;
        }

        return a + b + i + n;
    } 


    final int $noinline$test1() {
        int res = 0;
        int n = 10;

        res += test1(n) ;

        try {
            if (test1(n) > 0) //
                throw new Exception("...");
        } catch (Exception e) {
            res ^= test1(n) ; //
        } finally {
            res -= test1(n) ; //
        }

        res *= test1(n) ; //

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

        res += test3(n);

        try {
            if (test3(n) > 0)
                throw new Exception("...");
        } catch (Exception e) {
            res ^= test3(n);
        } finally {
            res -= test3(n);
        }

        res *= test3 (n);

        return res;
    }

    final int $noinline$test4() {
        int res = 0;
        int n = 10;

        res += test4(n);

        try {
            if (test4(n) > 0)
                throw new Exception("...");
        } catch (Exception e) {
            res ^= test4(n);
        } finally {
            res -= test4(n);
        }

        res *= test4 (n);

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