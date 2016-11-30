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

package OptimizationTests.OSR.FormBottomLoops_DevTest_02;

public class Main {
    private final static int LIMIT = 200*1200;

    public static void main(String[] args) {
        new Main().run();
    }

    private void run() {
        System.out.println("testZZZ3NodesComplexSumBBDirect: " + testZZZ3NodesComplexSumBBDirect());
        System.out.println("testZZZ3NodesComplexSumBBReverse: " + testZZZ3NodesComplexSumBBReverse());
        System.out.println("testZZZ3NodesComplexSumHHDirect: " + testZZZ3NodesComplexSumHHDirect());
        System.out.println("testZZZ3NodesComplexSumHHReverse: " + testZZZ3NodesComplexSumHHReverse());
        System.out.println("testZZZ3NodesComplexSumHBDirect: " + testZZZ3NodesComplexSumHBDirect());
        System.out.println("testZZZ3NodesComplexSumHBReverse: " + testZZZ3NodesComplexSumHBReverse());
        System.out.println("testZZZ3NodesComplexSetBBDirect: " + testZZZ3NodesComplexSetBBDirect());
        System.out.println("testZZZ3NodesComplexSetBBReverse: " + testZZZ3NodesComplexSetBBReverse());
        System.out.println("testZZZ3NodesComplexSetHHDirect: " + testZZZ3NodesComplexSetHHDirect());
        System.out.println("testZZZ3NodesComplexSetHHReverse: " + testZZZ3NodesComplexSetHHReverse());
        System.out.println("testZZZ3NodesComplexSetHBDirect: " + testZZZ3NodesComplexSetHBDirect());
        System.out.println("testZZZ3NodesComplexSetHBReverse: " + testZZZ3NodesComplexSetHBReverse());

        System.out.println("testZZZIntricateReversePhis2H: " + testZZZIntricateReversePhis2H());
        System.out.println("testZZZIntricateReversePhis2B: " + testZZZIntricateReversePhis2B());
        System.out.println("testZZZIntricateReversePhis3H: " + testZZZIntricateReversePhis3H());
        System.out.println("testZZZIntricateReversePhis3B: " + testZZZIntricateReversePhis3B());

        System.out.println("testZZZIntricateReversePhis2ComplexH: " + testZZZIntricateReversePhis2ComplexH());
        System.out.println("testZZZIntricateReversePhis2ComplexB: " + testZZZIntricateReversePhis2ComplexB());
        System.out.println("testZZZIntricateReversePhis3ComplexH: " + testZZZIntricateReversePhis3ComplexH());
        System.out.println("testZZZIntricateReversePhis3ComplexB: " + testZZZIntricateReversePhis3ComplexB());

        System.out.println("testZZZIntricateDirectPhis3H: " + testZZZIntricateDirectPhis3H());
        System.out.println("testZZZIntricateDirectPhis3B: " + testZZZIntricateDirectPhis3B());

        System.out.println("testZZZIntricateDirectPhis3ComplexH: " + testZZZIntricateDirectPhis3ComplexH());
        System.out.println("testZZZIntricateDirectPhis3ComplexB: " + testZZZIntricateDirectPhis3ComplexB());
        System.out.println("testZZZSuper: " + testZZZSuper());
    }

    private int testZZZIntricateReversePhis2H() {
        int i = 0;
        int a = 0;

        while (true) {
            a = i++;
            if (i >= LIMIT) break;
        }

        return a + i;
    }

    private int testZZZIntricateReversePhis2B() {
        int i = 0;
        int a = 0;

        while (true) {
            if (i >= LIMIT) break;
            a = i++;
        }

        return a + i;
    }

    private int testZZZIntricateReversePhis3H() {
        int i = 0;
        int a = 0;
        int b = 0;

        while (true) {
            b = a;
            a = i++;
            if (i >= LIMIT) break;
        }

        return a + b + i;
    }

    private int testZZZIntricateReversePhis3B() {
        int i = 0;
        int a = 0;
        int b = 0;

        while (true) {
            if (i >= LIMIT) break;
            b = a;
            a = i++;
        }

        return a + b + i;
    }

