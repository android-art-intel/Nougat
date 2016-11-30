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

package OptimizationTests.ValuePropagationThroughHeap.DoWhileLoopAtomicStatic_03;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 *
 * Atomic test cases when optimization should be applied: 'do while' loop plus in-lining
 * The iteration count is not known at compile time, VPTH optimization should be applied
 *
 **/

public class Main {                                                                                                                                                   
    static int iterations = 1100;

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

/* Positive: atomic tests for various sput / sget instructions */

/* sput / sget
*/
   public int test1() {
        int i = 1;
        i_arr[iterations-1] = 1; i1 = 2; i2 = 3;
        do {
            a1(i1, i2,i_arr);
            i += 1;
        }  while (i <= iterations);
        return i_arr[0];
    }

    private void a1(int paramInt1, int paramInt2, int[] paramArrayOfInt)
    {
        int ii = 123456 * (paramInt1 + paramInt2);
        paramArrayOfInt[0] = ii;
    }

/* sput-wide / sget-wide
*/
    public double test2() {
        int i = 1;
        d_arr[0] = 1.0D; d1 = 2.0D; d2 = 3.0D;
        do { 
            a2(d1, d2, d_arr);
            i += 1;
        } while (i <= iterations);
        return d_arr[0];
    }

    private void a2(double paramDouble1, double paramDouble2, double[] paramArrayOfDouble)
    {
        double dd = 0.123456D * (paramDouble1 + paramDouble2);
        paramArrayOfDouble[0] = dd;
    }


/* sput-short / sget-short
*/
    public short test3() {
        int i = 1;
        sh_arr[0] = 11; sh1 = 22; sh2 = 33;
        do {
            a3(sh1, sh2, sh_arr);
            i += 1;
        } while (i <= iterations);
        return sh_arr[0];
    }

    private void a3(short paramShort1, short paramShort2, short[] paramArrayOfShort)
    {
        short shsh = (short)(12345 * (paramShort2 - paramShort1));
        paramArrayOfShort[0] += shsh;
    }

/* sput-char / sget-char
*/
    public char test4() {
        int i = 1;
        ch_arr[0] = 'a'; ch1 = 'b'; ch2 = 'c';
        do {
            a4(ch1, ch2, ch_arr);
            i += 1;
        } while (i <= iterations);
        return ch_arr[0];
    }

    private void a4(char paramChar1, char paramChar2, char[] paramArrayOfChar)
    {
        char chch = (char)('f' + (paramChar2 - paramChar1));
        paramArrayOfChar[0] += chch;
    }

/* sput-byte / sget-byte
*/
    public byte test5() {
        int i = 1;
        b_arr[0] = 1; b1 = 2; b2 = 3;
        do {
            a5(b1, b2, b_arr);
            i += 1;
        } while (i <= iterations);
        return b_arr[0];
    }

    private void a5(byte paramByte1, byte paramByte2, byte[] paramArrayOfByte)
    {
        byte bb = (byte)(12 * (paramByte2 - paramByte1));
        paramArrayOfByte[0] += bb;
    }

/* sput-boolean / sget-boolean
*/
    public boolean test6() {
        int i = 1;
        bl_arr[0] = false; bl1 = true; bl2 = true;
        do {
            a6(bl1, bl2, bl_arr);
            i += 1;
        } while (i <= iterations);
        return bl_arr[0];
    }

    private void a6(boolean paramBoolean1, boolean paramBoolean2, boolean[] paramArrayOfBoolean)
    {
        boolean blbl = ( true | (paramBoolean2 & paramBoolean1));
        paramArrayOfBoolean[0] = !paramArrayOfBoolean[0] ^ blbl;
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
        if (args.length > 0) {
           iterations = Integer.parseInt(args[0]);
        }

        new Main().runTests();
    }

}  

