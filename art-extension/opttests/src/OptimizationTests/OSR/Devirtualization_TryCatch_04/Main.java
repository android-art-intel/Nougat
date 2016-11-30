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

package OptimizationTests.OSR.Devirtualization_TryCatch_04;

class Main {

    public static int runTest(int n ) {
        CondVirtBase test = new CondVirtExt();
        int result = 0;


        // try-catch before and after OSR candidate loop: OSR + devirtualization is applied
        // nested loops
        try {
            if (n > 0) 
                throw new Exception("Exception 1");
        } catch (Exception e) {
            System.out.println(e);
        }
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                for (int k = 0; k < 10*1000*1000; k++) {
                    result += test.getThingies();
                }
                for (int k = 0; k < 10*1000*1000; k++) {
                    result += test.getThingies();
                }

            }
        }
        try {
            if (n > 0) 
                throw new Exception("Exception 2");
        } catch (Exception e) {
            System.out.println(e);
        }

        return result;
    }

    public static void main(String[] args) {
        int result = runTest(5);
        System.out.println("Result " + result);
    }

}
