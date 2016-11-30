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

package OptimizationTests.LoopUnrolling.IntLoopTryCatchBefore;

public class Main {

    public class TestClass {
        private int value;
 
        public void setValue(int value) {
            this.value += value;
        }

        public int getValue() {
            return value;
        }
    }

    final int testLoop() {
        TestClass tc = new TestClass();
        int a = 0x70000007;

        for (int i = 0; i < 10; i++) {
            a++;
            tc.setValue(a);
        }
        return tc.getValue();
    }

    final int $noinline$testTryCatch() {
        int res = 0;
        res += testLoop();
        try {
            throw new Exception("My exception"); 
        } catch (Exception e) {
        } finally {
        }
        return res;
    }

    public void test() {
        System.out.println($noinline$testTryCatch());
    }

    public static void main(String[] args) {
        new Main().test();
    }    
}
