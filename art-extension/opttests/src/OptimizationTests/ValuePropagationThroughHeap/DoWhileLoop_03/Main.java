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

package OptimizationTests.ValuePropagationThroughHeap.DoWhileLoop_03;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 *
 * Simplest case when optimization should be applied: 'do while' loop
 * The iteration count is not known at compile time, VPTH optimization should be applied
 *
 **/

public class Main {                                                                                                                                                   
    static int iterations = 1100;

    double d = 0;
    double[] y = new double[iterations];
    double w = 1.0d;
    double v = 1.0d;

    private void a(double paramDouble1, double paramDouble2, double[] paramArrayOfDouble)
    {
        double d1 = 0.499975D * (paramDouble1 + paramDouble2);
        paramArrayOfDouble[0] = ((d1 + 0.499975D * (d1 + paramDouble2)) / 2.0D);
    }

//    public double b(int n) {
//        this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
//        for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d)) { a(this.v, this.w, this.y);} 
//        return n + this.y[0];
//    }

/* Positive: simple test, loop is not affected by other optimization */
/*
*/
    public int test1(int n) {

        this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
        this.d = 1;
        do {
//it is expected that method call to a(this.v, this.w, this.y); will be inlined
            a(this.v, this.w, this.y);
            this.d = (1 + this.d);
        } while (this.d <= this.iterations);
        return (int)this.y[0];
    }

/* Positive: iterator start value is unkown */
    public double test2(int n) {
        int k = n;
        double s = 0.0d;
        this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
        do {
//it is expected that method call to a(this.v, this.w, this.y); will be inlined
            a(this.v, this.w, this.y);
            k++;
        } while (k < iterations);
        return this.y[0] + n + k;
    }

    public int test3(int n) {

        this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D;
        double s = 0d;
        this.d = 1;
        do { 
//it is expected that method call to a(this.v, this.w, this.y); will be inlined
            a(this.v, this.w, this.y);
            this.d = (1 + this.d);
        } while (this.d <= this.iterations);
        return (int)this.y[0] + (int)this.v + (int)this.w;
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
        if (args.length > 0) {
           iterations = Integer.parseInt(args[0]);
        }
        new Main().runTests();
    }
}  

