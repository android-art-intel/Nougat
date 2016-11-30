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

package OptimizationTests.OSR.Devirtualization_InvokeVirtualInt;

class Main {

    public static int runTest() {
                CondVirtBase test = new CondVirtExt();
		int result = 0;
       
        for (int i = 0; i < 100*1000; i++) {
            result += test.getThingies();
        }
		return result;
    }

    public static void main(String[] args) {
        int result = runTest();
        System.out.println("Result " + result);
    }

}
