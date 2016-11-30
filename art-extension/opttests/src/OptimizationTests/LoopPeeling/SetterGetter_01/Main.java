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

package OptimizationTests.LoopPeeling.SetterGetter_01;

import OptimizationTests.LoopPeeling.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;

public class Main {
    public final int iterations = 10000;
    public static int s_field1 = 33;
    private double i_field1 = 0.00001234500012345d;
    public final static int s_final_field1 = 100;
    private volatile int i_volatile_field1 = 0;


    public static int foo_inlined(int x) {
        return x;
    }

    /* Peeling candidate: sget (+LoadClass), all conditions met, simple for loop */
    public double test_00(int x) {
        int sum = 0;
        for (int i = 0; i < iterations; i++) {
            sum = s_field1;
        }

        return sum;
    }

    /* Peeling candidate: sget (+LoadClass), sput, reading/writing to the same field twice, all conditions met, simple for loop */
    public double test_00a(int x) {
        int sum = 0;
        for (int i = 0; i < iterations; i++) {
            int a = s_field1;
            s_field1 = a + i;
            int b = s_field1;
            s_field1 = b + 2 * i;

            sum = s_field1;
        }

        return sum;
    }

    /* Peeling candidate: iget, iput, reading/writing to the same field twice, all conditions met, simple for loop */
    public double test_00b(int x) {
        double sum = 0;
        for (int i = 0; i < iterations; i++) {
            double a = i_field1;
            i_field1 = a + i;
            double b = i_field1;
            i_field1 = b + 2 * i;

            sum = i_field1;
        }

        return sum;
    }


    /* Peeling candidate: sget, all conditions met, simple for loop */
    public double test_01(int x) {
        int sum = 0;
        for (int i = 0; i < iterations; i++) {
            sum += Cls1.s_field1 + x + i;
        }

        return sum;
    }

    /* Peeling candidates: 2 sget (1 sget for final field); inlined function call, loop starts at -1, increment > 1 */
    public double test_02(int x) {
        double sum = 0.0;
        for (int i = -1; i < iterations; i += 5) {
            sum += s_final_field1 + x + Cls1.s_field1 + foo_inlined(10);
            sum += s_field1 + (i*i) ;

        }

        return sum;
    }

    /* Peeling candidates: 1 sget, 1 sget final field; intrinsic Math.sqrt, count down loop, decrement != 1 */
    public double test_03(int x) {
        double sum = 0.0;
        for (int i = iterations; i >= -2; i -= 5) {
            double tmp = -0.00001d;
            // sput but input not from outside -- not a candidate for peeling
            Cls1.s_field2 = tmp;
            sum += s_final_field1 + x + Cls1.s_field2 + Math.sqrt(10);
            tmp = sum ;
            tmp += s_field1 + (i*i) ;
            sum = tmp / 2;
        }

        return sum;
    }


    /*Peeling candidate: sput + LoadClass*/
    public double test_04(int x) {
        double sum = 0.0;
        for (int i = 0; i < iterations ; i++) {
            s_field1 = 5;

        }

        return sum;
    }





    /* 4 loops in a row, 2 sget (in particular, inherited class sget), 1 sput; Math.sqrt, non-constant increment */
    public double test_05(int x) {
        double sum = 0.0;
        Cls1_ext.s_field2 = x;
        Cls1_ext obj = new Cls1_ext();
        double smallvalue = Double.MIN_VALUE;
        for (int i = 1; i < iterations; i *= 2) {
            sum += Cls1.s_field2 + x + Cls1_ext.s_field2 + Math.sqrt(i);
            double temp = smallvalue;
            smallvalue *= Cls1.s_field1;
            sum += obj.s_field2 + smallvalue;
            smallvalue = temp;
        }

        for (int i = 1; i < iterations; i += 2) {
            sum += Cls1.s_field1 + x + Cls1_ext.s_field2 + Math.sqrt(i/2);
            double temp = smallvalue;
            smallvalue += Cls1.s_field1;
            sum += obj.s_field2 + smallvalue;
            smallvalue = temp;
        }

        for (int i = iterations; i > 1; i /= 2) {
            sum += Cls1.s_field1 + x + Cls1_ext.s_field1 + Math.sqrt(i);
            double temp = smallvalue*3;
            smallvalue *= Cls1.s_field2;
            sum += obj.s_field2 + smallvalue;
            smallvalue = temp;
        }

        //+ sput, only 2 or 3 iterations
        for (int i = iterations; i > 1; i -= iterations/3  ) {
            Cls1_ext.s_field2 = x + i;
            sum += Cls1.s_field1 + x + Cls1_ext.s_field1 + Math.sqrt(i);
            double temp = smallvalue*3;
            smallvalue *= Cls1.s_field2;
            sum += obj.s_field2 + smallvalue;
            smallvalue = temp;
        }


        return sum;
    }

    /*  float-point iterator, sput, sget, class loading, aput, aget - a lot of peeling candidates */
    public double test_06(int x) {
        double sum = 0;
        Cls1_ext[] obj_array = {new Cls1_ext(), new Cls1_ext()};
        for (float i = 0.0f; i < iterations; i += 1.33f) {
            obj_array[0] = new Cls1_ext();
            obj_array[1].arr_field1[0] = -1;
            double temp = obj_array[0].s_field1 + obj_array[1].s_field1 + obj_array[1].arr_field1[0];
            sum += ((1.0+2.0*(-1.0)*(i%2))*temp * x)/(double)(2*i+1);
        }

        return sum*4;
    }

    /*  float-point iterator, inlined func call in loop header and body, sput, sget, class loading, aput, aget, sget/sput before and after loop, if-else  - a lot of peeling candidates */
    public double test_07(int x) {
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

    /* Parent environment update in loop header; 1 loop is peeled! */
    public int test_08(int n) {
        int testVar = 0;

        Cls2 c = new Cls2(n);
        Cls2 c1 = new Cls2(n*2);

        boolean[] b = new boolean[n];

        int m = n - 1;
        for (int i = 0; i < c.getArrInlined(m).length; i++) {
            i++;
            b[i] = true;
            c1.count = 5;
            testVar = c1.count;
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


