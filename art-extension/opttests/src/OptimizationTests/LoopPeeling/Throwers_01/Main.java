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

package OptimizationTests.LoopPeeling.Throwers_01;

import OptimizationTests.LoopPeeling.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    public final int iterations = 10000;
    public static int iters = 10000;
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

    /* Peeling candidate: NullCheck */
    public int anotherTest_01(Cls1_ext x) {
        boolean sum = true;
        for (int i = 0; i < iterations; i++) {
            sum = (x.equals(null));
        }

        return sum?1:0;
    }

    /* Peeling candidate: BoundCheck */
    public int test_02(int x) {
        int sum = 0;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};

        while (sum < iterations) {
            tab[1] = 6;
            sum += x;    
            }
        return tab[1] + sum;
    }

    /* Peeling candidate: DivZero */
    public int test_03(int x) {
        int sum = 10000;
        int[] tab = new int[] {1,2,3,4,5,6,7,8,9};
        int i = 100;
        do {
            sum /= x;
            i =  i / 2;
        } while( i > 2);
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
        System.out.println("Test Throwers_01" + "; Subtest anotherTest_01" + "; Result: "+ new Main().anotherTest_01(c_e));


    }

    public static void main(String[] args)
    {
        new Main().runTests();
    }

}
