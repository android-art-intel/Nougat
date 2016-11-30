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

package OptimizationTests.ValuePropagationThroughHeap.TryCatchCombined_01;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    final int iterations = 1100;
    double d = 0;
    double[] y = new double[iterations];
    double w = 1.0d;
    double v = 1.0d;

    final double testDo(int n) {
        int res = 0;
        this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D;
        double s = 0d;
        this.d = 1;
        do { 
            double d1= 0.499975D * (this.w + this.v);
            s = ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
            this.d = (1 + this.d);
        } while (this.d <= this.iterations);

        res += this.y[0];

        try {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D; s = 0d; this.d = 1;
            do { 
                double d1= 0.499975D * (this.w + this.v);
                s = ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
                this.d = (1 + this.d);
            } while (this.d <= this.iterations);

            if (this.w > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D; s = 0d; this.d = 1;
            do { 
                double d1= 0.499975D * (this.w + this.v);
                s = ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
                this.d = (1 + this.d);
            } while (this.d <= this.iterations);

            res /= this.y[0] + this.v;
        } finally {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D; s = 0d; this.d = 1;
            do { 
                double d1= 0.499975D * (this.w + this.v);
                s = ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
                this.d = (1 + this.d);
            } while (this.d <= this.iterations);

            res -= this.y[0] + (int)this.v;
        }

        this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D; s = 0d; this.d = 1;
        do { 
            double d1= 0.499975D * (this.w + this.v);
            s = ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
            this.d = (1 + this.d);
        } while (this.d <= this.iterations);

        res *= this.y[0] + this.v + this.w;

        return res;
    }
    
    final double testFor(int n) {
        int res = 0;
        this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D; double s = 0d;
        for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d))
        {
            double d1= 0.499975D * (this.w + this.v);
            s = ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
        }

        res += this.y[0] + this.w;

        try {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D; s = 0d;
            for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d))
            {
                double d1= 0.499975D * (this.w + this.v);
                s = ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
            }

            if (this.v  > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D; s = 0d;
            for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d))
            {
                double d1= 0.499975D * (this.w + this.v);
                s = ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
            }

            res /= this.y[0] + this.v + this.w;
        } finally {
            this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D; s = 0d;
            for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d))
            {
                double d1= 0.499975D * (this.w + this.v);
                s = ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
            }

            res -= this.y[0] + this.v;
        }

        this.y[0] = 1.0D; this.w = 1.0D; this.v = 2.0D; s = 0d;
        for (this.d = 1; this.d <= this.iterations; this.d = (1 + this.d))
        {
            double d1= 0.499975D * (this.w + this.v);
            s = ((d1 + 0.499975D * (d1 + this.w)) / 2.0D);
        }

        res *= this.y[0] + this.v + this.w;

        return res;
    }

    final void myRun() {
        System.out.println(testFor(10));
        System.out.println(testDo(10));
    }

    public static void main(String[] args)
    {
        new Main().myRun();
    }
}

