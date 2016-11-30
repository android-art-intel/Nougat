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

package OptimizationTests.LoopPeeling.Volatile_01;

import OptimizationTests.LoopPeeling.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    public final int iterations = 10000;
    public volatile static int s_field1 = 33;
    private volatile double i_field1 = 0.00001234500012345d;
    public static volatile int s_final_field1 = 100;
    private volatile int i_volatile_field1 = 0;


    public static int foo_inlined(int x) {
        return x;
    }


    /* Peeling candidate: iget volatile - should be rejected */
    public double test_01(int x) {
        int sum = 0;
        for (int i = 0; i < iterations; i++) {
            sum += i_volatile_field1 + x + i;
        }

        return sum;
    }

    /* Peeling candidates: sget volatile, but should be peeled because of LoadClass*/
    public double test_02(int x) {
        double sum = 0.0;
        for (int i = -1; i < iterations; i += 5) {
            sum += s_field1 + (i*i) ;

        }

        return sum;
    }

    /* Peeling candidates: sput volatile, but should be peeled because of LoadClass */
    public double test_03(int x) {
        double sum = 0.0;
        for (int i = iterations; i >= -2; i -= 5) {
            s_field1 = -1;
        }

        return sum;
    }


    /*Peeling candidate: iput volatile - no peeling*/
    public double test_04(int x) {
        double sum = 0.0;
        for (int i = 0; i < iterations ; i++) {
            i_volatile_field1 = 5;

        }

        return sum;
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


