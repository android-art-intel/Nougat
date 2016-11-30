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

package OptimizationTests.LoopPeeling.Array_01;

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

    public static String foo_inlined_string(int x) {
        String str = "string";
        return str.concat(Integer.toString(x));
    }

    /* Peeling candidate: aget, all inputs from outside the loop, all conditions met, simple for loop */
    public int test_01(int x) {
        int[] arr = new int[] {0,1,2,3,4,5,6,7,8,9};
        int sum = 0;
        for (int i = 0; i < 10; i++) {
            sum += arr[4] ;

        }

        return sum;
    }

 /* Peeling candidate: 2 aput-wide with inputs from outside the loop, 1 aput-wide with input from inside loop, all conditions met, simple while loop */
    public long test_01a(int x) {
        long sum = 0L;
        long[] arr = new long[iterations];
        int  i = 0;
        while (i < iterations) {
            arr[0] = i;
            arr[1] = i + 1;
            arr[i] = 6L;
            i++;

        }

        return sum;
    }


 /* Peeling candidate: 1 aput,3 aget, all conditions met, simple do-while loop */
    public long test_01b(int x) {
        int sum = 0;
        int[] arr = new int[iterations];
        arr[0] = 1;
        arr[1] = 2;
        arr[2] = 3;
        arr[3] = 2;
        int i = 1;
        do {
            arr[arr[arr[2]]] = 10;
            i++;
        } while (i < iterations);

        return arr[0] + arr[1] + arr[2] + arr[3];

    }




    /* Peeling candidates: 1 aput, 1 aget, float type array,  increment !=1, incrementing using multiplication */
    public float test_02(int x) {
        float sum = 0;
        float[][] arr = new float[100][100];
        for (int i = 1; i < 100; i *= 6) {
            arr[i][5] = 11;
            sum += arr[i][i] + arr[i][5];

        }
        return arr[1][5];
    }


    /* Peeling candidates: aget, aput with Object type, while loop with break */
    public int test_03(int x) {
        
        Cls1[] arr1 = new  Cls1[iterations];
        Cls1[] arr2 = new  Cls1[iterations];
        Cls2_ext[] arr3 = new Cls2_ext[iterations];
        // was: Constructor is inlined, loop with array initialization is peeled
        // now: after 498012 patch merge unnecessary NullCheck is eliminated and no peeling occurs
        Cls2_ext c = new Cls2_ext(x);      

        //Not peeled - input from inside loop
        for (int i = 0; i < iterations ; i++ ) {
            arr3[i] = c;
        }
        
        int sum = 0;
        int  i = 0;

        //Peeled:
        while(true) {
            
            arr1[0] = arr2[10];
            arr1[i] = arr2[11];
            arr2[i] = arr1[i/2];
            arr2[i/2] = arr1[i/3];
            arr3[i] = c ;
            sum += arr1[i].s_field1 + arr2[i].s_field1 + arr3[i].count;
            i++;
            if (i>=iterations) {
                break;
            }
        }
        
        return sum;
    }


    /* 4 loops in a row */
    public double test_05(int x) {
        double sum = 0;
        double[] arr = new double[iterations];

        //not peeled
        for (int i = iterations-1; i >= 0; i--) {
            arr[i] = 0.12345d;
            double tmp = 0.5d;
            arr[i] += tmp;
        }

        for (int i = 1; i < iterations; i += 21) {
            double tmp = 0.5d;
            arr[i] += tmp;
            arr[iterations/2] = tmp;

        }

        for (int i = iterations; i > 1; i /= 2) {
            arr[5] = x;
        }

        arr[5] = -1;

        for (int i = iterations-1; i > 1; i -= iterations/3  ) {
            sum = arr[iterations/2];
            sum += arr[i];
            }


        return sum;
    }

    /*  float-point iterator, nested loop, complex loop condition */
    public int test_06(int x) {
        int sum = 0;
        int[] arr = new int[iterations];
        int j = 0;
        while (true) {
            j++;
            arr[0] = 100;
            arr[1] = 101;
            for (float i = 0.0f; i < 100 ; i += 1.33f) {
                arr[j] = x;
                arr[j/2] = x + 1;
            }
            arr[2] += 1;
            sum += arr[0] + arr[1] + arr[2];
            if (j > 15) {
                break;
            }
        }

        return sum;
    }

    /*  float-point iterator, array of objects, 2 nested loops, inlined func call in loop header, sput, sget, class loading, aput, aget, sget/sput before and after loop, if-else, continue */
    public int test_07(int x) {
        int sum = 0;
        Cls1_ext[] obj_array = {new Cls1_ext(), new Cls1_ext()};
        obj_array[0].s_field1 = 15;
        obj_array[1].s_field1 = 13;
        Cls1_ext obj1 = new Cls1_ext();
        Cls1_ext obj2 = new Cls1_ext();
        obj1.s_field1 = -1000;
        obj2.s_field1 = -30000;
        while (true) {
            if (x > 5) {
            for (float i = iterations; i > foo_inlined(iterations); i -= 1.33f) {
                obj_array[1] = obj1;
                if (i > 10) {
                    obj_array[0] = obj2;
                }
            }

            sum = sum + obj_array[0].s_field1 +  obj_array[1].s_field1;
            } else {
            for (float i = iterations; i > foo_inlined(iterations); i -= 1.33f) {
                if (i > 10) {
                    obj_array[0] = obj1;
                    continue;
                } 
            }
            }
            if ( sum > 100) {
                break;
            }
        }
        return sum + obj_array[0].s_field1 + obj_array[1].s_field1;
    }

    /* Parent environment update in loop header; after inliner pass there are 2 loops, each loop is peeled! A lot of loop candidates */
    /* now: after 498012 patch merge unnecessary NullCheck is eliminated and no peeling occurs in inlined loop, so only 1 peeling expected */
    public int test_08(int n) {
        int testVar = 0;
        // was: Constructor is inlined, loop with array initialization is peeled
        // now: after 498012 patch merge unnecessary NullCheck is eliminated and no peeling occurs
        Cls2 c = new Cls2(n);
        boolean[] b = new boolean[n];
        int sum = 0;
        int m = n - 1;
        for (int i = 0; i < c.getArrInlined(m).length + foo_inlined_string(n).length() - 8; i++) {
            i++;
            b[0] = false;
            b[c.arr.length/2] = true;
        }
        m++;
        n++;
        testVar++;
        if (b[0] || c.arr[0] ||  b[c.arr.length/2]) {
            return sum + n;
        } else {
            return sum + n + 1;
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

