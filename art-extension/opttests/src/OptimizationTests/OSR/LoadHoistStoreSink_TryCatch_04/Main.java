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

package OptimizationTests.OSR.LoadHoistStoreSink_TryCatch_04;

public class Main
{
    public class A 
    {
        public int value = 1000;
    }
    public int field = 0;

    //OSR + LHSS before try-catch block
    public int testLoop(int n)
    {
        A x;
        x = new A();
        A y = new A();
        int a0 = 0x70000007;
        int j = 0;
        do  {     
            x.value++;
        } while ( j++ < 30*100*1000);



        try {
                x.value += n/(n - 10);
        } catch (Exception e) {
            System.out.println("Exception caught");
            x.value++;
        } finally {
            j = 0;
            y.value--;
        }

        return x.value + y.value;
    }


    public static void main(String[] args)
    {
        System.out.println(new Main().testLoop(10));        
    }    
}
