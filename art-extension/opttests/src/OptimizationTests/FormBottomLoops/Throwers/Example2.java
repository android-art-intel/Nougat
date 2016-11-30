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

package OptimizationTests.FormBottomLoops.Throwers;

public class Example2 {
    private static String ID = null;
    private static String substr = null;
    private static int count = 0;
    public Example2 (String str) {
        Example2.ID = str.substring(0, str.length() - 1);
        

    }
    static {
        substr = ID.substring(0, ID.length() - 1 + Example2.count);
    }

    public String getSubstring() {
        Example2.count += 10;
        return substr;
    }
}


