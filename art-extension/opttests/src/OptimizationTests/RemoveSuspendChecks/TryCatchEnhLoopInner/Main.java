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

package OptimizationTests.RemoveSuspendChecks.TryCatchEnhLoopInner;

public class Main {
	
	
    public static int testLoop(int[] arr1) {
        int sum = 0;
				
		for (int i : arr1){
			sum += i;			
		}
		
        return sum;
    }

    public void $noinline$test() {
		int res = 0;
		
		final int[] arr1 = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
		final int[] arr2 = {13, 3, 45, 324, 32, 11, 643, 445, 23, 3};
	
		res += testLoop(arr1);
	
		try {
			if (testLoop(arr2) > 0)
				throw new Exception("expected");
		} catch (Exception e) {
			res /= testLoop(arr1);
		} finally {
			res -= testLoop(arr2);
		}
		
		res *= testLoop(arr1);
	
		System.out.println(res);
    }

    public static void main(String[] args) {
        new Main().$noinline$test();
    }
}


