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

package OptimizationTests.AUR.Misc_01;


import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;
import OptimizationTests.AUR.shared.*;

public class Main {

    public int field = 5;
    
    private int getSum(int arg) {
        return arg + 20;
    }


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


    // SuspendCheck with parent environment
    public int runTest(C c, int n) {
        C c1 = new C(n-1);
        return c.getArrInlined(c1.getArrInlined(n-2).length).length;
    }

    // wrapper method for runTest
    public int test(int x) {
        C c = new C(x);
        return runTest(c, x);
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


