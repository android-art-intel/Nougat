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

package OptimizationTests.OSR.FormBottomLoops_TryCatchNested_01;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 *
 * Simplest cases for loops that are already bottom tested
 *
 **/

public class Main {
    final int iterations = 1000*1100;

    final int test1(int n) {

        for (int j = 0; j < 10; j++) {
            for (int i = 0; i < iterations; i++) {
                n += i/2 + j/2 + 5*i -6*j +7*i + 8*j;
            }
            n += j/2;
        }

        return n;
    } 

    final int test2(int n) {
        int k = n;
        int i = 0;
        while ( i++ < iterations) {
            for ( k = n; k < 100; k++) {
                n += 5 + - k/3;
                k++;
            }
        }

        return n + k + i;
    }


    final int test3(int n) {
        int testVar = 0;

        do {
            for (int i = 0; i < 2; i++) {
                testVar += 5 + n/2 + i/2;
            }
            testVar += n + Integer.MAX_VALUE/3;

        } while (testVar <= iterations);
        return testVar + n;
    }


    final int test4(int n) {
        int testVar = 0;

        while (true) {
            for ( int i = 0; i++ < iterations; ) {
                testVar += 5 + n/2 + i/2 ;
            }
            if (testVar > iterations) {
                break;
            }
        }

        return testVar + n;
    }

    final int test5(int n) {
        int testVar = 0;

        for (long j = 0L; j < iterations ; j++) {
            for (int i = 0; i < iterations; i++) {
            }
        }


        return testVar;
    }

    final int $noinline$test1() {
        int res = 0;
        int n = 10;
        for (int j = 0; j < 1100; j++) {
            for (int i = 0; i < iterations; i++) {
                n += i/2 + j/2;
            }
            n += j/2;
        }


        res += n ;

        try {
            for (int j = 0; j < 1100; j++) {
                for (int i = 0; i < iterations; i++) {
                    n += i/2 + j/2;
                }
                n += j/2;
            }


            if (n > 0)
                throw new Exception("...");
        } catch (Exception e) {
            for (int j = 0; j < 1100; j++) {
                for (int i = 0; i < iterations; i++) {
                    n += i/2 + j/2;
                }
                n += j/2;
            }

            res ^= n ;
        } finally {
            for (int j = 0; j < 1100; j++) {
                for (int i = 0; i < iterations; i++) {
                    n += i/2 + j/2;
                }
                n += j/2;
            }

            res -= n ;
        }
        for (int j = 0; j < 1100; j++) {
            for (int i = 0; i < iterations; i++) {
                n += i/2 + j/2;
            }
            n += j/2;
        }

        res *= n ;

        return res;
    }

    final int $noinline$test2() {
        int res = 0;
        int n = 10;

        int k = n;
        int i = 0;
        while ( i++ < iterations) {
            for ( k = n; k < 100; k++) {
                n += 5 + - k/3;
                k++;
            }
        }


        res += n + k + i;

        try {

            k = n;
            i = 0;
            while ( i++ < iterations) {
                for ( k = n; k < 100; k++) {
                    n += 5 + - k/3;
                    k++;
                }
            }



            if (n + k + i > 0)
                throw new Exception("...");
        } catch (Exception e) {
            k = n;
            i = 0;
            while ( i++ < iterations) {
                for ( k = n; k < 100; k++) {
                    n += 5 + - k/3;
                    k++;
                }
            }


            res ^= n + k + i;
        } finally {
            k = n;
            i = 0;
            while ( i++ < iterations) {
                for ( k = n; k < 100; k++) {
                    n += 5 + - k/3;
                    k++;
                }
            }


            res -= n + k + i;
        }
        k = n;
        i = 0;
        while ( i++ < iterations) {
            for ( k = n; k < 100; k++) {
                n += 5 + - k/3;
                k++;
            }
        }


        res *= n + k + i;

        return res;
    }

