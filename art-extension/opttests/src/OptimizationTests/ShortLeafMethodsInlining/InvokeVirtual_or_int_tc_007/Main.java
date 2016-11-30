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

package OptimizationTests.ShortLeafMethodsInlining.InvokeVirtual_or_int_tc_007;
class Main {
    final static int iterations = 10;

    public static int outer_method(Test t, int workJ, int workK) throws Exception {
        try {
            return t.simple_2method(workJ, workK);
        } catch (Exception e) {
            return t.simple_2method(workJ, workK);
        } finally {
            return t.simple_2method(workJ, workK);
        }
    }

    public static void main(String[] args) {
        Test test = new Test();
        int workJ = 10;
        int workK = 15;

        System.out.println("Initial workJ value is " + workJ);

        try {
            outer_method(test, workK, workJ);
        } catch (Exception e) {
            workK++;
        } finally {
            workK--;
        }

        for(int i = 0; i < iterations; i++) {
            try {
                workJ = test.simple_method(workJ, workK) + i;
            } catch (Exception e) {
                workJ = test.simple_1method(workJ, workK) + i;
            }
        }

        System.out.println("Final workJ value is " + workJ);
    }
}
