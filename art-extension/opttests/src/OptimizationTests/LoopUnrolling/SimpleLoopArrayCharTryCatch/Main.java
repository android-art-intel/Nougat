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

package OptimizationTests.LoopUnrolling.SimpleLoopArrayCharTryCatch;

public class Main {
    public char[]  value = new char[3];

    public final char testLoop(char a0) {
        int iterations = 8;
        for (int i = 0; i < iterations; i++) {
            a0 = 'b';
            value[0] = a0;
            value[1] = (char)(i * 0x11);
        }
        return value[1];
    }

    final int $noinline$testTryCatch(char a) {
        int res = 0;
        res += testLoop(a);
        try {
            if (testLoop(a) > 0) {
                throw new Exception("My exception");
            } 
        } catch (Exception e) {
            res ^= testLoop(a);
        } finally {
            res *= testLoop(a);
        }
        return res;
    }

    public void test() {
        System.out.println($noinline$testTryCatch('a'));
    }
    
    public static void main(String[] args) {
        new Main().test();        
    }    
}
