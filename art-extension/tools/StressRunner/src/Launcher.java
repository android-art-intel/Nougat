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

import android.util.Log;

public class Launcher {
	String	testName;
	String	entryName;
	int 	testIterations = 1;
	String	stressMode;
	Stress stress = null;
	
	private static final String tag = "=== Launcher"; 
	private static final String info = " Test iteration";

	public static void main(String[] args){
		try {
			new Launcher().launch(args);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}		
	}

	public int launch(String[] args) throws InterruptedException{
		if (parseArgs(args) > 0) {
			System.out.println("Invalid arguments");
			return 1;
		}

		stress = new Stress();
		if (stress != null) {
			stress.init();
			stress.startStressor();
                        while (!Stress.stressorStarted) {
                               Thread.sleep(1);
                        }
		}
		// launch the test 
		try {
			Class cl = Class.forName(testName);  
			Method m;
			Class[] paramTypes = new Class[1];
                        Object[] param;
                        param = new String[0];
                        ArrayList<String> argsString = new ArrayList<String>();
			paramTypes[0] = String[].class;
			m = cl.getMethod(entryName, paramTypes);
			int shift = (args.length >= 3) ? (3) : (2);
			// the arg[2] (the 3rd parameter) is always the number of iterations
		        // other arguments if any are the arguments of the stressed class

			if ( (args.length - shift) > 0 ) {
				param = new String[args.length - shift];
				for (int i=0; i<param.length; i++) {
					param[i] = args[i+shift];
				//	System.out.println("param[" + i + "] = " + param[i]);
				}
			}
                // arguments of the stressed class could be set in the file arguments.list
                // arguments of the Launcher override arguments.list
                        else {
                        	File f = new File("arguments.list");
			        if (f != null && f.exists() && f.isFile()) {
					BufferedReader r = null;
            				try {
                				r = new BufferedReader(new FileReader(f));
                				while (r.ready()) {
                    					String tmp = r.readLine().trim();
                    					if (tmp.length() > 0)
                        					argsString.add(tmp);
                				}
            				} catch (Exception e) {
                				System.out.println("Unexpected exception while parse input file: " + e);
            				} finally {
                				if (r != null) {
                    					try {
                        					r.close();
                    					} catch (Exception e) {
                    					}
                				}
            				}
					param = new String[argsString.size()];
				        argsString.toArray(param);
        			}
                        }
			Object[] p1 = new Object[1];
			p1[0] = param;

			for (int i=0; i<testIterations; i++) {
				m.invoke(cl.newInstance(), p1);
				//              Thread.sleep(100);
			}
		} catch (Exception e) {
			System.out.println("Exception " + e);
			System.out.println(e.getStackTrace());
			System.out.println("Cause of exception " + e.getCause());
		}
		if (stress != null) {
			stress.shutdown();
                        while (!Stress.stressorStopped) {
                               Thread.sleep(1);
                        }
		}
		return 0;
	}

	public int parseArgs(String[] args){
		if (args.length < 2){
			System.out.println("Launcher usage: \n ");
			System.out.println("java -Xms2g -Xmx2g -cp <path to stressor classes>:<test class path> Launcher <Test class name> <Test class entry name> [<iterations>] [<test class arguments>]\n ");
			return 1;
		}

		testName = args[0];
		entryName = args[1];

		if (args.length > 2){
			try {
				testIterations = Integer.parseInt(args[2]);				
				//System.out.println("testIterations " + testIterations);
			} catch (Exception e) {
				System.out.println("exception " + e);
			}
		}
		// the arg[2] (the 3rd parameter) is always the number of iterations
		// other arguments if any are the arguments of the stressed class

		return 0;
	}
}
