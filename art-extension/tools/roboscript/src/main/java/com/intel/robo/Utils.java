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
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.Writer;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.Map;
import java.util.TreeMap;

public class Utils {

    static public String CALC_NAMES[] = { "min", "max", "avg", "median", "stdev", "var" };
    static public String CSV_SEP = ";";
    static public String stamp = "\n[uitest] ";
    static public boolean verbose = false;

    static public void vlog(String text) {
        if (verbose) {
            log(text);
        }
    }

    static public void log(String text) {
        if (text != null) {
            System.out.print(text = stamp + text.replace("\n", stamp));
            // Log.d(tag, text);
        }
    }

    static public void log(String prefix, String text) {
        if (text != null) {
            prefix = prefix.replace("\r", "").replace("\n", "_");
            prefix = stamp + prefix;
            System.out.print(text = prefix + text.replace("\n", prefix));
            // Log.d(tag, text);
        }
    }

    static public void log(String text, Throwable e) {
        log(text);
        if (e != null) {
            log(e.toString());
            for (StackTraceElement ste : e.getStackTrace()) {
                log("    at " + ste.toString());
            }
        }
    }

    static public String thrd() {
        Thread t = Thread.currentThread();
        return "(thread '" + t.getName() + "' id=" + t.getId() + ")";
    }

    static public String nn(int n) {
        if (n < 10)
            return "0" + n;
        return "" + n;
    }

    static public String nnn(int n) {
        if (n < 10)
            return "00" + n;
        if (n < 100)
            return "0" + n;
        return "" + n;
    }

    static public String space3(int i) {
        if (i <= 9)
            return "  " + i;
        if (i <= 99)
            return " " + i;
        else
            return "" + i;
    }

    static public String space4(int i) {
        if (i <= 9)
            return "   " + i;
        if (i <= 99)
            return "  " + i;
        if (i <= 999)
            return " " + i;
        else
            return "" + i;
    }

    static public String stamp() {
        Calendar c = Calendar.getInstance();
        String stamp = c.get(Calendar.YEAR)
                + nn(c.get(Calendar.MONTH))
                + nn(c.get(Calendar.DAY_OF_MONTH)) + "_"
                + nn(c.get(Calendar.HOUR_OF_DAY))
                + nn(c.get(Calendar.MINUTE))
                + nn(c.get(Calendar.SECOND));
        return stamp;
    }

    public static String trimText(String s) {
        if (s == null)
            return s;
        s = s.trim();
        s = s.replace("\\n", " ");
        s = s.replace("\\r", " ");
        s = s.replace("\r\n", " ");
        s = s.replace("\n", " ");
        s = s.replace("\r", " ");
        while (s.indexOf("  ") >= 0)
            s = s.replace("  ", " ");
        return s;
    }

    static public String correctFileName(String name) {
        name = name.replace(":", "_");
        name = name.replace("<", "_");
        name = name.replace(">", "_");
        name = name.replace("\"", "_");
        name = name.replace("'", "_");
        name = name.replace("|", "_");
        name = name.replace("/", "_");
        name = name.replace("?", "_");
        return name;
    }

    static public String join(double[] s, Object sep) {
        StringBuffer sb = new StringBuffer();
        int i = 0;
        for (double o : s) {
            if (i > 0 && sep != null)
                sb.append(sep);
            sb.append(o);
            i++;
        }
        return sb.toString();
    }

    static public <T> String join(T[] s, Object sep) {
        StringBuffer sb = new StringBuffer();
        int i = 0;
        for (Object o : s) {
            if (i > 0 && sep != null)
                sb.append(sep);
            sb.append(o);
            i++;
        }
        return sb.toString();
    }

    static public String join(Collection<?> s, Object sep) {
        StringBuffer sb = new StringBuffer();
        int i = 0;
        for (Object o : s) {
            if (i > 0 && sep != null)
                sb.append(sep);
            sb.append(o);
            i++;
        }
        return sb.toString();
    }

    static public double min(double[] values) {
        double res = 0;
        if (values != null && values.length > 0) {
            res = values[0];
            for (double val : values)
                if (res > val)
                    res = val;
        }
        return res;
    }

    static public double max(double[] values) {
        double res = 0;
        if (values != null && values.length > 0) {
            res = values[0];
            for (double val : values)
                if (res < val)
                    res = val;
        }
        return res;
    }

    static public double average(double[] values) {
        double res = 0;
        if (values != null && values.length > 0) {
            for (double val : values)
                res += val;
            res /= values.length;
        }
        return res;
    }

