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

package OptimizationTests.LoopPeeling.Negative_01;

import OptimizationTests.LoopPeeling.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    public final int iterations = 10000;
    public static int s_field1 = 33;
    private double i_field1 = 0.00001234500012345d;
    public final static int s_final_field1 = 100;
    private volatile int i_volatile_field1 = 0;


    public static int foo_inlined(int x) {
        return x;
    }



    /* Negative: Peeling candidate: sget, simple for loop, > 100 instructions */
    public double test_01(int x) {
        int sum = 0;
        int sum1 = 1;
        int sum2 = 2;
        int sum3 = 3;
        int sum4 = 4;
        int sum5= 5;
        for (int i = 0; i < iterations; i++) {
            sum += Cls1.s_field1 + x + i;
            sum1 += i/2 + i/3 - i*4 + i*5 + i%6 - i/7 + i*8 + i/9 - i/10 + sum/2 + i/11;
            sum2 += i/2 + i/3 - i*4 + sum1*5 + i%6 - sum3/7 + i*8 + i/9 - i/10 - sum/3 + i/12;
            sum3 += i/2 + i/3 - i*4 + sum1*5 + i%6 - sum3/7 + i*8 + i/9 - i/10 + sum/4 - i/13;
            sum4 += i/4 + i/3 - i*4 + sum4*5 + i%6 - sum2/7 + i*5 + i/9 - i/10 + sum1/4 - i/13;
            sum4 += i*14 + i/13 - i*34 + sum3*5 + i%6 - sum2/47 + i*5 + i/9 - i/410 + sum1/4 - i/13;


        }

        return sum + sum1 + sum2 + sum3 + sum4 + sum5;
    }

    /* Negative: Peeling candidate: sget, do-while loop, > 10 BB */
    public double test_02(int x) {
        int sum = 0;
        int sum1 = 1;
        int sum2 = 2;
        int sum3 = 3;
        int sum4 = 4;
        int sum5= 5;
        int i = 0;
        do {
            sum += Cls1.s_field1 + x + i;
            if  (sum > i ) {
                sum1 += i/2 + i/3 - i*4 + i*5 + sum/2;
            } else if (sum < 10) {
                sum2 += i/2 + i/3 - i*4 + sum1*5 - sum3/7 + i/12;
            } else if (sum == 11) {
                sum3 += i/2 + i/3 - i*4 + sum1*5 + i%6 - sum/4 - i/13;
                if (sum3 > 5*i) {
                    sum4 += i/4 + i/3 - i*4 +  sum1/4 - i/13;
                }
            }
            sum4 += i*14 + i/13 - i*34 + sum3*5 + sum2/47 + sum1/4;

            i++;
        } while (i < iterations);

        return sum + sum1 + sum2 + sum3 + sum4 + sum5;
    }

    /* Negative: Peeling failed because the loop phi doesn't have expected form */
    /* Loop phis must have only two inputs - one from outside the loop and one from inside */
    /* in this case phi for 'sum' has both inputs from ouside because constants constants get placed at entry into method */
    public String test_03(int x) {
        String sum = "" + x;
        String str = "3";
        int count = 0;
        for (int i = 0; i < iterations; i++) {
            sum = str;
            count += sum.length();
        }

        return sum + count;
    }

    /* Negative: Peeling failed because loop has control flow with phi merges */
    public String test_04(int x) {
        String sum = "";
        Cls1[] obj_array = {new Cls1(), new Cls1()};
        obj_array[0].str_field = "0";
        obj_array[1].str_field = "1";
        for (float i = iterations; i > foo_inlined(iterations); i -= 1.33f) {
            obj_array[1].str_field = "-1";
            String tmpString = "2";
            sum.concat(obj_array[0].str_field);
            if (i > 10) {
                sum.concat(tmpString);
                tmpString = "3";
            } else {
                obj_array[0].str_field = tmpString;
            }
            obj_array[0].str_field = tmpString;
        }
        obj_array[1].s_field1++;
        obj_array[0].str_field.concat("a");
        obj_array[1].str_field.concat("b");

        return sum + obj_array[0].str_field + obj_array[1].str_field;
    }

    /* Negative: Candidate has input from inside the loop: i */
    public int test_05(int x) {
        int[] arr = new int[] {0,1,2,3,4,5,6,7,8,9};
        int sum = 0;
        for (int i = 0; i < 10; i++) {
            sum += arr[i] ;
        }

        return sum;
    }


    /* Negative: loop has multiple exits */
    public int test_06(int x) {
        int sum = 0;
        int[] arr = new int[iterations];
        int j = 0;
        while (true) {
            j++;
            arr[0] = 100;
            arr[1] = 101;
            for (float i = 0.0f; (i < 100 && arr[2] > 10) ; i += 1.33f) {
                arr[0] = x;
                arr[1] = x + 1;
            }
            arr[2] += 1;
            sum += arr[0] + arr[1] + arr[2];
            if (j > 15) {
                break;
            }
        }

        return sum;
    }


    /* Negative: this iput doesn't trigger peeling */
    public double test_07(int x) {
        double sum = 0.0;
        Cls1_ext c = new Cls1_ext();
        for (int i = 0; i < iterations ; i++) {
            i_field1 = 4;

        }

        return sum + i_field1;
    }

    /* Peeling failed because the loop has multiple exits*/
    public double test_08(int x) {
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
                do {
                    c.i_field1 = c.i_field2 ;
                    if (l == k) {
                        break mylabel;
                    }

                    l += x;
                } while (l < iterations);
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


