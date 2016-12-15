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

import java.io.File;

import org.mozilla.javascript.EvaluatorException;
import org.mozilla.javascript.ScriptableObject;
import org.mozilla.javascript.WrappedException;

public class TestScript extends TestBase {

    protected String script;
    protected String inline;
    protected String scenarioName;
    protected boolean logSource;
    protected boolean dumpOnError;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        logOperationFailed = false;
        script = getSysPropStr("script", script);
        inline = getSysPropStr("inline", inline);
        scenarioName = getSysPropStr("scenarioName", scenarioName);
        if (scenarioName == null || scenarioName.length() == 0) {
            if (script != null && script.length() > 0) {
                scenarioName = new File(script).getName();
                scenarioName = scenarioName.replace(".js", "").replace(".scn", "");
            } else {
                scenarioName = "inline";
            }
        }
        logSource = getSysPropBool("logSource", logSource);
        dumpOnError = getSysPropBool("dumpOnError", dumpOnError);
    }

    @Override
    public String getScenarioName() {
        return scenarioName;
    }

    public void testScript() {
        Throwable err = null;
        String status = "FAILED";
        boolean errorInScript = false;
        try {
            if (script != null && script.length() > 0) {
                runScript(script);
            } else if (inline != null && inline.length() > 0) {
                runString(inline, "inline", ".");
            }
            status = "FINISHED";
        } catch (WrappedException e) {
            err = e.getWrappedException();
            errorInScript = true;
        } catch (Exception e) {
            err = e;
        }
        if (err != null) {
            if (err instanceof ExitException) {
                status = "STOPPED";
            } else if (err instanceof EvaluatorException) {
                log("Evaluator error: " + err.getMessage());
            } else {
                if (errorInScript)
                    ;// log("Error: " + err.getMessage());
                else
                    log("Exception: ", err);
            }
            if (dumpOnError && !(err instanceof ExitException)) {
                dump();
            }
        }
        screenshot(getScenarioName() + "-" + status);
        log("Scenario " + getScenarioName() + ": " + status);
    }

    public void runScript(String script) throws Exception {
        if (script == null) {
            log("Invalid arg (script == null)");
            return;
        }
        if (!new File(script).exists()) {
            log("Script file does not exist: " + script);
            return;
        }
        log("Script run started: " + script);
        String scriptString = Utils.loadString(script, "\n", 12000000);
        runString(scriptString, new File(script).getName(), new File(script).getParentFile().getAbsolutePath());
    }

    public void runString(String scriptString, String sourceName, String baseDir) throws Exception {
        int scriptLines = scriptString.split("\n").length;
        StringBuffer sb = new StringBuffer();
        int lines = RhinoScripter.preProcess(scriptString, sourceName, baseDir, sb);
        lines -= scriptLines;
        log("Preprocessed lines: " + lines);
        scriptString = sb.toString();
        RhinoScripter scripter = new RhinoScripter(this) {
            @Override
            protected void prepareScope(ScriptableObject scope) {
                super.prepareScope(scope);
                scope.defineProperty("device", device, 0);
                scope.defineProperty("verbose", Utils.verbose, 0);
            }
        };
        scripter.eval(scriptString, sourceName, 1, new RhinoDebugger(scripter.prologLines() + lines), logSource);
    }
}