    static public double geomean(double[] values) {
        double res = 0;
        if (values != null && values.length > 0) {
            res = 1;
            for (double val : values)
                res *= val;
            res = Math.pow(res, 1.0 / values.length);
        }
        return res;
    }

    static public double median(double[] values) {
        double res = 0;
        if (values != null && values.length > 0) {
            ArrayList<Double> list = new ArrayList<Double>();
            for (double val : values)
                list.add(val);
            Collections.sort(list);
            if ((list.size() % 2) == 0)
                res = (list.get(list.size() / 2 - 1) + list.get(list.size() / 2)) / 2;
            else
                res = list.get(list.size() / 2);
        }
        return res;
    }

    static public double stdev1(double[] values) {
        double res = 0;
        if (values != null && values.length > 1) {
            double a = average(values);
            for (double val : values)
                res += (val - a) * (val - a);
            res /= values.length;
            res = Math.sqrt(res);
        }
        return res;
    }

    static public double stdev2(double[] values) {
        double res = 0;
        if (values != null && values.length > 1) {
            double a = average(values);
            for (double val : values)
                res += (val - a) * (val - a);
            res /= values.length - 1;
            res = Math.sqrt(res);
        }
        return res;
    }

    static public double[] sortValues(double[] values) {
        values = values.clone();
        ArrayList<Double> list = new ArrayList<Double>();
        for (double val : values)
            list.add(val);
        Collections.sort(list);
        for (int i = 0; i < values.length; i++)
            values[i] = list.get(i);
        return values;
    }

    static public double[][] sortMatrix(double[][] values) {
        values = values.clone();
        for (int y = 0; y < values.length; y++)
            values[y] = sortValues(values[y]);
        return values;
    }

    static public double[][] sortMatrix(double[][] values, final int mainIndex) {
        values = values.clone();
        int w = values[0].length;
        int h = values.length;
        final double[][] a = new double[w][h];
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                a[x][y] = values[y][x];
        ArrayList<Object> list = new ArrayList<Object>();
        for (double[] val : a)
            list.add(val);
        Collections.sort(list, new Comparator<Object>() {
            @Override
            public int compare(Object o1, Object o2) {
                double[] a1 = (double[]) o1;
                double[] a2 = (double[]) o2;
                return a1[mainIndex] > a2[mainIndex] ? 1 : a1[mainIndex] < a2[mainIndex] ? -1 : 0;
            }
        });
        for (int i = 0; i < a.length; i++)
            a[i] = (double[]) list.get(i);
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                values[y][x] = a[x][y];
        return values;
    }

    static public double[] getMiddle(double[] values, int count) {
        int i0 = (values.length - count) / 2;
        if (i0 < 0)
            i0 = 0;
        int i1 = i0 + count;
        if (i1 > values.length)
            i1 = values.length;
        double[] res = new double[i1 - i0];
        for (int i = i0; i < i1; i++)
            res[i - i0] = values[i];
        return res;
    }

    static public int indexOf(String[] strs, String str) {
        for (int i = 0; i < strs.length; i++) {
            if (strs[i].equals(str))
                return i;
        }
        return -1;
    }

    static public double[] values(Collection<?> objs) {
        double[] values = new double[objs.size()];
        int i = 0;
        for (Object obj : objs) {
            values[i++] = Double.parseDouble(obj.toString());
        }
        return values;
    }

    static public double[][] values(Collection<?>[] objs) {
        double[][] values = new double[objs.length][objs[0].size()];
        for (int y = 0; y < objs.length; y++) {
            int i = 0;
            for (Object obj : objs[y]) {
                values[y][i++] = Double.parseDouble(obj.toString());
            }
        }
        return values;
    }

    @SuppressWarnings("unchecked")
    static public ArrayList<String>[] makeStringArrayList(int len) {
        ArrayList<String>[] results = new ArrayList[len];
        for (int i = 0; i < len; i++)
            results[i] = new ArrayList<String>();
        return results;
    }

    static public void printResults(String prefix, ArrayList<String>[] results, String[] resultNames) {
        for (int i = 0; i < resultNames.length; i++) {
            log(prefix + ";" + resultNames[i] + ";original (" + results[i].size() + ");" + Utils.join(results[i], ";") + ";");
        }
    }

    static public void addResults(ArrayList<String>[] results, String[] res) {
        for (int i = 0; i < results.length; i++) {
            if (i < res.length)
                results[i].add(res[i]);
            else
                results[i].add("");
        }
    }

