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

package OptimizationTests.OSR.LoadHoistStoreSink_TryCatch_02;

public class Main
{
    public class A 
    {
        public int value = 1000;
    }
    public int field = 0;


    public void inlineMe(A x) {
        int j = 0;
        // set/get pair will be hoisted/sunk out of the loop
        do  {     
            x.value++;
//            x.value /= 2;
        } while ( j++ < 30*1000);


    }

    //after inlining we will have OSR + LHSS in catch block
    public int testLoop(int n)
    {
        A x;
        x = new A();
        int a0 = 0x70000007;
        int j = 0;


        try {
                x.value += n/(n - 10);
        } catch (Exception e) {
            System.out.println("Exception caught");
            do  {
                x.value++;
                inlineMe(x);
            } while (j++ < 30*10*1000);
        }

        return x.value;
    }


    public static void main(String[] args)
    {
        System.out.println(new Main().testLoop(10));        
    }    
}