    final int $noinline$test3() {
        int res = 0;
        int n = 10;



        int testVar = 0;

        do {
            for (int i = 0; i < iterations; i++) {
                testVar += 5 + n/2 + i/2;
            }
            testVar += n + Integer.MAX_VALUE/3;

        } while (testVar <= iterations);


        res += testVar + n;;

        try {
            testVar = 0;

            do {
                for (int i = 0; i < iterations; i++) {
                    testVar += 5 + n/2 + i/2;
                }
                testVar += n + Integer.MAX_VALUE/3;

            } while (testVar <= iterations);



            if (testVar + n > 0)
                throw new Exception("...");
        } catch (Exception  e) {
            testVar = 0;

            do {
                for (int i = 0; i < iterations; i++) {
                    testVar += 5 + n/2 + i/2;
                }
                testVar += n + Integer.MAX_VALUE/3;

            } while (testVar <= iterations);


            res ^= testVar + n;
        } finally {
            testVar = 0;

            do {
                for (int i = 0; i < iterations; i++) {
                    testVar += 5 + n/2 + i/2;
                }
                testVar += n + Integer.MAX_VALUE/3;

            } while (testVar <= iterations);


            res -= testVar + n;
        }
        testVar = 0;

        do {
            for (int i = 0; i < iterations; i++) {
                testVar += 5 + n/2 + i/2;
            }
            testVar += n + Integer.MAX_VALUE/3;

        } while (testVar <= iterations);


        res *= testVar + n;

        return res;
    }

    final int $noinline$test4() {
        int res = 0;
        int n = 10;

        int testVar = 0;

        while (true) {
            for ( int i = 0; i++ < iterations; ) {
                testVar += 5 + n/2 + i/2 ;
            }
            if (testVar > iterations) {
                break;
            }
        }

        res += testVar + n;

        try {
            testVar = 0;

            while (true) {
                for ( int i = 0; i++ < iterations; ) {
                    testVar += 5 + n/2 + i/2 ;
                }
                if (testVar > iterations) {
                    break;
                }
            }

            if (testVar + n > 0)
                throw new Exception("...");
        } catch (Exception e) {
            testVar = 0;

            while (true) {
                for ( int i = 0; i++ < iterations; ) {
                    testVar += 5 + n/2 + i/2 ;
                }
                if (testVar > iterations) {
                    break;
                }
            }

            res ^= testVar + n;
        } finally {
            testVar = 0;

            while (true) {
                for ( int i = 0; i++ < iterations; ) {
                    testVar += 5 + n/2 + i/2 ;
                }
                if (testVar > iterations) {
                    break;
                }
            }

            res -= testVar + n;
        }
        testVar = 0;

        while (true) {
            for ( int i = 0; i++ < iterations; ) {
                testVar += 5 + n/2 + i/2 ;
            }
            if (testVar > iterations) {
                break;
            }
        }

        res *= testVar + n;

        return res;
    }

    final int $noinline$test5() {
        int res = 0;
        int n = 10;

        int testVar = 0;

        for (long j = 0L; j < 1100; j++) {
            for (int i = 0; i < iterations; i++) {
            }
        }


        res += testVar;

        try {
            testVar = 0;

            for (long j = 0L; j < 1100; j++) {
                for (int i = 0; i < iterations; i++) {
                }
            }


            if (testVar > 0)
                throw new Exception("...");
        } catch (Exception e) {
            testVar = 0;

            for (long j = 0L; j < 1100; j++) {
                for (int i = 0; i < iterations; i++) {
                }
            }


            res ^= testVar;
        } finally {
            testVar = 0;

            for (long j = 0L; j < 1100; j++) {
                for (int i = 0; i < iterations; i++) {
                }
            }


            res -= testVar;
        }
        testVar = 0;

        for (long j = 0L; j < 1100; j++) {
            for (int i = 0; i < iterations; i++) {
            }
        }


        res *= testVar;

        return res;
    }

    public void $noinline$runTests() {
        System.out.println($noinline$test1());
        System.out.println($noinline$test2());
        System.out.println($noinline$test3());
        System.out.println($noinline$test4());
        System.out.println($noinline$test5());
    }

    public static void main(String[] args)
    {
        new Main().$noinline$runTests();
    }


}
