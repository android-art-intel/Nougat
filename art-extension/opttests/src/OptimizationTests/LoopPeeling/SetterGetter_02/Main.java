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

package OptimizationTests.LoopPeeling.SetterGetter_02;

import OptimizationTests.LoopPeeling.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    public final int iterations = 10000;
    public static int s_field1 = 33;
    private double i_field1 = 0.00001234500012345d;
    protected double i_field2 = 1.00001234500012345d;
    public double i_field3 = 2.00001234500012345d;
    public final static int s_final_field1 = 100;
    private volatile int i_volatile_field1 = 0;
    Cls1_ext c_e = new Cls1_ext();


    public static int foo_inlined(int x) {
        return x;
    }

    /* Peeling candidate: only iget, all conditions met, simple for loop */
    public double test_00(int x) {
        double sum = 0;
        sum += x;
        for (int i = 0; i < iterations; i++) {
            sum = i_field1;
        }
        
        sum += 0.5d;

        return sum;
    }



    /* Peeling candidate: iget, all conditions met, simple for loop */
    public double test_01(int x) {
        float sum = 0;
        Cls1 c = new Cls1();
        if (x> 0) {
            c.i_field1 += 0.5f;
        } else {
            c.i_field1 -= 0.5f;
        }

        for (int i = 0; i < iterations; i++) {
            sum += c.i_field1 + x + i;
        }
        c.i_field1 +=0.1f;

        return sum;
    }

    /* Peeling candidates: 2 iget, 2 iput; inlined function call, loop starts at -1, increment > 1, using field as iterator */
    public double test_02(int x) {
        double sum = 0.0;
        Cls1_ext c = new Cls1_ext();
        c.set_i_field1(0);
        for (; c.i_field1 < iterations; c.set_i_field1(c.i_field1 + 2)) {
            sum += c.i_field1;

        }

        return sum;
    }

    /* Peeling candidates: iget, protected field ; count down loop, decrement != 1 */
    public double test_03(int x) {
        float sum = 0.0f;
        Cls1_ext c = new Cls1_ext();

        for (int i = iterations; i >= -2; i -= 5) {
            float tmp = -0.00001f;
            // iput but input not from outside -- not a candidate for peeling
            i_field2 = tmp;
            sum += tmp;// + Math.sqrt(10);
            if (x > 0) {
                tmp = x;
            } else {
                tmp = 0;
            }
            sum += i_field2;
        }

        return sum + i_field1 + i_field3 + i_field2;
    }

    /*Peeling candidate: LoadClass*/
    public double test_04(int x) {
        double sum = 0.0;
        Cls1_ext c = new Cls1_ext();
        for (int i = 0; i < iterations ; i++) {

           c_e.i_field1 = 5;
  //         i_field1 = 4;

        }

        return sum + c.i_field1 + i_field1 + c_e.i_field1;
    }


    /* 4 loops in a row, iget, iput; Math.sqrt, non-constant increment */
    public double test_05(int x) {
        double sum = 0.0;
        Cls1_ext c1 = new Cls1_ext();
        c1.i_field2 = x;
        Cls1_ext c2 = new Cls1_ext();
        c2.i_field2 = x + 1;
        float smallvalue = Float.MIN_VALUE;
        for (int i = 1; i < x*100; i *= 2) {
            sum += c1.i_field2 + x + Cls1_ext.s_field2 + Math.sqrt(i);
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
            smallvalue *= Cls1.s_field2;
            sum = c1.s_field2 + smallvalue;
            smallvalue = temp;
        }

        if (c1.i_field1 > 0) {    
            for (int i = iterations; i > 1; i -= iterations/3  ) {

                c2.i_field1 = -x; 
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


