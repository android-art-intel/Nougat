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

package OptimizationTests.LoopPeeling.LoadString_01;

import OptimizationTests.LoopPeeling.shared.*;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class Main {
    public final int iterations = 10000;
    public static int s_field1 = 33;
    private double i_field1 = 0.00001234500012345d;
    public final static int s_final_field1 = 100;
    private volatile int i_volatile_field1 = 0;


    public static int foo_inlined(int x) {
        return x;
    }

    public static String foo_inlined_string(int x) {
        String str = "string";
        return str.concat(Integer.toString(x));
    }

    /* Peeling candidate: LoadString, there is 1 invoke, all conditions met, simple for loop */
    public String test_01(int x) {
        String sum = "";
        String str = "";
        for (int i = 0; i < iterations; i++) {
            str = new String("1");
            sum = str;
        }

        return sum;
    }

 /* Peeling candidate: 2 LoadString, 2 NullChecks, there are 2 invokes, all conditions met, simple while loop */
    public String test_01a(int x) {
        String sum = "";
        int i = 0;
        while (i < iterations) {
            sum = sum.concat("1");
            i++;
            sum = sum.concat("a");
        }

        return sum;
    }


 /* Peeling candidate: 1 LoadClass, 1 LoadString, 1 StaticFieldGet, 1 NullCheck, 1 InvokeStaticOrDirect, all conditions met, simple do-while loop */
    public String test_01b(int x) {
        int i = 1;
        do {
            Cls1.str_field.concat("1b");
            i++;
        } while (i < iterations);

        return Cls1.str_field;
    }




    /* Peeling candidates: 2 LoadString, 2 invokes, inlined function call, loop starts at -1, increment !=1, incrementing using multiplication */
    public String test_02(int x) {
        String sum = "";
        for (int i = -1; i < iterations; i *= -6) {
            sum = foo_inlined_string(x);

        }
        return sum;
    }


    /* Peeling candidates: 2 LoadString, 2 invokes, inlined function call, loop starts at -1, increment !=1, incrementing using multiplication */
    public String test_03(int x) {
        String sum = "";
        String str = "" + x;
        String str1 = "str1";
        String str2 = "str2";
        String str3 = "str3";
        int  i = 0;
        while(true) {
            
            sum.concat(str);
            str = "0";
            String tmpStr = "2";
            sum.concat(tmpStr);
            str = "a";
            i++;
            str1 = "b";
            str2 = "b";

            if (i>10) {
                break;
            }
            str3 = "d";

        }
        return sum + str + str1 + str2 + str3;
    }


    /* 4 loops in a row */
    public String test_05(int x) {
        String sum = "";
        String str = "" + x;
        String str1 = "str1";
        String str2 = "str2";
        String str3 = "str3";
        String str4 = "str4";
        
        for (int i = 1; i < iterations; i *= 2) {
            Cls1.str_field.concat("05");
            String tmpStr = "tmpStr";
            str = tmpStr;
        }

        for (int i = 1; i < iterations; i += 21) {
            str.concat(Cls1.str_field);
            String tmpStr = "another string";
            str1.concat("tmpStr");
            tmpStr = "one more string";
            sum = tmpStr;

        }

        for (int i = iterations; i > 1; i /= 2) {
            sum.concat("a");
            String tmpStr = "A";
            sum.concat(tmpStr);
            tmpStr = sum;
            str4 = str1;
            str1 = "B";
        }

        str1 = str4;

        for (int i = iterations; i > 1; i -= iterations/3  ) {
            str2 = "STR2";
            str3 = Cls1.str_field;
            sum.concat(str3);
            str3 = "STR3";
            }


        return sum + str + str1 + str2 + str3 + str4;
    }

    /*  float-point iterator, nested loop */
    public String test_06(int x) {
        String sum = "";
        Cls1_ext[] obj_array = {new Cls1_ext(), new Cls1_ext()};
        int j = 0;
        while (true) {
            j++;
            obj_array[0].str_field = "a";
            obj_array[1].str_field = "b";
            for (float i = 0.0f; i < 100; i += 1.33f) {
                obj_array[0] = new Cls1_ext();
                obj_array[0].str_field = "A";
                obj_array[1].str_field = "B";
                sum = (sum.concat(obj_array[0].str_field)).concat(obj_array[1].str_field);
            }

            obj_array[0].str_field = "0";
            obj_array[1].str_field = "1";
            if (j > 5) {
                break;
            }
        }

        return sum;
    }

    /*  float-point iterator, 2 nested loops, inlined func call in loop header, sput, sget, class loading, aput, aget, sget/sput before and after loop, if-else, continue */
    public String test_07(int x) {
        String sum = "";
        Cls1_ext[] obj_array = {new Cls1_ext(), new Cls1_ext()};
        obj_array[0].str_field = "0";
        obj_array[1].str_field = "1";
        while (true) {
            for (float i = iterations; i > foo_inlined(iterations); i -= 1.33f) {
                obj_array[1].str_field = "-1";
                String tmpString = "2";
                sum.concat(obj_array[0].str_field);
                if (i > 10) {
                    obj_array[0].s_field1 += 2;
                }
                obj_array[0].str_field = tmpString;
            }
            obj_array[1].s_field1++;
            obj_array[0].str_field.concat("a");
            obj_array[1].str_field.concat("b");

            sum = sum + obj_array[0].str_field +  obj_array[1].str_field + "c";

            for (float i = iterations; i > foo_inlined(iterations); i -= 1.33f) {
                obj_array[1].str_field = "-1";
                String tmpString = "2";
                sum.concat(obj_array[0].str_field);
                if (i > 10) {
                    obj_array[0].s_field1 += 2;
                    continue;
                } 
                obj_array[0].str_field = tmpString;
            }
            if ( obj_array[0].s_field1 > 100) {
                break;
            }
        }
        return sum + obj_array[0].str_field + obj_array[1].str_field + obj_array[0].s_field1 + obj_array[1].s_field1;
    }

    /* Parent environment update in loop header; 1 loop is epxected to be peeled */
    public String test_08(int n) {
        int testVar = 0;

        Cls2 c = new Cls2(n);
        Cls2 c1 = new Cls2(n*2);
        Cls1_ext c_ext = new Cls1_ext();
        boolean[] b = new boolean[n];

        int m = n - 1;
        String sum = "";
        String str = "cd";
        for (int i = 0; i < c.getArrInlined(m).length - 8; i++) {
            i++;
            b[i] = true;
        //    str = new String("CD");
            //c1 = new Cls2(c1.arr.length);
            c_ext.str_field.concat("ab");
        }

        m++;
        n++;
        testVar++;
        
        sum = sum.concat(c_ext.str_field);

        if (b[0] || c1.arr[0]) {
            return sum + n;
        } else {
            return sum + n + 1;
        }

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
                try {
                    String names[] = c.getPackage().getName().split("\\.");
                    String testName = names[names.length-1];
                    System.out.println("Test "+testName+"; Subtest "+m.getName()+"; Result: "+ m.invoke(this, 10));
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                } catch (IllegalAccessException e) {
                    e.printStackTrace();
                } catch (InvocationTargetException e) {
                    e.printStackTrace();
                }
            }
        }



    }

    public static void main(String[] args)
    {
        new Main().runTests();
    }

}

