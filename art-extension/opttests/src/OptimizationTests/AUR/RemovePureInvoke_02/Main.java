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

package OptimizationTests.AUR.RemovePureInvoke_02;


import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;
import java.io.*;
import OptimizationTests.AUR.shared.*;

public class Main {

    public int field = 5;
    
    private int getSum(int arg) {
        return arg + 20;
    }


    // AUR example for InvokeStaticOrDirect: pure invoke should be removed if it's no longer used by any other instruction or environment
    // several invokes to be removed in a row
    public double runTest (int x, int n, Main m, Main other, double a, double b, double c, double d, double e, double f, double g, double h, double k, double l, double p, double q, double r, double s) {


        // if it is deleted, Dead Code Elimination will remove invoke
        n = n + x;

      int result = 0;
      for (int i = 0; i < 15000; i++) {
          result += i/2;
      }
      //e = Math.ulp(e); // pure, not intrinsic
      e = Math.asin(e); // pure, intrinsic
      b = Math.sin(b); //  pure, intrinsic
      a = Math.cos(a); // pure, intrinsic
      g = Math.tan(g); // pure, intrinsic
      k = Math.atan(k); // pure, intrinsic
      p = Math.acos(p); // pure, intrinsic
      r = Math.atan(r); // pure, intrinsic
        if (n > 6) {
            c += a;
            d += b;
            f += e;
            h += g;
            l += k;
            q += p;
            s += r;
        }
        return m.field + result;
    }

    // wrapper method for runTest
    public int test(int n) {
        Main m = new Main();
        Main other = new Main();
        return (int)runTest(11, n, m, other, 45d, 7d, -10d, 0d, -1.23456789d, 1.0d, 10000d, -10000d, 1d, -1d, Double.NaN, Double.POSITIVE_INFINITY, Double.POSITIVE_INFINITY, Double.POSITIVE_INFINITY);
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


