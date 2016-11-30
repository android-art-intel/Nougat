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

package OptimizationTests.LoopPeeling.Branching_01;

import OptimizationTests.LoopPeeling.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    public final int iterations = 10000;
    public static int s_field1 = 33;
    private double i_field1 = 0.00001234500012345d;
    public final static int s_final_field1 = 100;
    private volatile int i_volatile_field1 = 0;
    boolean i_b_field1 = false;
    boolean[] arr_b_field1 = new boolean[] {true, false, true, false};


    public static int foo_inlined(int x) {
        return x;
    }

    /* Peeling candidate: ClassLoad (goes with sput), if-else inside loop, no loop phi merges inside loop */
    public double test_01(int x) {
        double sum = 0;
        for (int i = 0; i < iterations; i++) {
                sum = s_field1;
                i_field1 = 1.25d;
            if ( x > 10) {
                i_field1 = 1.24d;
            } else {
                s_field1 = 5;
                i_field1 = 1.21d;
//                sum = i_field1;
            }
            i_field1 = 1.23d;
        }

        return sum + s_field1 + i_field1;
    }

    /* Peeling candidate: aput + BoundsCheck, iget, if-else inside loop, no loop phi merges inside loop */
    public double test_02(int x) {
        int sum = 0;
        for (int i = 0; i < iterations; i++) {
            if ( x > 10) {
                arr_b_field1[0] = i_b_field1;
            }
           else {
                boolean tmp = i_b_field1;
                arr_b_field1[1] = tmp;
            }
            
        }

        return sum + s_field1 + i_field1 + (arr_b_field1[0]?1:0) + (arr_b_field1[1]?1:0);
    }

    /* Peeling candidate: sget, iget, iput, sput, LoadClass, reading/writing to the same field twice, nested if-else inside loop, no loop phi merges inside loop*/
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
                        i_field1++;
                    }
                }
            }
            sum = s_field1 + i_field1;
        }

        return sum;
    }

    /* Peeling candidate: get, iput, reading/writing to the same field twice, all candidates inside if-else */
    public double test_04(int x) {
        double sum = 0;
        for (int i = 0; i < iterations; i++) {
            if (x > 0) {
                double a = i_field1;
                i_field1 = a + i;
                double b = i_field1;
                i_field1 = b + 2 * i;
            } else {
                System.out.println("test_04");
            }
        }

        return sum + i_field1;
    }


    /* Peeling candidate: iget, LoadClass, do while, switch */
    public double test_05(int x) {
        int sum = 0;
        int i = x;
        Cls1 c = new Cls1();
        do {
           switch (i) {
               case 0:  c.i_field1 = c.i_field2 ;
                        break;
               case 10: c.i_field1 = c.i_field2 + i;
                        break;
               case 20: c.i_field1 = c.arr_field2[i/10];
                        break;
               default: c.i_field1 = c.arr_field2[i/iterations];
                        break;
           }

                i += x;

        } while ( i < iterations);

        return sum;
    }

    /* Peeling candidates: 2 sget (1 sget for final field); iget, continue, inlined function call, loop starts at -1, increment > 1 */
    public double test_06(int x) {
        double sum = 0.0;
        for (int i = -1; i < iterations; i += 5) {
            sum += s_final_field1 + x + Cls1.s_field1 + foo_inlined(10);
            sum += s_field1 + (i*i) ;
            if (i > 100) continue;
            i_field1++;



        }

        return sum + i_field1;
    }

    /* Peeling candidates: outer loop - count down loop, two inner loops, if-else outside the inner loops, decrement != 1 */
    public double test_07(int x) {
        double sum = 0.0;
        Cls1 c = new Cls1();
        for (int i = iterations; i >= -2; i -= 5) {
            double tmp = -0.00001d;
            // sput but input not from outside -- not a candidate for peeling
            Cls1.s_field2 = tmp;
            sum += s_final_field1 + x + Cls1.s_field2 + Math.sqrt(10);
            tmp = sum ;
            if (i > 10) {
                for (int j = 0; j < x; j+=3) {
                    i_field1 = c.arr_field2[2] + i + j;
                }
            } else {
                c.i_field1++;
                for (int j = x; j > 0; j-=2) {
                    i_field1 = c.arr_field2[3] - i - j;
                }
            }
            sum = tmp / 2 + c.i_field1 + i_field1;
        }

        return sum;
    }


    /*  float-point iterator, inlined func call in loop header and body, sput, sget, class loading, aput, aget, sget/sput before and after loop, if-else  - a lot of peeling candidates */
    public double test_08(int x) {
        double sum = 0;
        Cls1_ext[] obj_array = {new Cls1_ext(), new Cls1_ext()};
            obj_array[1].arr_field1[0] = 0;
        for (float i = iterations; i > foo_inlined(iterations); i -= 1.33f) {
            obj_array[1].arr_field1[0] = -1;
            double temp = obj_array[0].s_field1 + obj_array[1].s_field1 + foo_inlined(obj_array[1].arr_field1[0]);
            sum += ((1.0+2.0*(-1.0)*(i%2))*temp * x)/(double)(2*i+1);
            if (i > 10) {
                obj_array[1].arr_field1[0] += 1;
            }
        }
        obj_array[1].s_field1++;
        return sum*4 + obj_array[1].s_field1++;
    }

    /* Parent environment update in loop header; after inliner pass there are 3 loops, each loop is peeled! */
    /* after 498012 patch merge unnecessary NullCheck is eliminated and no peeling occurs for inlined loops from Cls2 constructor, so no peeling expected at all */
    public int test_09(int n) {
        int testVar = 0;

        Cls2 c = new Cls2(n);
        Cls2 c1 = new Cls2(n*2);

        boolean[] b = new boolean[n];

        int m = n - 1;
        for (int i = 0; i < c.getArrInlined(m).length; i++) {
            i++;
            b[i] = true;
            if (b[i/2]) {
                c1 = new Cls2(c1.arr.length);
            }
        }

        m++;
        n++;
        testVar++;

        if (b[0] || c1.arr[0]) {
            return n;
        } else {
            return n + 1;
        }

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


