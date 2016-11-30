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

package OptimizationTests.OSR.FormBottomLoops_DevTest_01;

public class Main {
    private final static int LIMIT = 200*1200;

    public static void main(String[] args) {
        new Main().run();
    }

    private void run() {
        System.out.println("testZZZ2NodesSumB: " + testZZZ2NodesSumB());
        System.out.println("testZZZ2NodesSumH: " + testZZZ2NodesSumH());
        System.out.println("testZZZ2NodesSetB: " + testZZZ2NodesSetB());
        System.out.println("testZZZ2NodesSetH: " + testZZZ2NodesSetH());

        System.out.println("testZZZ2NodesComplexSumB: " + testZZZ2NodesComplexSumB());
        System.out.println("testZZZ2NodesComplexSumH: " + testZZZ2NodesComplexSumH());
        System.out.println("testZZZ2NodesComplexSetB: " + testZZZ2NodesComplexSetB());
        System.out.println("testZZZ2NodesComplexSetH: " + testZZZ2NodesComplexSetH());

        System.out.println("testZZZ3NodesSumBBDirect: " + testZZZ3NodesSumBBDirect());
        System.out.println("testZZZ3NodesSumBBReverse: " + testZZZ3NodesSumBBReverse());
        System.out.println("testZZZ3NodesSumHHDirect: " + testZZZ3NodesSumHHDirect());
        System.out.println("testZZZ3NodesSumHHReverse: " + testZZZ3NodesSumHHReverse());
        System.out.println("testZZZ3NodesSumHBDirect: " + testZZZ3NodesSumHBDirect());
        System.out.println("testZZZ3NodesSumHBReverse: " + testZZZ3NodesSumHBReverse());
        System.out.println("testZZZ3NodesSetBBDirect: " + testZZZ3NodesSetBBDirect());
        System.out.println("testZZZ3NodesSetBBReverse: " + testZZZ3NodesSetBBReverse());
        System.out.println("testZZZ3NodesSetHHDirect: " + testZZZ3NodesSetHHDirect());
        System.out.println("testZZZ3NodesSetHHReverse: " + testZZZ3NodesSetHHReverse());
        System.out.println("testZZZ3NodesSetHBDirect: " + testZZZ3NodesSetHBDirect());
        System.out.println("testZZZ3NodesSetHBReverse: " + testZZZ3NodesSetHBReverse());
    }

 
    private int testZZZ2NodesSumB() {
        int a = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            a += i + 6;
        }

        return a + i;
    }

    private int testZZZ2NodesSumH() {
        int a = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a += i + 6;
            if (i >= LIMIT) break;
        }

        return a + i;
    }

    private int testZZZ2NodesSetB() {
        int a = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            a = i;
        }

        return a + i;
    }

    private int testZZZ2NodesSetH() {
        int a = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a = i;
            if (i >= LIMIT) break;
        }

        return a + i;
    }

    private int testZZZ2NodesComplexSumB() {
        int a = 0;
        int i = 0;
        int res = 0;

        while (true) {
            res += a;
            a += i + 6;
            i = i + 1;
            if (i >= LIMIT) break;
            a += i + 6;
            res += a;
        }

        return a + i + res;
    }

    private int testZZZ2NodesComplexSumH() {
        int a = 0;
        int i = 0;
        int res = 0;

        while (true) {
            res += a;
            a += i + 6;
            i = i + 1;
            a += i + 6;
            if (i >= LIMIT) break;
            res += a;
        }

        return a + i + res;
    }

    private int testZZZ2NodesComplexSetB() {
        int a = 0;
        int i = 0;
        int res = 0;

        while (true) {
            res += a;
            i = i + 1;
            if (i >= LIMIT) break;
            a = i;
            res += a;
        }

        return a + i + res;
    }

    private int testZZZ2NodesComplexSetH() {
        int a = 0;
        int i = 0;
        int res = 0;

        while (true) {
            res += a;
            i = i + 1;
            a = i;
            if (i >= LIMIT) break;
            res += a;
        }

        return a + i + res;
    }

    private int testZZZ3NodesSumBBDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            a += i + 6;
            b += a + a;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSumBBReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            b += a + a;
            a += i + 6;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSumHHDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a += i + 6;
            b += a + a;
            if (i >= LIMIT) break;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSumHHReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b += a + a;
            a += i + 6;
            if (i >= LIMIT) break;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSumHBDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a += i + 6;
            if (i >= LIMIT) break;
            b += a + a;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSumHBReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b += a + a;
            if (i >= LIMIT) break;
            a += i + 6;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSetBBDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            a += i + 6;
            b = a;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSetBBReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            if (i >= LIMIT) break;
            b = a;
            a += i + 6;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSetHHDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a += i + 6;
            b = a;
            if (i >= LIMIT) break;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSetHHReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b = a;
            a += i + 6;
            if (i >= LIMIT) break;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSetHBDirect() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            a += i + 6;
            if (i >= LIMIT) break;
            b = a;
        }

        return a + b + i;
    }

    private int testZZZ3NodesSetHBReverse() {
        int a = 0;
        int b = 0;
        int i = 0;

        while (true) {
            i = i + 1;
            b = a;
            if (i >= LIMIT) break;
            a += i + 6;
        }

        return a + b + i;
    }
}
