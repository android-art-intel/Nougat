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
import org.mozilla.javascript.debug.DebugFrame;
import org.mozilla.javascript.debug.DebuggableScript;
import org.mozilla.javascript.debug.Debugger;

public class RhinoDebugger implements Debugger {
    protected int accumLine;
    protected int actualLine;
    protected int skipLines;

    public RhinoDebugger(int skip) {
        skipLines = skip;
    }

    @Override
    public void handleCompilationDone(Context cx, DebuggableScript fnOrScript, String source) {
        // System.out.println("DEBUG: handleCompilationDone: " );
    }

    @Override
    public DebugFrame getFrame(Context cx, DebuggableScript fnOrScript) {
        // System.out.println("DEBUG: getFrame: ");
        return new RhinoDebugFrame(this);
    }

    public boolean isStopped() {
        return false;
    }
}