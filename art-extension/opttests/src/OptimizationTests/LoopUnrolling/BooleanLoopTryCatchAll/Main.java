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

package OptimizationTests.LoopUnrolling.BooleanLoopTryCatchAll;

public class Main {

    public class TestClass {
        private boolean value;
 
        public void setValue(boolean value) {
            this.value |= value;
        }

        public boolean getValue() {
            return value;
        }
    }

    final boolean testLoop() {
        TestClass tc = new TestClass();
        boolean a0 = true;
        boolean b0 = false;

        for (int i = 0; i < 10; i++) {
            tc.setValue(a0 || b0 || (i != 0));
        }
        return tc.getValue();
    }

    final boolean $noinline$testTryCatch() {
        boolean res = false;
        res |= testLoop();
        try {
            if (testLoop() == true) {
                throw new Exception("My exception");
            } 
        } catch (Exception e) {
            res ^= testLoop();
        } finally {
            res &= testLoop();
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
