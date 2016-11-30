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

package OptimizationTests.ValuePropagationThroughHeap.ForLoopAtomicInstance_03;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 *
 * Atomic test cases when optimization should be applied: 'for' loop plus in-lining
 * The iteration count is not known at compile time, VPTH optimization should be applied
 *
 **/

public class Main {                                                                                                                                                   
    static int iterations = 1100;

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



/* Positive: atomic tests for various iput / iget instructions */

/* iput / iget
*/
   public int test1() {
        this.i_arr[iterations-1] = 1; this.i1 = 2; this.i2 = 3;
        for (int i = 1; i <= this.iterations; i++)
        {
            a1(this.i1, this.i2,this.i_arr);
        }
        return i_arr[0];
    }

    private void a1(int paramInt1, int paramInt2, int[] paramArrayOfInt)
    {
        int ii = 123456 * (paramInt1 + paramInt2);
        paramArrayOfInt[0] = ii;
    }

/* iput-wide / iget-wide
*/
    public double test2() {
        this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
        for (int i = 1; i <= this.iterations; i++) 
        { 
            a2(this.d1, this.d2, this.d_arr);
        } 
        return d_arr[0];
    }

    private void a2(double paramDouble1, double paramDouble2, double[] paramArrayOfDouble)
    {
        double dd = 0.123456D * (paramDouble1 + paramDouble2);
        paramArrayOfDouble[0] = dd;
    }


/* iput-short / iget-short
*/
    public short test3() {
        this.sh_arr[0] = 11; this.sh1 = 22; this.sh2 = 33;
        for (int i = 1; i <= this.iterations; i++)
        {
            a3(this.sh1, this.sh2, this.sh_arr);
        }
        return sh_arr[0];
    }

    private void a3(short paramShort1, short paramShort2, short[] paramArrayOfShort)
    {
        short shsh = (short)(12345 * (paramShort2 - paramShort1));
        paramArrayOfShort[0] += shsh;
    }

/* iput-char / iget-char
*/
    public char test4() {
        this.ch_arr[0] = 'a'; this.ch1 = 'b'; this.ch2 = 'c';
        for (int i = 1; i <= this.iterations; i++)
        {
            a4(this.ch1, this.ch2, this.ch_arr);
        }
        return ch_arr[0];
    }

    private void a4(char paramChar1, char paramChar2, char[] paramArrayOfChar)
    {
        char chch = (char)('f' + (paramChar2 - paramChar1));
        paramArrayOfChar[0] += chch;
    }

/* iput-byte / iget-byte
*/
    public byte test5() {
        this.b_arr[0] = 1; this.b1 = 2; this.b2 = 3;
        for (int i = 1; i <= this.iterations; i++)
        {
            a5(this.b1, this.b2, this.b_arr);
        }
        return b_arr[0];
    }

    private void a5(byte paramByte1, byte paramByte2, byte[] paramArrayOfByte)
    {
        byte bb = (byte)(12 * (paramByte2 - paramByte1));
        paramArrayOfByte[0] += bb;
    }

/* iput-boolean / iget-boolean
*/
    public boolean test6() {
        this.bl_arr[0] = false; this.bl1 = true; this.bl2 = true;
        for (int i = 1; i <= this.iterations; i++)
        {
            a6(this.bl1, this.bl2, this.bl_arr);
        }
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

