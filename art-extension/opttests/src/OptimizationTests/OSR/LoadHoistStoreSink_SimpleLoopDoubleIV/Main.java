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

package OptimizationTests.OSR.LoadHoistStoreSink_SimpleLoopDoubleIV;

public class Main
{
    public class A 
    {
        public double value;
    }

    public double testLoop()
    {
        A x;
        x = new A();
        double a0 = 1.7777777;

        for (double i = 0.2; i < 10*1000*1000; i += 0.99)
        {
            x.value = a0;
            a0 += i;
        }

        return x.value;
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
