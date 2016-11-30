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

package OptimizationTests.ValuePropagationThroughHeap.TryCatchAtomicStatic_01;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    final static int iterations = 1100;

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

    final int test1(int n) {
        int res = 0;
        int ii;
        int i = 1;
        i_arr[iterations-1] = 1; i1 = 2; i2 = 3;
        do {
            ii = 123456 * (i1 + i2);
            i_arr[iterations-1] = ii;
            i += 1;
        } while (i <= iterations);

        res += i_arr[iterations-1];

        try {
            i_arr[iterations-1] = 1; i1 = 2; i2 = 3;
            do {
                ii = 123456 * (i1 + i2);
                i_arr[iterations-1] = ii;
                i += 1;
            } while (i <= iterations);

            if (i_arr[iterations-1] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            i_arr[iterations-1] = 1; i1 = 2; i2 = 3;
            do {
                ii = 123456 * (i1 + i2);
                i_arr[iterations-1] = ii;
                i += 1;
            } while (i <= iterations);

            res /= i_arr[iterations-1];
        } finally {
            i_arr[iterations-1] = 1; i1 = 2; i2 = 3;
            do {
                ii = 123456 * (i1 + i2);
                i_arr[iterations-1] = ii;
                i += 1;
            } while (i <= iterations);

            res -= i_arr[iterations-1];
        }

        i_arr[iterations-1] = 1; i1 = 2; i2 = 3;
        do {
            ii = 123456 * (i1 + i2);
            i_arr[iterations-1] = ii;
            i += 1;
        } while (i <= iterations);

        res *= i_arr[iterations-1];

        return res;
    }
    
    final double test2(int n) {
        int res = 0;
        double dd;
        int i = 1;
        this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
        do { 
            dd = 0.123456D * (this.d1 + this.d2);
            this.d_arr[0] = dd;
            i += 1;
        } while (i <= this.iterations);

        res += d_arr[0];

        try {
            this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
            do { 
                dd = 0.123456D * (this.d1 + this.d2);
                this.d_arr[0] = dd;
                i += 1;
            } while (i <= this.iterations);

            if (d_arr[0] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
            do { 
                dd = 0.123456D * (this.d1 + this.d2);
                this.d_arr[0] = dd;
                i += 1;
            } while (i <= this.iterations);

            res /= d_arr[0];
        } finally {
            this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
            do { 
                dd = 0.123456D * (this.d1 + this.d2);
                this.d_arr[0] = dd;
                i += 1;
            } while (i <= this.iterations);

            res -= d_arr[0];
        }

        this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
        do { 
            dd = 0.123456D * (this.d1 + this.d2);
            this.d_arr[0] = dd;
            i += 1;
        } while (i <= this.iterations);

        res *= d_arr[0];

        return res;
    }

    final short test3(int n) {
        short res = 0;
        short shsh;
        int i = 1;
        this.sh_arr[0] = 11; this.sh1 = 22; this.sh2 = 33;
        do {
            shsh = (short)(12345 * (this.sh2 - this.sh1));
            this.sh_arr[0] = (short)(this.sh_arr[0] + shsh);
            i += 1;
        } while (i <= this.iterations);

        res += sh_arr[0];

        try {
            this.sh_arr[0] = 11; this.sh1 = 22; this.sh2 = 33;
            do {
                shsh = (short)(12345 * (this.sh2 - this.sh1));
                this.sh_arr[0] = (short)(this.sh_arr[0] + shsh);
                i += 1;
            } while (i <= this.iterations);

            if (sh_arr[0] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.sh_arr[0] = 11; this.sh1 = 22; this.sh2 = 33;
            do {
                shsh = (short)(12345 * (this.sh2 - this.sh1));
                this.sh_arr[0] = (short)(this.sh_arr[0] + shsh);
                i += 1;
            } while (i <= this.iterations);

            res /= sh_arr[0];
        } finally {
            this.sh_arr[0] = 11; this.sh1 = 22; this.sh2 = 33;
            do {
                shsh = (short)(12345 * (this.sh2 - this.sh1));
                this.sh_arr[0] = (short)(this.sh_arr[0] + shsh);
                i += 1;
            } while (i <= this.iterations);

            res -= sh_arr[0];
        }

        this.sh_arr[0] = 11; this.sh1 = 22; this.sh2 = 33;
        do {
            shsh = (short)(12345 * (this.sh2 - this.sh1));
            this.sh_arr[0] = (short)(this.sh_arr[0] + shsh);
            i += 1;
        } while (i <= this.iterations);

        res *= sh_arr[0];

        return res;
    }

    final int test4(int n) {
        char res = 0;
        int[] arr1 = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        char chch;
        this.ch_arr[0] = 'a'; this.ch1 = 'b'; this.ch2 = 'c';
        for (int i : arr1)
        {
            chch = (char)('f' + (this.ch2 - this.ch1));
            this.ch_arr[0] = (char)(this.ch_arr[0] + chch);
        }

        res += ch_arr[0];

        try {
            this.ch_arr[0] = 'a'; this.ch1 = 'b'; this.ch2 = 'c';
            for (int i : arr1)
            {
                chch = (char)('f' + (this.ch2 - this.ch1));
                this.ch_arr[0] = (char)(this.ch_arr[0] + chch);
            }

            if (ch_arr[0] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.ch_arr[0] = 'a'; this.ch1 = 'b'; this.ch2 = 'c';
            for (int i : arr1)
            {
                chch = (char)('f' + (this.ch2 - this.ch1));
                this.ch_arr[0] = (char)(this.ch_arr[0] + chch);
            }

            res /= ch_arr[0];
        } finally {
            this.ch_arr[0] = 'a'; this.ch1 = 'b'; this.ch2 = 'c';
            for (int i : arr1)
            {
                chch = (char)('f' + (this.ch2 - this.ch1));
                this.ch_arr[0] = (char)(this.ch_arr[0] + chch);
            }

            res -= ch_arr[0];
        }

        this.ch_arr[0] = 'a'; this.ch1 = 'b'; this.ch2 = 'c';
        for (int i : arr1)
        {
            chch = (char)('f' + (this.ch2 - this.ch1));
            this.ch_arr[0] = (char)(this.ch_arr[0] + chch);
        }
        
        res *= ch_arr[0];

        return (int)res;
    }
    
    final byte test5(int n) {
        byte res = 0;
        int[] arr1 = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        byte bb;
        this.b_arr[0] = 1; this.b1 = 2; this.b2 = 3;
        for (int i : arr1)
        {
            bb = (byte)(12 * (this.b2 - this.b1));
            this.b_arr[0] = (byte)(this.b_arr[0] + bb);
        }

        res += b_arr[0];

        try {
            this.b_arr[0] = 1; this.b1 = 2; this.b2 = 3;
            for (int i : arr1)
            {
                bb = (byte)(12 * (this.b2 - this.b1));
                this.b_arr[0] = (byte)(this.b_arr[0] + bb);
            }

            if (b_arr[0] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.b_arr[0] = 1; this.b1 = 2; this.b2 = 3;
            for (int i : arr1)
            {
                bb = (byte)(12 * (this.b2 - this.b1));
                this.b_arr[0] = (byte)(this.b_arr[0] + bb);
            }

            res /= b_arr[0];
        } finally {
            this.b_arr[0] = 1; this.b1 = 2; this.b2 = 3;
            for (int i : arr1)
            {
                bb = (byte)(12 * (this.b2 - this.b1));
                this.b_arr[0] = (byte)(this.b_arr[0] + bb);
            }

            res -= b_arr[0];
        }

        this.b_arr[0] = 1; this.b1 = 2; this.b2 = 3;
        for (int i : arr1)
        {
            bb = (byte)(12 * (this.b2 - this.b1));
            this.b_arr[0] = (byte)(this.b_arr[0] + bb);
        }
        
        res *= b_arr[0];

        return res;
    }
    
    final boolean test6(int n) {
        boolean res = false;
        int[] arr1 = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        boolean blbl = true;
        this.bl_arr[0] = false; this.bl1 = true; this.bl2 = true;
        for (int i : arr1)
        {
            blbl = ( true | (this.bl2 & this.bl1));
            this.bl_arr[0] = !this.bl_arr[0] ^ blbl;
        }

        res = (!bl_arr[0] ^ blbl);

        try {
            this.bl_arr[0] = false; this.bl1 = true; this.bl2 = true;
            for (int i : arr1)
            {
                blbl = ( true | (this.bl2 & this.bl1));
                this.bl_arr[0] = !this.bl_arr[0] ^ blbl;
            }

            if (res != true) throw new Exception("expected");
        } catch (Exception e)
        {
            this.bl_arr[0] = false; this.bl1 = true; this.bl2 = true;
            for (int i : arr1)
            {
                blbl = ( true | (this.bl2 & this.bl1));
                this.bl_arr[0] = !this.bl_arr[0] ^ blbl;
            }

            res = bl2 & blbl;
        } finally {
            this.bl_arr[0] = false; this.bl1 = true; this.bl2 = true;
            for (int i : arr1)
            {
                blbl = ( true | (this.bl2 & this.bl1));
                this.bl_arr[0] = !this.bl_arr[0] ^ blbl;
            }

            res = !bl_arr[0] | blbl;
        }

        this.bl_arr[0] = false; this.bl1 = true; this.bl2 = true;
        for (int i : arr1)
        {
            blbl = ( true | (this.bl2 & this.bl1));
            this.bl_arr[0] = !this.bl_arr[0] ^ blbl;
        }
        
        res = (bl_arr[0] == false);

        return res;
    }

    final int test7(int n) {
        int res = 0;
        int[] arr1 = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int ii;
        this.i_arr[iterations-1] = 1; this.i1 = 2; this.i2 = 3;
        for (int i : arr1)
        {
            ii = 123456 * (this.i1 + this.i2);
            this.i_arr[iterations-1] = ii;
        }

        res += i_arr[iterations-1];

        try {
            this.i_arr[iterations-1] = 1; this.i1 = 2; this.i2 = 3;
            for (int i : arr1)
            {
                ii = 123456 * (this.i1 + this.i2);
                this.i_arr[iterations-1] = ii;
            }

            if (i_arr[iterations-1] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.i_arr[iterations-1] = 1; this.i1 = 2; this.i2 = 3;
            for (int i : arr1)
            {
                ii = 123456 * (this.i1 + this.i2);
                this.i_arr[iterations-1] = ii;
            }

            res /= i_arr[iterations-1];
        } finally {
            this.i_arr[iterations-1] = 1; this.i1 = 2; this.i2 = 3;
            for (int i : arr1)
            {
                ii = 123456 * (this.i1 + this.i2);
                this.i_arr[iterations-1] = ii;
            }

            res -= i_arr[iterations-1];
        }

        this.i_arr[iterations-1] = 1; this.i1 = 2; this.i2 = 3;
        for (int i : arr1)
        {
            ii = 123456 * (this.i1 + this.i2);
            this.i_arr[iterations-1] = ii;
        }

        res *= i_arr[iterations-1];

        return res;
    }

    final double test8(int n) {
        double res = 0;
        int[] arr1 = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        double dd;
        this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
        for (int i : arr1)
        { 
            dd = 0.123456D * (this.d1 + this.d2);
            this.d_arr[0] = dd;
        } 

        res += d_arr[0];

        try {
            this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
            for (int i : arr1)
            { 
                dd = 0.123456D * (this.d1 + this.d2);
                this.d_arr[0] = dd;
            } 

            if (d_arr[0] > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
            for (int i : arr1)
            { 
                dd = 0.123456D * (this.d1 + this.d2);
                this.d_arr[0] = dd;
            } 

            res /= d_arr[0];
        } finally {
            this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
            for (int i : arr1)
            { 
                dd = 0.123456D * (this.d1 + this.d2);
                this.d_arr[0] = dd;
            } 

            res -= d_arr[0];
        }

        this.d_arr[0] = 1.0D; this.d1 = 2.0D; this.d2 = 3.0D;
        for (int i : arr1)
        { 
            dd = 0.123456D * (this.d1 + this.d2);
            this.d_arr[0] = dd;
        } 

        res *= d_arr[0];

        return res;
    }
    


    final int test9(int n) {
        int res = 0;
        
        int ii;
        this.i1 = 2; this.i2 = 3;
        for (int i = 1; i <= this.iterations; i++)
        {
            ii = this.i1;
        }

        res += i1;

        try {
            this.i1 = 2; this.i2 = 3;
            for (int i = 1; i <= this.iterations; i++)
            {
                ii = this.i1;
            }

            if (i2 > 0) throw new Exception("expected");
        } catch (Exception e)
        {
            this.i1 = 2; this.i2 = 3;
            for (int i = 1; i <= this.iterations; i++)
            {
                ii = this.i1;
            }

            res /= i1;
        } finally {
            this.i1 = 2; this.i2 = 3;
            for (int i = 1; i <= this.iterations; i++)
            {
                ii = this.i1;
            }

            res -= i1;
        }

        this.i1 = 2; this.i2 = 3;
        for (int i = 1; i <= this.iterations; i++)
        {
            ii = this.i1;
        }

        res *= i2;

        return res;
    }


    final void myRun() {
        System.out.println(test1(10));
        System.out.println(test2(10));
        System.out.println(test3(10));
        System.out.println(test4(10));
        System.out.println(test5(10));
        System.out.println(test6(10));
        System.out.println(test7(10));
        System.out.println(test8(10));
        System.out.println(test9(10));
    }

    public static void main(String[] args)
    {
        new Main().myRun();
    }
}

