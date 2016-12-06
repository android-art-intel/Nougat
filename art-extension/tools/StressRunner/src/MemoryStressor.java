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

public class MemoryStressor  extends BaseStressor {
    int threadCnt = processorsNmbr * 4;
    int chainLength = 1000;

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
                chainLength = Integer.parseInt(args[1]);
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
            try {
                while (stopIt != 1) {
                    ChainElement root = new ChainElement();
                    root.prev = null;
                    root.next = new ChainElement();
                    ChainElement current = root;
                    for (int i = 0; i < chainLength; i++) {
                        ChainElement newElem = new ChainElement();
                        current.next = newElem;
                        newElem.prev = current;
                        current = newElem;
                    }
                    root = null;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        class ChainElement {
            Object prev;
            Object next;
        }
    }
}