    private int testZZZIntricateReversePhis2ComplexH() {
        int i = 0;
        int a = 0;
        int res = 0;

        while (true) {
            res += a;
            a = i++;
            if (i >= LIMIT) break;
            res += a;
        }

        return a + i + res;
    }

    private int testZZZIntricateReversePhis2ComplexB() {
        int i = 0;
        int a = 0;
        int res = 0;

        while (true) {
            res += a;
            if (i >= LIMIT) break;
            a = i++;
            res += a;
        }

        return a + i + res;
    }

    private int testZZZIntricateReversePhis3ComplexH() {
        int i = 0;
        int a = 0;
        int b = 0;
        int res = 0;

        while (true) {
            res += a;
            b = a;
            a = i++;
            if (i >= LIMIT) break;
            res += a;
        }

        return a + b + i + res;
    }

    private int testZZZIntricateReversePhis3ComplexB() {
        int i = 0;
        int a = 0;
        int b = 0;
        int res = 0;

        while (true) {
            res += a;
            if (i >= LIMIT) break;
            b = a;
            a = i++;
            res += a;
        }

        return a + b + i + res;
    }

    private int testZZZIntricateDirectPhis3H() {
        int a = 0;
        int b = 0;
        int res = 0;

        for (int i = 0; i < 1100; ++i) {
            for (int j = 0; j < LIMIT; ++j) {
                a = (b++);
            }
        }

        return a + b + res;
    }

    private int testZZZIntricateDirectPhis3B() {
        int a = 0;
        int b = 0;
        int res = 0;

        for (int i = 0; i < LIMIT; ++i) {
            for (int j = 0; j < LIMIT; ++j) {
                a = (++b);
            }
        }

        return a + b + res;
    }

    private int testZZZIntricateDirectPhis3ComplexH() {
        int a = 0;
        int b = 0;
        int res = 0;

        for (int i = 0; i < 1100; ++i) {
            for (int j = 0; j < LIMIT; ++j) {
                res += a + b;
                a = (b++);
                res += a + b;
            }
        }

        return a + b + res;
    }

    private int testZZZIntricateDirectPhis3ComplexB() {
        int a = 0;
        int b = 0;
        int res = 0;

        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < LIMIT; ++j) {
                res += a + b;
                a = (++b);
                res += a + b;
            }
        }

        return a + b + res;
    }




    private int testZZZ3NodesComplexSumBBDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            a += i + 6;
            b += a + a;
            a += i + 6;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSumBBReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            b += a + a;
            a += i + 6;
            b += a + a;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSumHHDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a += i + 6;
            b += a + a;
            if (i >= LIMIT) break;
            a += i + 6;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSumHHReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b += a + a;
            a += i + 6;
            if (i >= LIMIT) break;
            b += a + a;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSumHBDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a += i + 6;
            if (i >= LIMIT) break;
            b += a + a;
            a += i + 6;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSumHBReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b += a + a;
            if (i >= LIMIT) break;
            a += i + 6;
            b += a + a;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSetBBDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            a += i + 6;
            b = a;
            a += i + 6;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSetBBReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            b = a;
            a += i + 6;
            b = a;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSetHHDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a += i + 6;
            b = a;
            if (i >= LIMIT) break;
            a += i + 6;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSetHHReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b = a;
            a += i + 6;
            if (i >= LIMIT) break;
            b = a;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSetHBDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a += i + 6;
            if (i >= LIMIT) break;
            b = a;
            a += i + 6;
        }

        return a + b + i;
    }

    private int testZZZ3NodesComplexSetHBReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b = a;
            if (i >= LIMIT) break;
            a += i + 6;
            b = a;
        }

        return a + b + i;
    }

    private int testZZZSuper() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b = a;
            if (i >= LIMIT) break;
            a += i + 6;
            b = a;
        }

        while (true) {
            i = i + 1;
            b = a;
            if (i >= 2 * LIMIT) break;
            a += i + 6;
            b = a;
        }

        return a + b + i;
    }
}
