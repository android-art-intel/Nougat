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

package OptimizationTests.LoopPeeling.Exceptions_01;

import OptimizationTests.LoopPeeling.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    public final int iterations = 10000;
    public static int s_field1 = 33;
    private double i_field1 = 0.00001234500012345d;
    protected double i_field2 = 1.00001234500012345d;
    public int i_field4 = 5;
    public double i_field3 = 2.00001234500012345d;
    public final static int s_final_field1 = 100;
    private volatile int i_volatile_field1 = 0;
    Cls1_ext c_e = new Cls1_ext();


    public static int foo_inlined(int x) {
        return x;
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



    /* Peeling candidate: only iget, simple for loop, try-catch in call - not inlined. If try-catch method is inlined in future - postproc script should be modified to be negative for this method */
    public double test_00(int x) {
        int a = x;
        int b = x + 100;
        for (int i = 0; i < iterations; i++) {
            a = i_field4;
            b = tryDiv(b + 10, (i - 10)); 

        }

        a += 15;

        return a + b + i_field4;
    }

    /* Peeling candidate: only iget, simple for loop, division by zero exception is thrown */
    public double test_01(int x) {
        int a = x;
        int b = x + 100;
        for (int i = 0; i < iterations; i++) {
            a = i_field4;
            b = (b + 10)/(i - 10); 

        }

        a += 15;

        return a + b + i_field4;
    }

    /* Peeling candidate: only iget, simple for loop, division by zero exception is thrown during the first iteration  - which should be peeled */
    public double test_01a(int x) {
        int a = x;
        int b = x + 100;
        for (int i = 0; i < iterations; i++) {
            a = i_field4;
            b = (b + 10)/(i + x - 10); 

        }

        a += 15;

        return a + b + i_field4;
    }


 /* Peeling candidate: iget, nested infinite loop with ArrayIndexOutOfBoundsException exception */
    public double test_02(int x) {
        int a = x;
        int b = x + 100;
        for (int i = 0; i < iterations; i++) {
            while (true) {
                a = c_e.arr_field1[x + i++];
                if (i > x + 100) break;
            }
        }

        a += 15;

        return a + b + i_field4;
    }

    /* 4 loops in a row, iget, iput; Math.sqrt, non-constant increment + 2 division by zero exceptions */
    public double test_03(int x) {
        double sum = 0.0;
        Cls1_ext c1 = new Cls1_ext();
        c1.i_field2 = x;
        Cls1_ext c2 = new Cls1_ext();
        c2.i_field2 = x + 1;
        float smallvalue = Float.MIN_VALUE;
        for (int i = 1; i < x*100; i *= 2) {
            sum += c1.i_field2 + x + Cls1_ext.s_field2 + Math.sqrt(i)/(i -x) ;
            float temp = smallvalue;
            smallvalue *= Cls1.s_field1;
            sum += c2.i_field1 + smallvalue;
            smallvalue = temp;
        }

        for (int i = 1; i < iterations; i += 2) {
            c2.i_field1 = x;
            c1.i_field1 = c2.i_field1 + 2;
            c2.i_field1 = c1.i_field2;
            c1.i_field1 = 10f;
            i += x + i%2;
        }

            if (x > 0) {    
                c2.i_field1 = -x; 
                c1.i_field1 = -10f;
            } else {
                c1.i_field1 = c2.i_field1 + 2;
                c2.i_field1 = -1*c1.i_field2;
            }

        for (int i = iterations; i > 1; i /= 2) {
            sum = c1.i_field1 + x + c2.i_field1 + Math.sqrt(i);
            float temp = smallvalue*3;
            smallvalue *= Cls1.s_field2 ;
            sum = c1.s_field2 + smallvalue;
            smallvalue = temp;
        }

        if (c1.i_field1 > 0) {    
            for (int i = iterations; i > 1; i -= iterations/3  ) {

                c2.i_field1 = -x / (i - x); 
                c1.i_field1 = -10f;
                }
        } else {
            for (int i = iterations; i > 1; i -= iterations/3  ) {
                c1.i_field1 = c2.i_field1 + 2;
                c2.i_field1 = -1*c1.i_field2;
            }
        }

        return sum + c1.i_field1 + c2.i_field1;
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
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: " + e);
                    //e.printStackTrace();
                } catch (IllegalAccessException e) {
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: " + e);
                    //e.printStackTrace();
                } catch (InvocationTargetException e) {
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: " + e + ", caused by: " + e.getCause());
                    //e.printStackTrace();
                }
            }
        }



    }

    public static void main(String[] args)
    {
        new Main().runTests();
    }

}


