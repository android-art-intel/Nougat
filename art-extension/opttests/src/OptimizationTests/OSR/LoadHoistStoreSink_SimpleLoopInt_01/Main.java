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

package OptimizationTests.OSR.LoadHoistStoreSink_SimpleLoopInt_01;

public class Main
{
    public class A 
    {
        public int value;
    }
    public int field = 0;
    public int testLoop()
    {
        A x;
        x = new A();
        int a0 = 0x70000007;

        // set/get pair will be hoisted/sunk out of the loop
        for (int j =0; j < 100*1000*1000; j++) {     
            x.value++;
        }

        return x.value;
    }


    public static void main(String[] args)
    {
        System.out.println(new Main().testLoop());        
    }    
}
