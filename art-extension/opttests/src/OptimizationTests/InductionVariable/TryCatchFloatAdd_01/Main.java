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

package OptimizationTests.InductionVariable.TryCatchFloatAdd_01;

/**
 * myLoop will be inlined into test().
 * so it's expected that "IV.*loop increment: 1" ... "IV.*loop increment: 5" will be found in logcat
 */
 
public class Main {

	static float threshold = 10.4f;

	final float myLoop(float step) {
		float i = 0.0f;
		
		while (i < threshold) {
			i += step;
		}
		
		return i;
	}

	public void $noinline$test() {	
		float res = 7.2f;
	
		res += myLoop(1.1f); // Before try-catch.
	
		try {
			if (myLoop(2.2f) > 0)
				throw new Exception("expected"); // In Try section
		} catch (Exception e) {
			res /= myLoop(3.3f); // In catch secton.
		} finally {
			res -= myLoop(4.4f); // in Finally section.
		}
		
		res *= myLoop(5.5f); // After try-catch.
	
		System.out.println(res);
	}
	

	public static void main(String[] args) {
		new Main().$noinline$test();
	}
}