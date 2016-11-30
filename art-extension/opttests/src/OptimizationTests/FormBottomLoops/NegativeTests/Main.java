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

package OptimizationTests.FormBottomLoops.NegativeTests;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;

/**
 *
 * Few negative test cases not covered by other tests
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
            if (x == 0) {
                arr = new int[] {0};
            }
            for (int i = 0; i < x; i++) {
                arr[i] = i + i*x/2;
            }

        }

        int[] getArr() {
            return arr;
        }

        List<Integer> getList() {
            List arrList = new ArrayList();
            for (int i = 0; i < arr.length; i++) {
                arrList.add(new Integer(arr[i]));
            }
            return arrList;
        }

        int[] modifyAndGetArr() {
            arr[0] = 100;
            return arr;
        }

        boolean foo() {
            return (arr[arr.length/2] > 10);
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

    /* Negative: loop header doesn't exit the loop */
    public int test1(int n) {
        int testVar = 0;
        int i = 0;

        while (true) {
            static_field += i/4;
            if (i > 10) {
                static_field += i/4;
            }
            if (i++ > iterations) break;
            static_field -= i/2;
            static_field += i/3;

        }

        return static_field;
    }


    /* Negative: Rejecting due to cycled Phis */
    public int test2(int n) {
        int testVar = 0;
        int additionalTestVar = 100;
        int i = 0;


        while (true) {

            if (i++ > iterations) break;
            int swap  = testVar;
            testVar = n;
            n = swap;
            additionalTestVar += testVar + n + swap;


        }

        return n + testVar + additionalTestVar;
    }


    /* Negative: Loop header doesn't end with HIf */
    public void test3 (int n) throws NullPointerException {
        int testVar = 0;
        int additionalTestVar = 100;
        int i = 0;
        MyClass c = new MyClass(0);
        while (true) {
            count++;
            if (c.foo()) {
               System.out.println(count + testVar + additionalTestVar); 
               throw new NullPointerException();
            }
            i++;
            c = new MyClass(i);
            count += 6;
            additionalTestVar += testVar + n*i;
            testVar += n + i/2;
            count = testVar/2;
            

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

