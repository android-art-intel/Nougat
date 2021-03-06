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

package OptimizationTests.ShortMethodsInliningNonVirtualInvokes.InvokeDirectIObject_002;
class Main {
    final static int iterations = 10;
    public static Foo tmpThingy;
    
    public static void main(String[] args) {
        Test test = new Test();
        Foo nextThingy = new Foo('F');

        System.out.println("Initial nextThingy value is " + nextThingy.getClass().toString());
        System.out.println("Initial nextThingy.a is " + nextThingy.a);

        for(int i = 0; i < iterations; i++) {
            nextThingy = test.gimme();
            tmpThingy = nextThingy.toFoo(); 
            test.hereyouare(tmpThingy);
        }

        System.out.println("Initial nextThingy value is " + nextThingy.getClass().toString());
        System.out.println("Initial nextThingy.a is " + nextThingy.a);

    }
}
