/*
 * Copyright (C) 2016 Intel Corporation
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
 */

package OptimizationTests.LoopPeeling.Multithreaded_01;

import OptimizationTests.LoopPeeling.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    public final int iterations = 10000;
    public static int s_field1 = 33;
    private double i_field1 = 0.00001234500012345d;
    private int i_field2 = 6;
    public final static int s_final_field1 = 100;
    private volatile int i_volatile_field1 = 0;
    boolean i_b_field1 = false;
    boolean[] arr_b_field1 = new boolean[] {true, false, true, false};
    Cls1_ext cls1_ext_field1 = new Cls1_ext();


    public static int foo_inlined(int x) {
        return x;
    }

    /* Peeling candidate: iget + LoadClass inside loop in t2, small number of iteration */
    public double test_01(int x) {
        double sum = 0;
        final int x1 = x;


        double tmp1 = i_field1;
        double tmp2 = cls1_ext_field1.i_field3;

        Thread t1 = new Thread(new Runnable(){
                public void run(){
                 cls1_ext_field1.inc_i_field3(x1);
                //volatile
                cls1_ext_field1.i_v_field--;
                }
                });
        Thread t2 = new Thread(new Runnable(){
                public void run(){
                for (int i=0; i<10; i++){
                //peeling candidate
                i_field1= cls1_ext_field1.i_field3--;
                //volatile
                cls1_ext_field1.i_v_field++;
                }
                for (int i=0; i<10; i++){
                //peeling candidate
                i_field1= cls1_ext_field1.i_field3--;
                //volatile
                cls1_ext_field1.i_v_field++;
                }

                }
                });
        t1.start();
        t2.start();
        try{
            t1.join();
            t2.join();
        } catch(Exception e){
            System.out.println(e);
        }

        i_field1 = tmp1;
        cls1_ext_field1.i_field3 = tmp2;
        return sum + cls1_ext_field1.i_v_field;// + i_field1 + cls1_ext_field1.i_field3;
    }


    /* Peeling candidate: iget inside loop in t2, integer overflow */
    public int test_02(int x) {
        int sum = 0;
        final int x1 = x;

        i_field2 = 1; 
        cls1_ext_field1.i_field4 = 6;

        int tmp1 = i_field2;
        int tmp2 =  cls1_ext_field1.i_field4;

        cls1_ext_field1.i_v_field = -61;

        Thread t1 = new Thread(new Runnable(){
                public void run(){
                        do {
                        cls1_ext_field1.i_field4 = 5;
                        } while ( cls1_ext_field1.i_v_field != -5); // reading volatile which is modified in another thread
                    }
                });
        Thread t2 = new Thread(new Runnable(){
                public void run(){
                for (int i=0; i<100; i++){
                    for (int j=0; j<10000; j++){
                        //peeling candidate
                        i_field2 *= cls1_ext_field1.i_field4;;
                        cls1_ext_field1.i_field4 = -5;
                    }
                //volatile
                cls1_ext_field1.i_v_field *= cls1_ext_field1.i_v_field + 6;
                }

                }
                });
        t1.start();
        t2.start();
        try{
            t1.join();
            t2.join();
        } catch(Exception e){
            System.out.println(e);
        }

        i_field2 = tmp1;
        cls1_ext_field1.i_field4 = tmp2;
        return sum + cls1_ext_field1.i_v_field + cls1_ext_field1.i_field4;
 
    }


    /* Peeling candidate: iget + LoadClass inside loop in t2, int fields, overflow */
    public double test_03(int x) {
        int sum = 0;
        final int x1 = x;
        cls1_ext_field1.i_v_field = 0;

        double tmp1 = i_field1;
        double tmp2 = cls1_ext_field1.i_field3;

        Thread t1 = new Thread(new Runnable(){
                public void run(){
                 cls1_ext_field1.inc_i_field3(x1);
                //read volatile
                cls1_ext_field1.i_field3 = cls1_ext_field1.i_v_field;
                }
                });
        Thread t2 = new Thread(new Runnable(){
                public void run(){
                for (int i=0; i<1000000; i++){
                //peeling candidate
                i_field1= cls1_ext_field1.i_field3--;
                //volatile
                cls1_ext_field1.i_v_field++;
                }
                for (int i=0; i<1000000; i++){
                //peeling candidate
                i_field1= cls1_ext_field1.i_field3--;
                //volatile
                cls1_ext_field1.i_v_field++;
                }

                }
                });
        t1.start();
        t2.start();
        try{
            t1.join();
            t2.join();
        } catch(Exception e){
            System.out.println(e);
        }

        i_field1 = tmp1;
        cls1_ext_field1.i_field3 = tmp2;
        return sum + cls1_ext_field1.i_v_field;// + i_field1 + cls1_ext_field1.i_field3;
    }

    /* Peeling candidate: iget + LoadClass + Null Check inside loop in t2, aget, BoundsCheck; exception is thrown  */
    /* expecting 1 peeling in t1, 2 peelings in t2 */
    public double test_04(int x) {
        int sum = 0;
        final int x1 = x;
        cls1_ext_field1.i_v_field = 0;

        double tmp1 = i_field1;
        double tmp2 = cls1_ext_field1.i_field3;


        Thread t1 = new Thread(new Runnable(){
                public void run(){
                for (int i = 0; i< cls1_ext_field1.arr_field1.length; i++) {
                    //peeling candidate
                    cls1_ext_field1.arr_field1[i] = x1;
                    // we use volatile field as a flag
                    cls1_ext_field1.i_v_field = -5*x1;
                }
                //read volatile
  //              cls1_ext_field1.i_field3 = cls1_ext_field1.i_v_field;
                }
                });
        Thread t2 = new Thread(new Runnable(){
                public void run(){
                // waiting for t1 to start initializing this array - no peeling because of volatile field
                while (true) {
                    if (cls1_ext_field1.i_v_field == -5*x1) {
                        break;
                    }
                }
                for (int i=0; i<1000000; i++){
                //peeling candidate: iget, aget, BoundsCheck throws on 100.000th iteration, 
                cls1_ext_field1.i_v_field = cls1_ext_field1.arr_field1[i/10000];
                //volatile
//                cls1_ext_field1.i_v_field++;
                }
                }
                });
        Thread.UncaughtExceptionHandler h = new Thread.UncaughtExceptionHandler() {
                public void uncaughtException(Thread th, Throwable ex) {
                            System.out.println("Expected uncaught exception: " + ex);
                                }
        };
        t2.setUncaughtExceptionHandler(h);
        t1.start();
        t2.start();
        try{
            t1.join();
            t2.join();
        } catch(Exception e){
            System.out.println(e);
        }

        i_field1 = tmp1;
        cls1_ext_field1.i_field3 = tmp2;
        return sum + cls1_ext_field1.i_v_field;// + i_field1 + cls1_ext_field1.i_field3;
    }



    public void runTests() {
        Class c = new Main().getClass();
        Method[] methods = c.getDeclaredMethods();
        Method tempMethod;
        for (int i = 1; i < methods.length; i++) {
            int j = i;
            while (j > 0 &&  methods[j-1].getName().compareTo(methods[j].getName()) > 0) {
                tempMethod = methods[j];
                methods[j] = methods[j-1];
                methods[j-1] = tempMethod;
                j = j - 1;
            }
        }

        Object[] arr = {null};
        for (Method m: methods){
            if (m.getName().startsWith("test")){
                String names[] = c.getPackage().getName().split("\\.");
                String testName = names[names.length-1];
                try {
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: "+ m.invoke(this, 10));
                } catch (IllegalArgumentException e) {
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: " + e);
                    //e.printStackTrace();
                } catch (IllegalAccessException e) {
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: " + e);
                    //e.printStackTrace();
                } catch (InvocationTargetException e) {
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: " + e + ", caused by: " + e.getCause());
                    //e.printStackTrace();
                }
            }
        }


    }

    public static void main(String[] args)
    {
        new Main().runTests();
    }

}


