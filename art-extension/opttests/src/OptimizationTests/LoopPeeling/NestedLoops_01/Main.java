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

package OptimizationTests.LoopPeeling.NestedLoops_01;

import OptimizationTests.LoopPeeling.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    public final int iterations = 10000;
    public static int s_field1 = 33;
    private double i_field1 = 0.00001234500012345d;
    private int i_field2 = 10;
    public final static int s_final_field1 = 100;
    private volatile int i_volatile_field1 = 0;
    boolean i_b_field1 = false;
    boolean[] arr_b_field1 = new boolean[] {true, false, true, false};


    public static int foo_inlined(int x) {
        return x;
    }

    /* Peeling candidate: iget, nested loops */
    public double test_01(int x) {
        double sum = 0;
        i_field1 = 0.5d;
        for (int i = 0; i < iterations; i++) {
                do {
                    sum = i_field1;
                } while (sum > 1.25d); // 1 iteration only
                // will give opportunity to Value Propagattion Through Heap to hoist i_field, so next loop is empty
                i_field1 = x/100;
                do {
                    sum = i_field1;
                } while (sum > 1.25d); // 1 iteration only
                // this loop should be peeled
                do {
                    sum = i_field1;
                } while (sum > 1.25d); // 1 iteration only


                i_field1 = -1.27d;
        }

        return sum + s_field1 + i_field1;
    }

    /* Peeling candidate: aput + BoundsCheck, iget, nested loop -- depth 4 */
    public double test_02(int x) {
        int sum = 0;
        for (int i = 0; i < iterations; i++) {
                for (int j = 0; j < x; j++) {
                    for (int k = 0; k < x; k++) {
                        for (int l = 0; l < x; l++) {
                            arr_b_field1[(k + l)/iterations] = i_b_field1;
                        }
                    }
                }
            }
            

        return sum + (arr_b_field1[0]?1:0);
    }

    /* Peeling candidate: sget, iget, iput, sput, LoadClass, reading/writing to the same field twice, nested if-else inside loop, nested loop*/
    public double test_03(int x) {
        double sum = 0;
        for (int i = 0; i < iterations; i++) {
            if (x > 0) {
                int a = s_field1;
                s_field1 = a + i;
                int b = s_field1 + 2;
                s_field1 = b - 2 * i;
            } else {
                int a = s_field1/2;
                s_field1 = a - i;
                if (x > -5) {
                    double b = i_field1;
                    i_field1 = b + 2 * i;
                    if (x == -1) {
                        for (int j = 0; j < Math.sqrt(s_field1*x*x); j += s_field1)
                        s_field1++;
                    }
                }
            }
            sum = s_field1 + i_field1;
        }

        return sum;
    }

    /* Peeling candidate: iget, two inner loops, integer overflow */
    public int test_04(int x) {
        int sum = 0;
        for (int i = 0; i < iterations; i++) {
                for (int j = 0; j < x; j++) {
                    sum = i_field2;
                    i_field2 *= 37;
                }
                for (int j = 0; j < x; j++) {
                    sum = i_field2;
                    i_field2 *= -37;
                }


        }

        return sum + i_field2;
    }


    /* Peeling candidate: iget, LoadClass; do-while nested loops, break to label in outer loop */
    public double test_05(int x) {
        int sum = 0;
        Cls1 c = new Cls1();
        
        int i = x;
        int k = iterations;
        int l = -1;
mylabel:
        do {
            k = iterations;
            do {
                l = -1;
                if (k < 10) {
                    if (l > iterations - 2*x) {
                        break mylabel;
                    }

                    do {
                        c.i_field1 = c.i_field2 ;
                        l += x;
                        k++;

                    } while (l < iterations);
                    if (l > iterations - x) {
                        break mylabel;
                    }
                }
                k -= x;
            } while (k > -1);
            i *= 6;
        } while ( i < iterations + x);

        return sum + c.i_field1 + c.i_field2 + i + k + l;
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


