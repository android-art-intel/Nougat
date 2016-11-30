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

package OptimizationTests.OSR.LoadHoistStoreSink_TryCatch_06;

public class Main
{
    public class A 
    {
        public int value = 1000;
        public double dValue = 0.567;
    }
    public int field = 0;


    public void inlineMe(A x) {
        int j = 0;
        // set/get pair will be hoisted/sunk out of the loop
        do  {     
            x.value += 1;
            x.dValue +=0.345;
        } while ( j++ < 30*1000);

        x.value += 5;

    }

    // Just a little variation of LoadHoistStoreSink_TryCatch_01
    // After inlining we will have outer loop handled by OSR and inner loop will have LHSS applied
   // According to QA doc: * OSR injections will not be done to Try block. - but in this case OSR is applied w/o silver optimizations to outer loop only
    public double testLoop()
    {
        A x;
        x = new A();
        int a0 = 0x70000007;
        int j = 0;
        A y = new A();
        y = null;


        try {
            do  
            {
                inlineMe(x);
                x.value += 100/(29*10*1000 - j);
               inlineMe(new A());
            } while (j++ < 30*10*1000);
            x.value -= 100/(29*10*1000 - j - 1);
        } catch (Exception e) {
            System.out.println("Exception caught");
            x.value += 7;
        }

        return x.value + x.dValue;
    }


    public static void main(String[] args)
    {
        System.out.println(new Main().testLoop());        
    }    
}
