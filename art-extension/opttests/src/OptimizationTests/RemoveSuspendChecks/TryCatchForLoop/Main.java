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

package OptimizationTests.RemoveSuspendChecks.TryCatchForLoop;

public class Main {
    public static double testLoop(int m) {
        double res = 1;
        for (int i = 1; i < m; i++) {
            res = res + i * (i+1);
        }
        return res;
    }

       public void $noinline$test() {
		int res = 0;
	
		res += testLoop(50000);
	
		try {
			if (testLoop(40000) > 0)
				throw new Exception("expected");
		} catch (Exception e) {
			res /= testLoop(60000);
		} finally {
			res -= testLoop(30000);
		}
		
		res *= testLoop(70000);
	
		System.out.println(res);
    }


    public static void main(String[] args) {
        new Main().$noinline$test();
    }
}


