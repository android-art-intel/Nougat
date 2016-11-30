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

package OptimizationTests.LoopInformation.shared;

import java.lang.reflect.Method;

public class TestUtils {

    static public void runTests(Object test, Object... args) {
        Method[] methods = test.getClass().getDeclaredMethods();
        for (int i = 1; i < methods.length; i++) {
            int j = i;
            while (j > 0 && methods[j - 1].getName().compareTo(methods[j].getName()) > 0) {
                Method tempMethod = methods[j];
                methods[j] = methods[j - 1];
                methods[j - 1] = tempMethod;
                j--;
            }
        }
        for (Method method : methods) {
            if (method.getName().startsWith("test")) {
                try {
                    String names[] = test.getClass().getPackage().getName().split("\\.");
                    String testName = names[names.length - 1];
                    System.out.println(testName + "." + method.getName() + " result: " + method.invoke(test, args));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static int hash(float... fs) {
        return hash$noinline$(fs);
    }

    private static int hash$noinline$(float... fs) {
        int hash = 0;
        for (float f : fs) {
            //System.out.println("hash: " + f + " -> " + Float.floatToIntBits(f));
            hash += Float.floatToIntBits(f);
        }
        return hash;
    }

}
