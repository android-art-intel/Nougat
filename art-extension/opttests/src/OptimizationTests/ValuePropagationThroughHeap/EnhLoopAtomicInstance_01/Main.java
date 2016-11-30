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

package OptimizationTests.ValuePropagationThroughHeap.EnhLoopAtomicInstance_01;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 *
 * Atomic test cases when optimization should be applied: 'enhanced for' loop
 *
 **/

public class Main {                                                                                                                                                   
    final int iterations = 1100;
    int[] arr1 = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    int i1;
    int i2;
    int[] i_arr = new int[iterations];

    double d1;
    double d2;
    double[] d_arr = new double[iterations];

    short sh1;
    short sh2;
    short[] sh_arr = new short[iterations];

    char ch1;
    char ch2;
    char[] ch_arr = new char[iterations];

    byte b1;
    byte b2;
    byte[] b_arr = new byte[iterations];

    boolean bl1;
    boolean bl2;
    boolean[] bl_arr = new boolean[iterations];



/* Positive: atomic tests for various iget/iput instructions */

/* iput / iget
*/
   public int test1() {
        int ii;
        this.i_arr[iterations-1] = 1; this.i1 = 2; this.i2 = 3;
        for (int i : arr1)
        {
            ii = 123456 * (this.i1 + this.i2);
            this.i_arr[iterations-1] = ii;
        }
        return i_arr[iterations-1];
    }

/* iput-wide / iget-wide
*/
    public double test2() {
        double dd;
        this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
        for (int i : arr1)
        { 
            dd = 0.123456D * (this.d1 + this.d2);
            this.d_arr[0] = dd;
        } 
        return d_arr[0];
    }

/* iput-short / iget-short
*/
    public short test3() {
        short shsh;
        this.sh_arr[0] = 11; this.sh1 = 22; this.sh2 = 33;
        for (int i : arr1)
        {
            shsh = (short)(12345 * (this.sh2 - this.sh1));
            this.sh_arr[0] = (short)(this.sh_arr[0] + shsh);
        }
        return sh_arr[0];
    }

/* iput-char / iget-char
*/
    public char test4() {
        char chch;
        this.ch_arr[0] = 'a'; this.ch1 = 'b'; this.ch2 = 'c';
        for (int i : arr1)
        {
            chch = (char)('f' + (this.ch2 - this.ch1));
            this.ch_arr[0] = (char)(this.ch_arr[0] + chch);
        }
        return ch_arr[0];
    }

/* iput-byte / iget-byte
*/
    public byte test5() {
        byte bb;
        this.b_arr[0] = 1; this.b1 = 2; this.b2 = 3;
        for (int i : arr1)
        {
            bb = (byte)(12 * (this.b2 - this.b1));
            this.b_arr[0] = (byte)(this.b_arr[0] + bb);
        }
        return b_arr[0];
    }

/* iput-boolean / iget-boolean
*/
    public boolean test6() {
        boolean blbl;
        this.bl_arr[0] = false; this.bl1 = true; this.bl2 = true;
        for (int i : arr1)
        {
            blbl = ( true | (this.bl2 & this.bl1));
            this.bl_arr[0] = !this.bl_arr[0] ^ blbl;
        }
        return bl_arr[0];
    }

    public void runTests() {
        Class c = new Main().getClass();
        Method[] methods = c.getDeclaredMethods();
        Method tempMethod;
        for (int i = 1; i < methods.length; i++) {
            int j = i;
            while (j > 0 &&  methods[j-1].getName().compareTo(methods[j].getName()) > 0) {
                tempMethod = methods[j];
                methods[j] = methods[j-1];
                methods[j-1] = tempMethod;
                j = j - 1;
            }
        }

        Object[] arr = {null};
        for (Method m: methods){
            if (m.getName().startsWith("test")){
                try {
                    String names[] = c.getPackage().getName().split("\\.");
                    String testName = names[names.length-1];
                    System.out.println("getGenericReturnType() = " + m.getGenericReturnType());
                    if (m.getGenericReturnType().toString().equals("char")){
                       System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: "+ String.format ("\\u%04x", (int)(m.invoke(this).toString().charAt(0))));
                    }
                    else {
                       System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: "+ m.invoke(this));
                    }
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static void main(String[] args)
    {
        new Main().runTests();
    }

}  

