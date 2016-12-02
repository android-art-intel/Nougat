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

package OptimizationTests.Devirtualization.InvokeVirtual_ByteTryCatch;
import java.util.*;

public class Main {

    public int div(int d) {
        return 1 / d;
    }

    public void runTestNewInstance() {
        try {
            div(0);
        } catch (Exception e) {
            ArrayList<Byte> list = new ArrayList<Byte>();
            list.add((byte)33);
        }
    }

    static public void main(String[] args) {
       Main n = new Main();
    }
}
