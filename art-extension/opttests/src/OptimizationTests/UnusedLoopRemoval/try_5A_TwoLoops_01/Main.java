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

package OptimizationTests.UnusedLoopRemoval.try_5A_TwoLoops_01;

public class Main {

    public static int loop() {
        int res = 0;
        try { throw new Exception();}
        catch (Exception e){ res++; }
        finally { res-=2; }
        for (int i = 0; i < 10000; i++) {
            res += 5;
        }
        for (int j = 0; j < 10000; j++) {
            res -= 3;
        }
        return res;
    }

    public static void main(String[] args) {
        int res = new Main().loop();
        System.out.println(res);
    }
}
