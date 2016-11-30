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

package OptimizationTests.OSR.LoopPeeling_TryCatch_01;

import OptimizationTests.OSR.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    public final int iterations = 1000*10000;
    public static int iters = 1000*10000;
    public static int s_field1 = 33;
    private double i_field1 = 0.00001234500012345d;
    protected double i_field2 = 1.00001234500012345d;
    public double i_field3 = 2.00001234500012345d;
    public final static int s_final_field1 = 100;
    private volatile int i_volatile_field1 = 0;
    Cls1_ext c_e = new Cls1_ext();

    static int bar_inlined(int testVar) {
        int additionalVar = 1;
        for (int i = 0; i < iters; i++) {
            testVar += 5;
            additionalVar += i + 2*i - i*7 + i*4 + i*5 + i*6*i;
        }
        return testVar + additionalVar;
    }

    /* Peeling candidate: Custom, should not be peeled due to try-catch inside */
    public int test_04(int x) {
        int sum = 10000;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};
        int i = iterations;
        try {
            do {
                sum /= x;
                i =  i - 2;
                if (i < 4)
                    throw new Exception("Test");
            } while( i > 2);
        } catch (Exception ex) {
            sum++;
        }
        return sum;
    }

    /* Peeling candidate: Custom, should not be peeled due to try-catch inside */
    public int test_05(int x) {
        int sum = 100;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};
        int i = 10;
        try {
            do {
                sum /= x;
                int additionalVar = 1;
                for (int j = 0; j < iters; j++) {
                    sum += 15/x;
                    additionalVar += j + 2*j - j*7 + j*4 + j*5 + j*6*j;
                }

                sum += additionalVar;
                i =  i / 2;
                if (i < 4)
                    throw new Exception("Test");
            } while( i > 2);
        } catch (Exception ex) {
            sum++;
        }
        return sum;
    }

    /* Peeling candidate: Custom, should be peeled inside catch block?*/
    public int test_06(int x) {
        int sum = 10000;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};
        int i = 100;
        try {
            if (x > 0)
            throw new Exception("Test");
        } catch (Exception e) {
            do {
                sum /= x;
                int additionalVar = 1;
                for (int j = 0; j < iters; j++) {
                    sum += 15/x;
                    additionalVar += j + 2*j - j*7 + j*4 + j*5 + j*6*j;
                }
                sum += additionalVar;
                i =  i / 2;
            } while( i > 2);
        }
        return sum;
    }

    /* Peeling candidate: Custom, should be peeled as it is in finally section */
    public int test_07(int x) {
        int sum = 10000;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};
        int i = 100;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            sum++;
        } finally {
            do {
                sum /= x;
                int additionalVar = 1;
                for (int j = 0; j < iters; j++) {
                    sum += 15/x;
                    additionalVar += j + 2*j - j*7 + j*4 + j*5 + j*6*j;
                }
                sum += additionalVar;


                i =  i / 2;
            } while( i > 2);
        }
        return sum;
    }

    /* Peeling candidate: Custom, should be peeled: in catch block */
    public int test_08(int x) {
        int sum = 10000;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};
        int i = iterations;
        try {
            if (x > 0)
                throw new Exception("Test");
        } catch (Exception e) {
            do {
                sum /= x;
                i =  i - 2;
            } while( i > 2);
        }
        return sum;
    }

    /* Peeling candidate: Custom, should be peeled as it is in finally section */
    public int test_09(int x) {
        int sum = 10000;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};
        int i = iterations;
        try {
            if (x > 0)
                throw new Exception("Test");
        } catch (Exception e) {
            sum++;
        } finally {
            do {
                sum /= x;
                i =  i - 2;
            } while( i > 2);
        }
        return sum;
    }

    /* Peeling candidate: DivZero, should not be peeled due to try-catch inside */
    public int test_10(int x) {
        int sum = 1000*10000;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};
        int i = iterations;
        do {
            try {
                sum /= x;
            } catch (Exception ex) {
                sum += 1000;
            } finally {
                sum += 100;
            }
            i =  i -2;
        } while( i > 2);
        return sum;
    }

    /* Peeling candidate: DivZero, try-catch before */
    public int test_11(int x) {
        int sum = 10000;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};
        int i = iterations;
        try {
            if (x > 0)
            throw new Exception("Test");
        } catch (Exception e) {
            i++;
        }
        do {
            sum /= x;
            i =  i - 2;
        } while( i > 2);
        return sum;
    }

    /* Peeling candidate: DivZero, try-catch after */
    public int test_12(int x) {
        int sum = 10000;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};
        int i = iterations;
        do {
            sum /= x;
            i =  i - 2;
        } while( i > 2);
        try {
            if (x > 0)
            throw new Exception("Test");
        } catch (Exception e) {
            i++;
        }
        return sum;
    }

    public void runTests() {
        Class c = new Main().getClass();
        Method[] methods = c.getDeclaredMethods();
        Method tempMethod;
        for (int i = 1; i < methods.length; i++) {
            int j = i;
            while (j > 0 &&  methods[j-1].getName().compareTo(methods[j].getName()) > 0) {
                tempMethod = methods[j];
                methods[j] = methods[j-1];
                methods[j-1] = tempMethod;
                j = j - 1;
            }
        }

        Object[] arr = {null};
        for (Method m: methods){
            if (m.getName().startsWith("test")){
                try {
                    String names[] = c.getPackage().getName().split("\\.");
                    String testName = names[names.length-1];
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: "+ m.invoke(this, 10));
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static void main(String[] args)
    {
        new Main().runTests();
    }

}


