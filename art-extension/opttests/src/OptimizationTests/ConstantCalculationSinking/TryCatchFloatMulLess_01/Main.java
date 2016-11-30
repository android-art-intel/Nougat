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

package OptimizationTests.ConstantCalculationSinking.TryCatchFloatMulLess_01;

/**
*
* Simplest case when optimization should be applied
*  1 sinking expected
*
**/

public class Main {                                                                                                                                                   
    final int iterationsLessThanThreshold = 50;

    final float testLoopMulLess(float n) {
        float testVar = 1.0f;
        float additionalVar = 0;
        for (int i = -10; i < iterationsLessThanThreshold; i++) {
              testVar *= 1.1f;
              additionalVar += (i*2)/5 + n;
        }
        testVar += additionalVar;
        return testVar;
    }

    final float $noinline$testTryCatch(float n) {
        float res = 0;
        res += testLoopMulLess(n);
        try {
            if (testLoopMulLess(n) > 0) {
                throw new Exception("My exception"); 
            }
        } catch (Exception e) {
            res /= testLoopMulLess(n);
        } finally {
            res -= testLoopMulLess(n);
        }
        res *= testLoopMulLess(n);
        return res;
    }

    public static void main(String[] args) {
         System.out.println(Float.floatToIntBits(new Main().$noinline$testTryCatch(1.2f)));
    }
}  

