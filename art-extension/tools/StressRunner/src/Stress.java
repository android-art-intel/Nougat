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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.lang.reflect.Method;
import java.util.ArrayList;

public class Stress extends Thread {
    public static final String METHODNAME   = "init";
    protected ArrayList<String>        toRunStrings = new ArrayList<String>();
    public Class cl;
    public BaseStressor basestressor;
    public static volatile boolean stressorStarted = false;
    public static volatile boolean stressorStopped = false;
    
    public void init() {
        File f = new File("stress.param");

        if (f != null && f.exists() && f.isFile()) {
            BufferedReader r = null;
            try {
                r = new BufferedReader(new FileReader(f));
                while (r.ready()) {
                    String tmp = r.readLine().trim();
                    if (tmp.length() > 0)
                        toRunStrings.add(tmp);
                }
            } catch (Exception e) {
                System.out
                    .println("Unexpected exception while parse input file: "
                        + e);
            } finally {
                if (r != null) {
                    try {
                        r.close();
                    } catch (Exception e) {
                    }
                }
            }
        }
        if (toRunStrings.size() == 0) {
            toRunStrings.add("SimpleStressor");
        }
    }

    public void startStressor() {
        for (int i = 0; i < toRunStrings.size(); i++) {
            try {
                String[] cmd = Util.stringToArray(toRunStrings.get(i).toString());
                if (cmd == null || cmd.length < 1)
                    return;
                cl = Class.forName(cmd[0], false, this.getClass().getClassLoader());
                Method m;
                Class[] paramTypes = new Class[1];
                paramTypes[0] = String[].class;
                m = cl.getMethod(METHODNAME, paramTypes);
                m.setAccessible(true);
                Object[] p = new String[cmd.length - 1];
                for (int pCnt = 0; pCnt < p.length; pCnt++) {
                    p[pCnt] = cmd[pCnt + 1];
                }
                Object[] p1 = new Object[1];
                p1[0] = p;
                basestressor = (BaseStressor)cl.newInstance();
                m.invoke(basestressor, p1);
            } catch (Exception e) {
                System.out.println("can not run stressor: " + e);
            }
        }
    }

    public void shutdown() {

        for (int i = 0; i < toRunStrings.size(); i++) {
            try {
                String[] cmd = Util.stringToArray(toRunStrings.get(i).toString());
                if (cmd == null || cmd.length < 1)
                    return;
                Method m;
                Class[] paramTypes = null;
                m = cl.getMethod("shutdown", paramTypes);
                m.setAccessible(true);
                Object[] p = null;
                Object[] p1 = new Object[1];
                p1[0] = p;
                m.invoke(basestressor, p);
            } catch (Exception e) {
                System.out.println("can not run shutdown: " + e);
            }
        }
        Thread.yield();
    }
}
