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

package OptimizationTests.OSR.Devirtualization_TryCatch_03;

class Main {

    public static int runTest(int n ) {
                CondVirtBase test = new CondVirtExt();
		int result = 0;
       

        // devirtualization inside finally block: OSR + devirtualization is applied
        try {
            if (n > 0) 
            throw new Exception("Exception 1");
        } catch (Exception e) {
            if ( n > 5)
            throw new Exception("Exception 1");
        } finally {
            for (int i = 0; i < 10*1000*1000; i++) {
                result += test.getThingies();
            }
		    return result;
        }
    }

    public static void main(String[] args) {
        int result = runTest(10);
        System.out.println("Result " + result);
    }

}
