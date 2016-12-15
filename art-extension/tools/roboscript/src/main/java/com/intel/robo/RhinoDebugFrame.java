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

import org.mozilla.javascript.Context;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.WrappedException;
import org.mozilla.javascript.debug.DebugFrame;

public class RhinoDebugFrame implements DebugFrame {

    protected RhinoDebugger debugger;

    public RhinoDebugFrame(RhinoDebugger d) {
        debugger = d;
    }

    @Override
    public void onEnter(Context cx, Scriptable activation, Scriptable thisObj, Object[] args) {
        // TestBase.log("DEBUG: onEnter: " + activation.toString());
    }

    @Override
    public void onExit(Context cx, boolean byThrow, Object resultOrException) {
        // TestBase.log("DEBUG: onExit: " + byThrow + " - " +
        // resultOrException);
    }

    @Override
    public void onLineChange(Context cx, int lineNumber) {
        if (lineNumber - debugger.skipLines > 0) {
            debugger.accumLine = lineNumber;
            debugger.actualLine = lineNumber - debugger.skipLines;
            // System.out.println("DEBUG: onLineChange: line: " +
            // debugger.actualLine + " (" + debugger.accumLine + ")");
            // debugger.processor.visualizer.selectLine(debugger.actualLine);
            // debugger.processor.updateData(false, false, false);
            if (debugger.isStopped()) {
                String msg = "DEBUG: Scenario interrupted on line: " + debugger.actualLine + " (" + debugger.accumLine + ")";
                TestBase.log(msg);
                throw new RuntimeException(msg);
            }
        } else {
            // System.out.println("DEBUG: onLineChange: prolog line: " +
            // lineNumber);
        }
    }

    @Override
    public void onExceptionThrown(Context cx, Throwable ex) {
        String lineInfo = " [line in source: " + debugger.actualLine + ", combined: " + debugger.accumLine + "]";
        if (ex instanceof WrappedException) {
            String s = ((WrappedException) ex).getWrappedException() + lineInfo;
            TestBase.log(s.replace("java.lang.RuntimeException: ", ""));
        } else {
            TestBase.log("Error: " + ex + lineInfo);
        }
    }

    @Override
    public void onDebuggerStatement(Context cx) {
    }
}