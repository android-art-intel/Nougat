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

package OptimizationTests.ValuePropagationThroughHeap.ForLoopAtomicStatic_01;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 *
 * Atomic test cases when optimization should be applied: 'for' loop
 *
 **/

public class Main {                                                                                                                                                   
    static final int iterations = 1100;

    static int i1;
    static int i2;
    static int[] i_arr = new int[iterations];

    static double d1;
    static double d2;
    static double[] d_arr = new double[iterations];

    static short sh1;
    static short sh2;
    static short[] sh_arr = new short[iterations];

    static char ch1;
    static char ch2;
    static char[] ch_arr = new char[iterations];

    static byte b1;
    static byte b2;
    static byte[] b_arr = new byte[iterations];

    static boolean bl1;
    static boolean bl2;
    static boolean[] bl_arr = new boolean[iterations];



/* Positive: atomic tests for various sget/sput instructions */

/* sput / sget
*/
   public int test1() {
        int ii;
        i_arr[iterations-1] = 1; i1 = 2; i2 = 3;
        for (int i = 1; i <= iterations; i++)
        {
            ii = 123456 * (i1 + i2);
            i_arr[iterations-1] = ii;
        }
        return i_arr[iterations-1];
    }

/* sput-wide / sget-wide
*/
    public double test2() {
        double dd;
        d_arr[0] = 1.0D; d1 = 2.0D; d2 = 3.0D;
        for (int i = 1; i <= iterations; i++) 
        { 
            dd = 0.123456D * (d1 + d2);
            d_arr[0] = dd;
        } 
        return d_arr[0];
    }

/* sput-short / sget-short
*/
    public short test3() {
        short shsh;
        sh_arr[0] = 11; sh1 = 22; sh2 = 33;
        for (int i = 1; i <= iterations; i++)
        {
            shsh = (short)(12345 * (sh2 - sh1));
            sh_arr[0] = (short)(sh_arr[0] + shsh);
        }
        return sh_arr[0];
    }

/* sput-char / sget-char
*/
    public char test4() {
        char chch;
        ch_arr[0] = 'a'; ch1 = 'b'; ch2 = 'c';
        for (int i = 1; i <= iterations; i++)
        {
            chch = (char)('f' + (ch2 - ch1));
            ch_arr[0] = (char)(ch_arr[0] + chch);
        }
        return ch_arr[0];
    }

/* sput-byte / sget-byte
*/
    public byte test5() {
        byte bb;
        b_arr[0] = 1; b1 = 2; b2 = 3;
        for (int i = 1; i <= iterations; i++)
        {
            bb = (byte)(12 * (b2 - b1));
            b_arr[0] = (byte)(b_arr[0] + bb);
        }
        return b_arr[0];
    }

/* sput-boolean / sget-boolean
*/
    public boolean test6() {
        boolean blbl;
        bl_arr[0] = false; bl1 = true; bl2 = true;
        for (int i = 1; i <= iterations; i++)
        {
            blbl = ( true | (bl2 & bl1));
            bl_arr[0] = !bl_arr[0] ^ blbl;
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

