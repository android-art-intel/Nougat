/*
 * Copyright (C) 2015 Intel Corporation.
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
 * 
 */

package com.intel.robo;

import java.util.Locale;

import com.android.uiautomator.core.UiObject;

public class TestDump extends TestBase {

    public String dump = "tree";

    @Override
    public String getScenarioName() {
        return "dump";
    }

    @Override
    protected void setUp() throws Exception {
    	dumpParameters = false;
        super.setUp();
        dump = getSysPropStr("dump", dump);
    }

    public void testDump() {
    	log("DUMP OPTION: " + dump);
    	String dump = "," + this.dump + ",";
    	dump = dump.toLowerCase(Locale.getDefault()).replace(" ", "");
        if (dump.indexOf(",tree,") >= 0) {
            dump();
        }
        if (dump.indexOf(",instance,") >= 0) {
        	dumpByInstance();
        }
        if (this.dump.startsWith("text:")) {
            String elem = this.dump.substring(5).replace("%20", " ");
            log("ELEM: " + elem);
            try {
                UiObject uiObject = searchViewMatches(elem);
                String text = uiObject.getText();
                log("ELEM TEXT: ", text);
            } catch (Exception e) {
                fail("Elem not found: " + elem);
            }
        }
    }
}
