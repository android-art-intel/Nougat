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
import java.lang.reflect.Method;
import java.util.Map;
import java.util.Stack;
import java.util.TreeMap;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.ScriptableObject;
import org.mozilla.javascript.debug.Debugger;

public class RhinoScripter {
    static public final String D_INCLUDE = "#include";
    static public final String D_INCLUDE_ONCE = "#include_once";

    private int prologLines;
    private String prolog;
    private Object target;

    public RhinoScripter(Object obj) {
        Utils.stamp = "\n[uitest] ";
        target = obj;
        Map<String, Method> map = new TreeMap<String, Method>();
        prologLines = Utils.collectScriptableMethods(target.getClass(), map) + 1;
        prolog = Utils.makeScriptProlog("app", map, "\n");
    }

    public int prologLines() {
        return prologLines;
    }

    static public int preProcess(String scriptString, String sourceName, String baseDir, StringBuffer sb) throws Exception {
        Stack<String> included = new Stack<String>();
        Stack<String> allIncluded = new Stack<String>();
        String sourcePath = new File(baseDir, sourceName).getAbsolutePath();
        included.push(sourcePath);
        allIncluded.push(sourcePath);
        return preProcess(scriptString, sourceName, baseDir, sb, included, allIncluded);
    }

    static protected int preProcess(String scriptString, String sourceName, String baseDir, StringBuffer sb, Stack<String> included, Stack<String> allIncluded)
            throws Exception {
        scriptString = scriptString.replace("\r\n", "\n");
        String[] scriptLines = scriptString.split("\n");
        int line = 0;
        int lines = 0;
        for (String scriptLine : scriptLines) {
            line++;
            if (scriptLine.startsWith(D_INCLUDE) || scriptLine.startsWith(D_INCLUDE_ONCE)) {
                String fileName = scriptLine.substring(D_INCLUDE.length()).trim();
                if (scriptLine.startsWith(D_INCLUDE_ONCE)) {
                    fileName = scriptLine.substring(D_INCLUDE_ONCE.length()).trim();
                }
                if (fileName.startsWith("\"") && fileName.endsWith("\"")) {
                    fileName = fileName.substring(1, fileName.length() - 1);
                } else if (fileName.startsWith("'") && fileName.endsWith("'")) {
                    fileName = fileName.substring(1, fileName.length() - 1);
                }
                String absPath = new File(baseDir, fileName).getAbsolutePath();
                if (scriptLine.startsWith(D_INCLUDE_ONCE) && allIncluded.contains(absPath)) {
                    TestBase.log("Skipping " + absPath + " - once included");
                    continue;
                }
                if (included.contains(absPath)) {
                    throw new Exception("Recursive preprocessor directive: " + scriptLine + " in " + sourceName + ":" + line);
                }
                String includedScript = Utils.loadString(new File(baseDir, fileName), "\n", 12000000);
                included.push(absPath);
                allIncluded.push(absPath);
                lines += preProcess(includedScript, fileName, baseDir, sb, included, allIncluded);
                included.pop();
            } else {
                sb.append(scriptLine).append("\n");
                lines++;
            }
        }
        return lines;
    }

    public void eval(String scriptString, String sourceName, int sourceLine, Debugger debugger, boolean logSource) throws Exception {
        String text = scriptString;
        Context cx = Context.enter();
        try {
            cx.setOptimizationLevel(-1);
            if (debugger != null)
                cx.setDebugger(debugger, null);
            ScriptableObject scope = cx.initStandardObjects();
            prepareScope(scope);
            text = prolog + "\n" + text;
            String[] ss = text.split("\n");
            String text2 = "";
            int n = 0;
            for (String s : ss) {
                n++;
                text2 += Utils.space3(n) + ": " + s + "\n";
            }
            if (logSource) {
                TestBase.log("sourceLine: " + sourceLine);
                TestBase.log("prologLines: " + prologLines);
                TestBase.log("------------------------------------------------------------------");
                TestBase.log(text2);
                TestBase.log("------------------------------------------------------------------");
            }
            // Object result =
            cx.evaluateString(scope, text, sourceName, sourceLine, null);
            // System.err.println("result: " + Context.toString(result));
        } finally {
            Context.exit();
        }
    }

    protected void prepareScope(ScriptableObject scope) {
        scope.defineProperty("app", target, 0);
        scope.defineProperty("landscape", "landscape", 0);
        scope.defineProperty("portrait", "portrait", 0);
    }
}
