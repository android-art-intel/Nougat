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

package OptimizationTests.AUR.ParentEnvironment_01;


import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;
import java.io.*;
import OptimizationTests.AUR.shared.*;

public class Main {

    public int field = 5;
    private int[] fieldArray;




    C c_field = new C(2000);

    class C {
        boolean[] arr;
        int count = 0;
        public C(int n) {
            int arr_length = 0;
            if (n > 0) {
                arr_length = n;
            } else {
                arr_length = 0;
            }
            arr = new boolean[arr_length];
            for (int i = 0; i < arr.length; i++) {
                arr[i] = false;
            }
        }
        int bar (int x) {
            return arr[x] ? x : 0;
        }

        /* inlined */
        boolean[] getArr1() {
            return arr;
        }

        /* not inlined */
        boolean[] getArrNotInlined(int x) {
            arr[x] = false;
            return arr;
        }


        /* inlined */
        boolean[] getArrInlined(int x) {
            count = count/x;
            return arr;
        }




    }





    private int getSum(int arg) {
        return arg + 20;
    }



    // post-processing is very sensitive to inlining changes
    // there is a parent environment in this test
    public int runTest (int n, Main unusedVar, Main other, int[] arr, int x) {

        int testVar = 0;

        C c1 = null;


        for (int i = 0; i < 2000; i++) {
            i++;
            //C constructor will be inlined, NewArray and SuspendCheck from inlined method will have parent environment
            c1 = new C(n);
        }

        //Add and ArrayGet will be removed
        x = arr[x] + (new int[10])[2/n] + unusedVar.getSum(n);

        return c1.count + testVar + other.field;

    }

    // wrapper method for runTest
    public int test(int n) {
        Main unusedVar = new Main();
        Main other = new Main();
        if (n == 11) { 
            unusedVar = null; 
        }
        return runTest(n, unusedVar, other, new int[n*2], n);
    }

    // wrapper method for runTest with GC stress threads running
    public String testWithGCStress(int n) {
        String res = "";
        Thread t = new Thread(new Runnable(){
                public void run() {
                new StressGC().stressGC();
                }
                });

        t.start();
        try {
            res += test(n);
        } catch (Throwable e) {
            res += e;
        }
        try {
            t.join();
        } catch (InterruptedException ie) {
        }
        return res;
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
                    StringWriter sw = new StringWriter();
                    StackTraceElement[] ste = e.getCause().getStackTrace();
                    // To workaround difference between android detailed message and java brief one:
                    System.out.println(e.getCause().getClass());
                    for (int i = 0; i < ((ste.length < 2)?ste.length:2); i++) {
                        System.out.println(ste[i]);
                    }
                }
            }
        }



    }

    public static void main(String[] args)
    {
        new Main().runTests();
    }

}


