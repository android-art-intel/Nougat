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

package OptimizationTests.ClassHierarchyAnalysis.Test_13;
/* The test checks that when the class has more then 30 children
 * we have 'CHA #target = 0 '
 ***
 * from the document:
 * If any class has more than 30 children, CHA will stop the query to report zero (.not analyzed.) as the result.
 ***
 */

class Main {
    public static void main(String[] args) {
        A1 a1 = new A1();
        A2 a2 = new A2();
        A3 a3 = new A3();
        A4 a4 = new A4();
        A5 a5 = new A5();
        A6 a6 = new A6();
        A7 a7 = new A7();
        A8 a8 = new A8();
        A9 a9 = new A9();
        A10 a10 = new A10();
        A11 a11 = new A11();
        A12 a12 = new A12();
        A13 a13 = new A13();
        A14 a14 = new A14();
        A15 a15 = new A15();
        A16 a16 = new A16();
        A17 a17 = new A17();
        A18 a18 = new A18();
        A19 a19 = new A19();
        A20 a20 = new A20();
        A21 a21 = new A21();
        A22 a22 = new A22();
        A23 a23 = new A23();
        A24 a24 = new A24();
        A25 a25 = new A25();
        A26 a26 = new A26();
        A27 a27 = new A27();
        A28 a28 = new A28();
        A29 a29 = new A29();
        A30 a30 = new A30();
        A31 a31 = new A31();

        System.out.println("a1.test() = " + a1.test());
        System.out.println("a2.test() = " + a2.test());
        System.out.println("a3.test() = " + a3.test());
        System.out.println("a4.test() = " + a4.test());
        System.out.println("a5.test() = " + a5.test());
        System.out.println("a6.test() = " + a6.test());
        System.out.println("a7.test() = " + a7.test());
        System.out.println("a8.test() = " + a8.test());
        System.out.println("a9.test() = " + a9.test());
        System.out.println("a10.test() = " + a10.test());
        System.out.println("a11.test() = " + a11.test());
        System.out.println("a12.test() = " + a12.test());
        System.out.println("a13.test() = " + a13.test());
        System.out.println("a14.test() = " + a14.test());
        System.out.println("a15.test() = " + a15.test());
        System.out.println("a16.test() = " + a16.test());
        System.out.println("a17.test() = " + a17.test());
        System.out.println("a18.test() = " + a18.test());
        System.out.println("a19.test() = " + a19.test());
        System.out.println("a20.test() = " + a20.test());
        System.out.println("a21.test() = " + a21.test());
        System.out.println("a22.test() = " + a22.test());
        System.out.println("a23.test() = " + a23.test());
        System.out.println("a24.test() = " + a24.test());
        System.out.println("a25.test() = " + a25.test());
        System.out.println("a26.test() = " + a26.test());
        System.out.println("a27.test() = " + a27.test());
        System.out.println("a28.test() = " + a28.test());
        System.out.println("a29.test() = " + a29.test());
        System.out.println("a30.test() = " + a30.test());
        System.out.println("a31.test() = " + a31.test());
  }
}
