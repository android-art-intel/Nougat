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

package OptimizationTests.ClassHierarchyAnalysis.Test_05;
/* The test checks that for more then 2 implementations of the method m()
 * we still have 'CHA #target = 2 '
 ***
 * from the document:
 * If a method has two or more implementations, CHA will stop the query and report two implementations. 
 * Since the devirtualization pass only optimizes when there is one implementation for a virtual call, there is no harm for CHA to report two when there are more than two implementations.
 ***
 */

class Main {
    public static void main(String[] args) {
        B b = new B();
        C c = new C();
        D d = new D();
        System.out.println("b.test() = " + b.test());
        System.out.println("c.test() = " + c.test());
        System.out.println("d.test() = " + d.test());
    }
}
