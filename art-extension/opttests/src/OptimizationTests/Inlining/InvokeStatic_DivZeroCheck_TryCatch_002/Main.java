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

package OptimizationTests.Inlining.InvokeStatic_DivZeroCheck_TryCatch_002;
// InvokeStatic

public class Main
{
    public static long b(long div) {
        if (div == 10L) {
            div = 0L;
        }
        long d = 1L / div;
        return d;
    }

    public static int d(int div) {
        return 1 / div;
    }

    public void test() {
        long c = 0L;
        for (int i = 1; i < 11; i++) {
            try {
                c = (long)d(0);
            } catch (Exception e) {
                c = b(i);
            }
            System.out.println(c);
        }
        System.out.println(c);
    }

    public static void main(String[] args)
    {
        new Main().test();
    }
}