    static public void toCollection(Collection<String>[] objs, double[][] ddd) {
        for (int y = 0; y < objs.length; y++) {
            objs[y].clear();
            for (int x = 0; x < ddd[y].length; x++) {
                objs[y].add("" + ddd[y][x]);
            }
        }
    }

    static public void closeStream(InputStream s) {
        try {
            if (s != null)
                s.close();
        } catch (IOException e) {
        }
    }

    static public void closeStream(OutputStream s) {
        try {
            if (s != null)
                s.close();
        } catch (IOException e) {
        }
    }

    static public String loadString(String file, String lineFeed, int maxLength) throws IOException {
        return loadString(new File(file), lineFeed, maxLength);
    }

    static public String loadString(File file, String lineFeed, int maxLength) throws IOException {
        String ret = null;
        InputStream is = null;
        try {
            is = new FileInputStream(file);
            ret = loadString(is, lineFeed, maxLength);
        } finally {
            closeStream(is);
        }
        return ret;
    }

    static public String loadString(InputStream is, String lineFeed, int maxLength) throws IOException {
        String ret = null;
        if (is == null)
            return ret;
        if (maxLength <= 0)
            return ret;
        BufferedReader bis = new BufferedReader(new InputStreamReader(is));
        String line;
        StringBuffer buff = new StringBuffer();
        while (buff.length() < maxLength && (line = bis.readLine()) != null) {
            if (buff.length() + line.length() > maxLength)
                line = line.substring(0, maxLength - buff.length());
            buff.append(line);
            buff.append(lineFeed);
        }
        ret = buff.toString();
        return ret;
    }

    static public void saveString(String file, String str) throws IOException {
        saveString(new File(file), str);
    }

    static public void saveString(File file, String str) throws IOException {
        OutputStream is = null;
        try {
            is = new FileOutputStream(file, false);
            saveString(is, str);
        } finally {
            closeStream(is);
        }
    }

    static public void saveString(OutputStream os, String str) throws IOException {
        if (os == null)
            return;
        os.write(str.getBytes());
        //BufferedWriter bis = new BufferedWriter(new OutputStreamWriter(os));
        //bis.write(str);
    }

    static public Map<String, Method> collectScriptableMethods(Class<?> clazz) {
        Map<String, Method> map = new TreeMap<String, Method>();
        collectScriptableMethods(clazz, map);
        return map;
    }

    static public int collectScriptableMethods(Class<?> clazz, Map<String, Method> map) {
        int n = 0;
        Method[] allMethods = clazz.getMethods();
        Method[] skipMethods = Object.class.getMethods();
        loop: for (Method method : allMethods) {
            String name = method.getName();
            for (Method skip : skipMethods) {
                if (skip.equals(method)) {
                    continue loop;
                }
            }
            if (map.containsKey(name)) {
                Method method0 = map.get(name);
                if (method.getParameterTypes().length > method0.getParameterTypes().length)
                    map.put(name, method);
            } else {
                map.put(name, method);
                n++;
            }
        }
        return n;
    }

    static public String makeScriptProlog(String refName, Map<String, Method> map, String sep) {
        String prolog = "";
        for (Method method : map.values()) {
            String name = method.getName();
            int paramCount = method.getParameterTypes().length;
            String func = "function " + name + "(";
            for (int i = 0; i < paramCount; i++) {
                if (i > 0)
                    func += ",";
                func += "p" + i;
            }
            func += ") { return ";
            for (int param = paramCount - 1; param >= 0; param--) {
                func += "(typeof p" + param + " === 'undefined') ? ";
            }
            for (int count = 0; count <= paramCount; count++) {
                if (count > 0)
                    func += " : ";
                func += refName + "." + name + "(";
                for (int i = 0; i < count; i++) {
                    if (i > 0)
                        func += ",";
                    func += "p" + i;
                }
                func += ")";
            }
            func += "; }";
            prolog += func + sep;
        }
        return prolog;
    }

   static public int execFinish(Writer w, Process p) throws Exception {
        BufferedReader bf = new BufferedReader(new InputStreamReader(p.getInputStream()));
        String line;
        while ((line = bf.readLine()) != null) {
            w.write(line);
        }
        return p.waitFor();
    }

    static public String cutString(String data, String start, String end) {
        int pos = data.indexOf(start);
        if (pos >= 0)
            data = data.substring(pos + start.length());
        pos = data.indexOf(end);
        if (pos >= 0)
            data = data.substring(0, pos);
        return data;
    }
    
