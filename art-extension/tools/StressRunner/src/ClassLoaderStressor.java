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

import java.util.jar.JarEntry;
import java.util.Enumeration;
import java.util.jar.JarFile;
import dalvik.system.PathClassLoader;
import dalvik.system.DexFile;

public class ClassLoaderStressor  extends BaseStressor {
    int threadCnt = processorsNmbr * 4;

    public static void main(String[] args) {
        ClassLoaderStressor cls = new ClassLoaderStressor();
        Run obj = cls.new Run(1);
        obj.start();
        try {
            Thread.sleep(200);
            obj.stopIt = 1;
            obj.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    /*
     * Note, this method called through the Method.invoke() and should return to
     * the main thread ASAP
     */
    public void run(String[] args) {

        if (args.length > 0) {
            try {
                threadCnt = Integer.parseInt(args[0]);
            } catch (Exception e) {
                // just use default value
            }
        }
        new DoNothing().start();
    }

    class DoNothing extends Thread {
        public void run() {
            Run[] threads = new Run[threadCnt];
            for (int i = 0; i < threads.length; i++) {
                threads[i] = new Run(i);
                threads[i].start();
            }
             while (!stopStressor) {
                for (int i = 1; i < threads.length; i++)
                    threads[i - 1].i = threads[i].i;
                Thread.yield();
            }
            for (int i = 0; i < threads.length; i++) {
                threads[i].stopIt = 1;
            }
        }
    }

    byte[] buff = null;
    class Run extends Thread {
        volatile int  stopIt = 0;
        volatile long i      = 0;
        public int    id     = 0;

        public Run(int id) {
            this.id = id;
        }

        public void run() {
            String inFileName = "/system/framework/com.google.android.media.effects.jar";

            try {
                while (stopIt != 1) {
                    JarFile jar = new JarFile(inFileName);
                    Enumeration<JarEntry> entries = jar.entries();
                    while (entries.hasMoreElements())
                    {
                        JarEntry entry = entries.nextElement();
                        String entryName = entry.getName();
                        if (entryName.endsWith(".dex")) {
                            DexFile df = new DexFile(inFileName);
                            Enumeration<String> classNames = df.entries();
                            while (classNames.hasMoreElements()){
                                String className = classNames.nextElement();
                                if (df.loadClass(className, ClassLoader.getSystemClassLoader()) != null) {
                                }
                            }
                        }
                    }  
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
    class SpecialClassLoader extends PathClassLoader {
        public SpecialClassLoader(String dexPath, ClassLoader parent) { super(dexPath, parent); }
        protected Class findClass(String name) throws ClassNotFoundException {
            try {
                return defineClass(buff, 0, buff.length);
            } catch (Throwable e) {
                return null;
            }
        }
    }
}
