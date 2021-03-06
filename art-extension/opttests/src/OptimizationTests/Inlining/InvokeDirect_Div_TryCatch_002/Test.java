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

package OptimizationTests.Inlining.InvokeDirect_Div_TryCatch_002;
// InvokeDirect:

class Test
{
    private double b(int div) {
        return 1.0d / div;
    }

    private int d(int div) {
        return 1 / div;
    }

    public void a(int iter) {
        double c = 0.0d;
        for (int i = 1; i < iter; i++) {
            try {
                c = (double)d(0);
            } catch (Exception e) {
                c = b(i);
            }
            System.out.println(c);
        }
        System.out.println(c);
    }
}
