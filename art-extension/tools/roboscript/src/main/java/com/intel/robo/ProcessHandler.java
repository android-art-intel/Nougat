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

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.regex.Pattern;

import android.os.Environment;

public class ProcessHandler {
    static public int nextCmdIndex = 1;
    private Process process;
    private BufferedWriter copyOutput;
    public Pattern pattern;
    final public boolean isLogcat;
    final public boolean verbose;
    final public String cmdline;
    final public int commandIndex;

    public void log(String text) {
        TestBase.log(text);
    }
    public void log(String text, Throwable e) {
        TestBase.log(text, e);
    }

    public ProcessHandler(String testName, String cmdl, boolean v) throws Exception {
        this(testName, cmdl, v, null);
    }

    public ProcessHandler(String testName, String cmdl, boolean v, String pat) throws Exception {
        commandIndex = nextCmdIndex++;
        cmdline = cmdl;
        verbose = v;
        if (pat != null) {
            pattern = Pattern.compile(pat);
        }
        String cmd[] = cmdline.split(" ");
        isLogcat = cmd[0].equals("logcat");
        process = new ProcessBuilder(cmd).start();
        if (pat != null) {
            log(getCmdName() + " started with pattern matcher: " + pat);
        } else {
            log(getCmdName() + " started");
        }
        if (testName != null && testName.length() > 0) {
            try {
                File file = new File(Environment.getExternalStorageDirectory(), testName);
                file.mkdirs();
                String filename = cmd[0] + "_" + commandIndex + ".out";
                file = new File(file, filename);
                log(getCmdName() + " creating output in file: " + file.getAbsolutePath());
                FileOutputStream fos = new FileOutputStream(file);
                copyOutput = new BufferedWriter(new OutputStreamWriter(fos));
            } catch (FileNotFoundException e) {
                log(getCmdName() + " cannot save output", e);
            }
        }
    }

    public String getCmdName() {
        return (isLogcat ? "Logcat" : "Process") + " #" + commandIndex + " [" + cmdline + "]";
    }

    public boolean lineMatches(String line) {
        if (pattern != null) {
            return pattern.matcher(line).matches();
        }
        return isLogcat == false;
    }

    public String exec() throws Exception {
        StringBuffer sb = new StringBuffer();
        exec(sb);
        return sb.toString();
    }

    public void exec(final StringBuffer sb) throws Exception {
        exec(new Writer() {
            @Override
            public void close() throws IOException {
            }
            @Override
            public void flush() throws IOException {
            }
            @Override
            public void write(char[] buf, int offset, int count) throws IOException {
                throw new IOException("should not used");
            }
            @Override
            public void write(String str) throws IOException {
                synchronized (sb) {
                    sb.append(new String(str));
                    sb.append("\n");
                }
            }
        });
    }

    public void exec(Writer w) throws Exception {
        Process process = this.process;
        BufferedWriter copyOutput = this.copyOutput;
        BufferedReader bf = new BufferedReader(new InputStreamReader(process.getInputStream()));
        int skippedLines = 0;
        int skippedCharacters = 0;
        int processedLines = 0;
        int processedCharacters = 0;
        try {
            String line;
            while ((line = bf.readLine()) != null) {
                if (copyOutput != null) {
                    try {
                        copyOutput.write(line);
                        copyOutput.write("\n");
                    } catch (Exception e) {
                        try {
                            copyOutput.close();
                        } catch (Exception ignore) {
                        }
                        copyOutput = null;
                        log((isLogcat ? "Logcat" : "Process") + " [" + cmdline + "] stopped saving output due to error", e);
                    }
                }
                boolean skip = isLogcat && line.indexOf("[robolog] ") >= 0;
                if (skip == false && lineMatches(line)) {
                    processedLines++;
                    processedCharacters += line.length();
                    w.write(line);
                } else {
                    skippedLines++;
                    skippedCharacters += line.length();
                }
            }
            int exitCode = process.waitFor();
            if (verbose) {
                log(getCmdName() + " exited with code: " + exitCode + ", processed lines: " + processedLines + ", chars: " + processedCharacters
                        + " skipped lines: " + skippedLines + ", chars: " + skippedCharacters);
            }
        } catch (Exception e) {
            // check not stopped
            if (this.process != null) {
                if (verbose) {
                    log(getCmdName() + " failed: processed lines: " + processedLines + ", chars: " + processedCharacters + " skipped lines: "
                            + skippedLines + ", chars: " + skippedCharacters);
                }
                throw e;
            } else {
                if (verbose) {
                    log(getCmdName() + " stopped: processed lines: " + processedLines + ", chars: " + processedCharacters + " skipped lines: "
                            + skippedLines + ", chars: " + skippedCharacters);
                }
            }
        } finally {
            try {
                if (copyOutput != null)
                    copyOutput.close();
            } catch (Exception ignore) {
            }
        }
    }

    public void stop() {
        Process p = process;
        process = null;
        if (p != null) {
            log(getCmdName() + " stopping...");
            p.destroy();
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
            }
        }
    }
}