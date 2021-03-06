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

package OptimizationTests.LoadHoistStoreSink.RangeCheckSetter_2;

// negative, memory aliasing set:
// loadhoist_storesink: Set v68 ArraySet [ l113 i115 i57 ] aliases with instruction i57 ArrayGet [ l14 i94 ]

public class Main
{
    public int[] value = new int [3];

    public int testLoop()
    {
        int[] arr = new int[100];
        for (int i = 0; i < 100; i++) {
            arr[i] = 1 + i + i * i;
            value[0] = arr[i];
        }

        return value[0];
    }

    public void test()
    {
        System.out.println(testLoop());
    }
    public static void main(String[] args)
    {
        new Main().test();        
    }    
}
