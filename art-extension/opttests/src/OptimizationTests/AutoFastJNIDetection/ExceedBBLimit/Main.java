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

package OptimizationTests.AutoFastJNIDetection.ExceedBBLimit;


/* Test add */
public class Main {

    public  final int iterations = 5000;
    static
    {
        System.loadLibrary("JniTest");
    }

    public native int nativeExceedBBLimit(int a, int b, int c); 

    // Test case harnesses
    public int PleaseInlineMe() {
        int sum = 0;
        for (int i = 0; i < iterations; i++) {
            try {
            Thread.currentThread().sleep(1);
            } catch (Exception e) {
                System.out.println(e);
            }
            sum += nativeExceedBBLimit(-1, 20, Integer.MAX_VALUE/10000);
        }
        return sum;
    }




    public static void main(String[] args)
    {
        Main test = new Main();
        System.out.println("result: " + test.PleaseInlineMe());
        System.out.println("Test completed");
    }

}
