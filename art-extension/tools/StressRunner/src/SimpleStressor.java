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

public class SimpleStressor  extends BaseStressor {
    int threadCnt = processorsNmbr * 4;

    /*
     * Note, this method called through the Method.invoke() and should return to
     * the main thread ASAP
     */
    public void init(String[] args) {
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
        boolean threadsIsAlive=true;
        public void run() {
            Run[] threads = new Run[threadCnt];
            for (int i = 0; i < threads.length; i++) {
                threads[i] = new Run("StressThread_"+i);
                threads[i].start();
            }
            Stress.stressorStarted=true;
            while (!stopStressor) { 
                for (int i = 1; i < threads.length; i++)
                    threads[i - 1].i = threads[i].i;
            }
            for (int i = 0; i < threads.length; i++) {
                threads[i].cancel();
            }

            while (threadsIsAlive) {
                threadsIsAlive=false;
                for (int i = 0; i < threads.length; i++) {
                    threadsIsAlive |= threads[i].isAlive();
                }
            }
           Stress.stressorStopped=true;
        }
    }

    class Run extends Thread {
        volatile long i      = 0;
        public int    id     = 0;

        public Run(String thread_name) {
            super(thread_name);
        }


        public void run() {

            try {
                while (!Thread.currentThread().isInterrupted()) {
                    i++;
                }
                join();
            } catch (InterruptedException ie) {
               /* Allow thread to exit */
            }
            
        }

        public void cancel() { interrupt(); }
    }
}
