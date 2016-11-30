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

package OptimizationTests.FormBottomLoops.Throwers;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 *
 * Few cases dealing with possible throwers
 *
 **/

public class Main {                                                                                                                                                   
    final int iterations = 1100;

    int count = 0;
    static int static_field = 6;
    static MyClass myclass_field;
    static MyClass myclass_field_not_init;

    class MyClass {
        int value = 0;
        int value1 = 1;
        int value2 = 2;
        int value3 = 3;
        int[] arr;
        public MyClass(int x) {
            value = x;
            value1 = x/2;
            value2 = x*3;
            value3 = x%4;
            arr = new int[x];
            for (int i = 0; i < x; i++) {
                arr[i] = i*x/2;
            }

        }

        int[] getArr() {
            return arr;
        }

        int[] modifyAndGetArr() {
            arr[0] = 100;
            return arr;
        }
    }

    public int foo(int x) {
        for (int i = 0; i < iterations; i++) {
            x = x + i/2;
        }
         return x;
    }

    public int sideEffectFunc(int x) {
        count++;
        return x + count;

    }

    public int tryDiv(int x, int y) {
        int res = 0;
        try {
            res = x/y;
        } catch (Exception e) {
            System.out.println("Exception when dividing " + x + " by " + y);
            res = x/(y+1);
        }
        return res;

    }

    /* Positive: ExceptionInInitializerError */
    public int another_test1(int n, MyClass c) {
        int testVar = 0;
        int i = 0;
        

        while (true) {
            Example1 ex = new Example1();
            if (i++ > iterations) {
                break;
            } 
            testVar += ex.value;
            c.getArr()[i%100] = c.modifyAndGetArr()[n];
        }
        return c.getArr()[n] + testVar;
    }

    /* Wrapper for another_test1 */
    public String test1(int n) {
        myclass_field = new MyClass(100);
        int res = -1;
        try {
            res += another_test1(n, myclass_field);
        } catch (Throwable e) {
            return "" + res + ", " + e.getCause().toString();
        }

        return "" + res;

    }
 
    /* Positive: ExceptionInInitializerError : Clinit, NullCheck (but they are actually optimized away by load_store_elimination, GVN) */
    public int another_test2(int n, MyClass c, String str) {
        int testVar = 0;
        int i = 0;
        

        while (true) {
            Example2 ex = new Example2(str);
            if (i++ > iterations) {
                break;
            } 
            testVar += ex.getSubstring().length();
            c.getArr()[i%100] = c.modifyAndGetArr()[n];
        }
        return c.getArr()[n] + testVar;
    }


    /*  Wrapper for another_test2 */
    public String test2 (int n) {
        int res = -1;
        try {
            res += another_test2(n, null, null);
        } catch (Throwable e) {
            return "" + res + ", " + e.getCause().getClass().getName();
        }

        return "" + res;


    }

    /* Positive: ExceptionInInitializerError: same as in previous test but in another part of the loop */
    public int another_test3(int n, MyClass c, String str) {
        int testVar = 0;
        int i = 0;
        

        while (true) {
            if (i++ > iterations) {
                break;
            } 
            Example3 ex = new Example3(str);
            testVar += ex.getSubstring().length();
        }
        return testVar;
    }


    /*  Wrapper for another_test3 */
    public String test3(int n) {
        int res = -1;
        try {
            res += another_test3(n, null, "Test");
        } catch (Throwable e) {
            return "" + res + ", " + e.getCause().getClass().getName();
        }

        return "" + res;
    }





    /* Positive: BoundsCheck */
    public int test4(int n) {
        int testVar = 0;
        int additionalVar = 1;
        int i = 0;
        MyClass c = new MyClass(iterations);
        while (true) {
            if (i++ > iterations - 2)  break;
            c.arr[i] = i;
            testVar += c.arr[i/2];
        }

        return testVar + additionalVar;
    }


    /* Positive: DivZeroCheck */
    public int another_test5(int n) {
        int testVar = 0;
        int i = 0;
        MyClass c = new MyClass(iterations);
        while (true) {
            count += n / (i - n);
            if (i++ > iterations - 2)  break;
            testVar += n / (i - n + 2);
            count  = testVar;
            testVar = 0;
        }

        return testVar + count;
    }

    /*  Wrapper for another_test5 */
    public String test5(int n) {
        int res = -1;
        try {
            res += another_test5(n);
        } catch (Throwable e) {
            return "" + count + ", " + e.getClass().getName();
        }

        return "" + res;
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
                String names[] = c.getPackage().getName().split("\\.");
                String testName = names[names.length-1];
                try {
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: "+ m.invoke(this, 10));
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: " + e + ", caused by: " + e.getCause());
//                    e.printStackTrace();
                }
            }
        }



    }

    public static void main(String[] args)
    {
        new Main().runTests();
    }


}  

