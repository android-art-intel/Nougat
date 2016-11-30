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

package OptimizationTests.OSR.LoadHoistStoreSink_NestedLoops;

public class Main
{
    public class A 
    {
        public int value;
        protected float fvalue;
    }

    public int testLoop()
    {
        A x;
        x = new A();
        int a0 = 0x70000007;
        A y = new A();
        float b0 = -1.2345f;

        int j = 1;
        for (int k = 0; k < 20; k += 2) {
            do {
                for (int i = 0; i < 100*1000; i++)
                {
                    x.value = a0;
                    a0++;
                }
                x.value = a0;
                a0++;
                for (int i = 0; i < 100*1000; i++)
                {
                    x.fvalue = b0;
                    b0 += 2;
                }
                b0--;
                x.fvalue = b0;
                for (int i = 0; i < 100*1000; i++)
                {
                    x.value = a0;
                    a0 += 3;
                }

                x.value = a0;
                a0--;

                int l = 0;
                while (true) {
                y.fvalue = b0;
                for (int i = 10*1000; i > 0; i--)
                {
                    y.fvalue = b0;
                    b0 -=1;
                }
                if (l++ > 100) break;
                }

            }
            while (j++ < 200);
        }


        return x.value + (int)x.fvalue + y.value + (int)y.fvalue;
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
