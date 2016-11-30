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

package OptimizationTests.ValuePropagationThroughHeap.TryCatchForLoop_01;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    final int iterations = 1100;

    MyClass c_field = new MyClass(100);

    int count = 0;
    double d = 0;
    double[] y = new double[iterations];
    double w = 1.0d;
    double v = 1.0d;

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

    final double test1(int n) {
        double res = 0;
        this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;

        for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d)) 
        {
            this.y[0]= ((0.499975D * (0.434975D + this.w)) * 2.0D);
        }

        res += this.y[0];

        try {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d)) 
            {
                this.y[0] = ((0.499575D * (0.434455D + this.w)) * 2.0D);
            }

            if (this.y[0] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d)) 
            {
                this.y[0]= ((0.499575D * (0.434455D + this.v)) * 2.0D);
            }

            res /= this.y[0];
        } finally {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d)) 
            {
                this.y[0]= ((0.499575D * (0.434455D + this.v)) * 2.0D);
            }

            res *= this.y[0];
        }

        this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
        for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d)) 
        {
            this.y[0]= ((0.499575D * (0.434455D + this.v)) * 2.0D);
        }

        res -= this.y[0];

        return res;
    }

    private final void a(double paramDouble1, double paramDouble2, double[] paramArrayOfDouble)
    {
        double d1 = 0.499975D * (paramDouble1 + paramDouble2);
        paramArrayOfDouble[0] += ((d1 + 0.499975D * (d1 + paramDouble2)) + 2.0D);
    }

    final double test2(int n) {
        double res = 0;
        this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;

        for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d))
        {
            a(this.v, this.w, this.y);
        }

        res += this.y[0];

        try {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d))
            {
                a(this.v, this.w, this.y);
            }

            if (this.y[0] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d))
            {
                a(this.v, this.w, this.y);
            }

            res /= this.y[0];
        } finally {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d))
            {
                a(this.v, this.w, this.y);
            }

            res *= this.y[0];
        }

        this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
        for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d))
        {
            a(this.v, this.w, this.y);
        }

        res -= this.y[0];

        return res;
    }

    final double test3(int n) {
        double res = 0;
        int k = n;
        this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;

        for (c_field.value = 1; c_field.value <= iterations; k++)
        {
            a(this.v, this.w, this.y);
            if (k >= iterations) break;
        }

        res += this.y[0];

        try {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (c_field.value = 1; c_field.value <= iterations; k++)
            {
                a(this.v, this.w, this.y);
                if (k >= iterations) break;
            }

            if (this.y[0] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (c_field.value = 1; c_field.value <= iterations; k++)
            {
                a(this.v, this.w, this.y);
                if (k >= iterations) break;
            }

            res /= this.y[0];
        } finally {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (c_field.value = 1; c_field.value <= iterations; k++)
            {
                a(this.v, this.w, this.y);
                if (k >= iterations) break;
            }

            res *= this.y[0];
        }

        this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
        for (c_field.value = 1; c_field.value <= iterations; k++)
        {
            a(this.v, this.w, this.y);
            if (k >= iterations) break;
        }

        res -= this.y[0];

        return res;
    }

    final double test4(int n) {
        double res = 0;
        int[] arr1 = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int k = n;
        double s = 0.0d;
        c_field.value = 1;

        for (int i : arr1)
        {
            double d1= 0.499975D  + c_field.value;
            s += ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
        }

        res += (s + n + k);

        try {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (int i : arr1)
            {
                double d1= 0.499975D  + c_field.value;
                s += ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
            }

            if (this.y[0] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (int i : arr1)
            {
                double d1= 0.499975D  + c_field.value;
                s += ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
            }

            res /= (s + n + k);
        } finally {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
            for (int i : arr1)
            {
                double d1= 0.499975D  + c_field.value;
                s += ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
            }

            res *= (s + n + k);
        }

        this.y[0] = 1.0D; this.w = 1.0D; this.v = 1.0D;
        for (int i : arr1)
        {
            double d1= 0.499975D  + c_field.value;
            s += ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
        }

        res -= (s + n + k);

        return res;
    }

    final void myRun() {
        System.out.println(test1(10));
        System.out.println(test2(10));
        System.out.println(test3(10));
        System.out.println(test4(10));
    }

    public static void main(String[] args)
    {
        new Main().myRun();
    }
}

