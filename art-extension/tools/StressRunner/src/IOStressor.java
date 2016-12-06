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

import java.io.FileWriter;
import java.util.Date;

public class IOStressor  extends BaseStressor {
    int threadCnt = processorsNmbr * 4;
    boolean append = false;

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
        if (args.length > 1) {
            try {
                append = Boolean.parseBoolean(args[1]);
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
            }
            for (int i = 0; i < threads.length; i++) {
                threads[i].stopIt = 1;
            }
        }
    }

    class Run extends Thread {
        volatile int  stopIt = 0;
        volatile long i      = 0;
        public int    id     = 0;

        public Run(int id) {
            this.id = id;
        }

        public void run() {
            String fileName = "IOStressor.tmp." + id;
            try {
                while (stopIt != 1) {
                    FileWriter fw = new FileWriter(fileName, append);
                    fw.append(fileName + " was updated at "
                        + new Date().toString() + "\n");
                    fw.flush();
                    fw.close();
                }
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }
}
