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

package OptimizationTests.ShortLeafMethodsInlining.InvokeDirect_add_double_tc_001;
class Main {
    final static int iterations = 10;

    public static void main(String[] args) {
        Test test = new Test();
        double workJ = 10.0;
        double workK = 15.0;

        System.out.println("Initial workJ value is " + workJ);

        double j = -1.0;
        try {
            throw new Exception("Test");
        } catch (Exception e) {
            j += 1.0;
        }

        for(double i = 0.0; i < iterations; i++) {
            workJ = test.shim(workJ, workK) + i;
        }

        workJ += j;

        System.out.println("Final workJ value is " + workJ);
    }
}