    static public boolean checkAccuracy(ArrayList<String> strs, double accuracy, int count) {
        if (strs == null || strs.size() == 0)
            return false;
        return checkAccuracy(Utils.values(strs), accuracy, count);
    }

    static public boolean checkAccuracy(double[] values, double accuracy, int count) {
        if (values == null || values.length == 0)
            return false;
        int total = values.length;
        values = Utils.getMiddle(Utils.sortValues(values), count);
        double variability = 100 * Utils.stdev2(values) / Utils.average(values);
        log("Accuracy check: total: " + total + ", middle: " + values.length + ", variability: " + variability + ", accuracy: " + accuracy
                + ", result: " + (variability <= accuracy));
        return variability <= accuracy;
    }

    static public boolean checkResults(ArrayList<String>[] results, String[] resultNames, int measures, int accuracy, String mainScore, String mainScores) {
        if (results[0].size() < 2) {
            log("Accuracy check: too few results: " + results[0].size());
            return false;
        }
        if (mainScore != null) {
            log("Checking accuracy for main score: " + mainScore);
            return checkAccuracy(results[Utils.indexOf(resultNames, mainScore)], accuracy, measures);
        }
        boolean res = true;
        boolean checkName;
        for (int i = 0; i < resultNames.length; i++) {
            if (mainScores != null) {
                checkName = mainScores.indexOf(resultNames[i]) >= 0;
            } else {
                checkName = true;
            }
            if (checkName) {
                log("Checking accuracy for: " + resultNames[i]);
                if (!checkAccuracy(results[i], accuracy, measures))
                    res = false;
            }
        }
        return res;
    }

    static public String calcNames() {
        return Utils.join(CALC_NAMES, CSV_SEP);
    }

    static public String calcRow(Collection<?> strs) {
        double[] values = Utils.values(strs);
        return calcRow(values);
    }

    static public String calcRow(double[] values) {
        double[] res = new double[] { Utils.min(values), Utils.max(values), Utils.average(values), Utils.median(values), Utils.stdev2(values),
                Utils.stdev2(values) / Utils.average(values) };
        return Utils.join(res, CSV_SEP);
    }

    static public void printResults(String workloadNameRev, ArrayList<String>[] results, String[] resultNames, String resultsPrefix, String scoreSuffix, int measures, String mainScore) {
        double[] midResults;
        String header;
        ArrayList<String> output = new ArrayList<String>();

        // print original collected scores
        header = resultsPrefix + "Benchmark" + CSV_SEP + "Score" + scoreSuffix + CSV_SEP + "original (" + results[0].size() + ")" + CSV_SEP;
        for (int i = 0; i < results[0].size(); i++) {
            header += "run" + (i + 1) + CSV_SEP;
        }
        header += CSV_SEP + calcNames();
        output.add(header);

        for (int i = 0; i < resultNames.length; i++) {
            output.add(resultsPrefix + workloadNameRev + CSV_SEP + resultNames[i] + scoreSuffix + CSV_SEP + "original (" + results[0].size() + ")" + CSV_SEP
                    + Utils.join(results[i], CSV_SEP) + CSV_SEP + "-" + CSV_SEP + calcRow(results[i]));
        }
        output.add(resultsPrefix);

        if (measures > results[0].size())
            measures = results[0].size();

        // print sorted scores
        header = resultsPrefix + "Benchmark" + CSV_SEP + "Score" + scoreSuffix + CSV_SEP + "sorted (" + measures + ")" + CSV_SEP;
        for (int i = 0; i < measures; i++)
            header += "value" + (i + 1) + CSV_SEP;
        header += CSV_SEP + calcNames();
        output.add(header);

        double[][] scroreValues = Utils.values(results);
        if (mainScore != null)
            scroreValues = Utils.sortMatrix(scroreValues, Utils.indexOf(resultNames, mainScore));
        else
            scroreValues = Utils.sortMatrix(scroreValues);
        results = Utils.makeStringArrayList(resultNames.length);
        Utils.toCollection(results, scroreValues);

        for (int i = 0; i < resultNames.length; i++) {
            midResults = Utils.getMiddle(Utils.values(results[i]), measures);
            output.add(resultsPrefix + workloadNameRev + CSV_SEP + resultNames[i] + scoreSuffix + CSV_SEP + "sorted (" + measures + ")" + CSV_SEP + Utils.join(midResults, CSV_SEP)
                    + CSV_SEP + "-" + CSV_SEP + calcRow(midResults));
        }

        for (String s : output) {
            log(s);
        }
    }
}
