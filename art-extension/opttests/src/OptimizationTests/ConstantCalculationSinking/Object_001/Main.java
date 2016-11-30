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

package OptimizationTests.ConstantCalculationSinking.Object_001;

import java.util.Random;

/**
*
* M: No limitations on basic blocks number, creating new objects allowed
* M: Expected result: 1 sinking for each method (if var to be sunk is not passed to object constructoe as parameter)
* N: more sinking due to inliner
*
**/

public class Main {                                                                                                                                                   
    final int iterations = 1100;



    class C_Int {
        int value = 0;
        public C_Int(int v) {
            value = v;
            if (v > 10) {
                    value += (int)Math.random()*10;
            } else {
                value = v;
            }
        }
    }



    /* 1 sinking on N due to inlining, no sinking on M*/
    public int testLoop1() {
        int testVar = 10000;
        int additionalVar = 0;
        C_Int c = new C_Int(3);
        for (int i = 0; i < iterations; i++) {
            testVar += 5;

            if (i>100) {
                additionalVar++;
            } else {
                if (i <80) {
                    additionalVar += 2;
                } else {
                    additionalVar += 3;
                }
            }
            

            additionalVar += (new C_Int(3)).value;
        }

        return testVar + additionalVar ;
    }

    /* 1 sinking */
    public float testLoop2(int n) {
        int additionalVar = 0;
        float testVar = 0.5f;        

        
        for (int i = 0; i < 35; i++) {
            testVar *= 0.5f;
            additionalVar += (new C_Int(i)).value + n/2;
        }

        return testVar + additionalVar ;
    }



    /* 1 sinking */
    public long testLoop3(long testVar) {
        long additionalVar = 0;


        for (int i = 0; i < iterations; i++) {
            testVar -= 10;
            additionalVar += (new C_Int(i)).value;
        }

        return testVar + additionalVar ;
    }

    /* no sinking: testVar is used again in the loop */
    public double testLoop4(int n) {
        int additionalVar = 0;
        double testVar = Double.MAX_VALUE - 10;

        for (int i = 0; i < 51; i++) {
            testVar *= -3d;
            additionalVar += (new C_Int( (int)testVar)).value;
        }

        return testVar + additionalVar ;
    }

    /* 1 sinking on N due to inlining, no sinking on M*/
    public int testLoop5(int n) {
        int testVar = 1;
        int additionalVar = 0;

      C_Int c = new C_Int(testVar);

        for (int i = 0; i < 9; i++) {
            testVar *= (new C_Int(3)).value;
            additionalVar += (new C_Int(i + n)).value + i%2 + i%3 + i%4 + i%4 + i%6 + i%7 +i%8 +i%9 + i%10 + c.value;
        }

        return testVar + additionalVar ;
    }

    public static void main(String[] args)
    {
         System.out.println(new Main().testLoop1());
         System.out.println(new Main().testLoop2(10));
         System.out.println(new Main().testLoop3(Long.MIN_VALUE));
         System.out.println(new Main().testLoop4(10));
         System.out.println(new Main().testLoop5(6));
    }

}  

