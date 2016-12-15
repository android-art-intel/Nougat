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
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.io.Writer;
import java.util.ArrayList;

import org.xml.sax.Attributes;
import org.xml.sax.ContentHandler;
import org.xml.sax.InputSource;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.XMLReaderFactory;

import com.android.uiautomator.core.UiCollection;
import com.android.uiautomator.core.UiDevice;
import com.android.uiautomator.core.UiObject;
import com.android.uiautomator.core.UiObjectNotFoundException;
import com.android.uiautomator.core.UiScrollable;
import com.android.uiautomator.core.UiSelector;
import com.android.uiautomator.testrunner.UiAutomatorTestCase;
import com.android.*;

import android.graphics.Point;
import android.graphics.Rect;
import android.os.Environment;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;

public abstract class TestBase extends UiAutomatorTestCase {

    static class ExitException extends RuntimeException {
        private static final long serialVersionUID = 1L;
    }

    static class ErrorException extends RuntimeException {
        private static final long serialVersionUID = 1L;
    }

    static protected boolean logOperationFailed = true;

    static public final String sdcard;
    static public final String externalStorage;
    static boolean externalStorageReadOnly;
    static {
        String s = "/";
        sdcard = s + "sdcard";
        String state = "";
        try {
            state = Environment.getExternalStorageState();
        } catch (ArrayIndexOutOfBoundsException e1) {
        } catch (Exception e) {
            log("Failed to getExternalStorageState: " + e);
        }
        if (state.equals(Environment.MEDIA_MOUNTED) || state.equals(Environment.MEDIA_MOUNTED_READ_ONLY)) {
            externalStorage = Environment.getExternalStorageDirectory().getAbsolutePath();
            externalStorageReadOnly = state.equals(Environment.MEDIA_MOUNTED_READ_ONLY);
        } else {
            externalStorage = sdcard;
        }
    }

    protected UiDevice device;
    protected long startTime = currentTime();
    protected int pid;
    protected int inputRetries = 3;
    protected int appStartSleep = 1000;
    protected int autoSleep = 300;
    protected int retrySleep = 1000;
    protected int maxRunTime = 900000;
    protected int screenshotDelay = 3000;
    protected int doubleClickDelay = 50;
    protected int profilingStep;
    protected boolean tryDumpByInstance = true;
    protected boolean dumpParameters = true;
    protected boolean takeScreenshots = true;
    protected boolean collectLogcat;
    protected boolean saveLogcatCopy;
    protected boolean collectThermalEvents;
    protected boolean collectGCEvents;
    protected boolean keepActivities;
    protected boolean profiling;
    protected String status;
    protected String logcatOptions = "";
    protected String dumpDir = "/data/local/tmp";
    protected String dataRoot = '/' + "data/data/";
    protected String gcprofileDir = "/data/local/tmp/gcprofile";
    protected String setUpCmd;
    protected String setUpCmdOutput;
    protected String tearDownCmd;
    protected String tearDownCmdOutput;
    protected File screenshotRoot = new File(sdcard, "robo-screenshots");

    public TestBase() {
    }

    public String getScenarioName() {
        return getName();
    }

    public int autoSleep() {
        return autoSleep;
    }

    public void autoSleep(int v) {
        autoSleep = v;
    }

    public int doubleClickDelay() {
        return doubleClickDelay;
    }

    public void doubleClickDelay(int v) {
        doubleClickDelay = v;
    }

    public boolean tryDumpByInstance() {
        return tryDumpByInstance;
    }

    public void tryDumpByInstance(boolean v) {
        tryDumpByInstance = v;
    }

    public String screenshotRoot() {
        return screenshotRoot.getAbsolutePath();
    }

    public void screenshotRoot(String v) {
        screenshotRoot = new File(v);
    }

    public UiSelector UiSelector() {
        return new UiSelector();
    }

    public UiScrollable UiScrollable(UiSelector selector) {
        return new UiScrollable(selector);
    }

    public UiCollection UiCollection(UiSelector selector) {
        return new UiCollection(selector);
    }

    public UiObject UiObject(UiSelector selector) {
        return new UiObject(selector);
    }

    public UiObject byResourceId(String resID) {
        return UiObject(UiSelector().resourceId(resID));
    }

    public UiObject byText(String text) {
        return UiObject(UiSelector().text(text));
    }

    public UiObject byTextMatches(String text) {
        return UiObject(UiSelector().textMatches(text));
    }

    public UiObject byTextContains(String text) {
        return UiObject(UiSelector().textContains(text));
    }

    public String getPropName(String propName) {
        return propName.replace("-", "_").replace(":", "_");
    }

    public boolean getSysPropBool(String propName, boolean defValue) {
        return Boolean.parseBoolean(getSystemProperty(getPropName(propName), "" + defValue));
    }

    public int getSysPropInt(String propName, int defValue) {
        try {
            return Integer.parseInt(getSystemProperty(getPropName(propName), "" + defValue));
        } catch (Exception e) {
            return (int) Double.parseDouble(getSystemProperty(getPropName(propName), "" + defValue));
        }
    }

    public float getSysPropFloat(String propName, float defValue) {
        try {
            return Float.parseFloat(getSystemProperty(getPropName(propName), "" + defValue));
        } catch (Exception e) {
            return (float) Double.parseDouble(getSystemProperty(getPropName(propName), "" + defValue));
        }
    }

    public String getSystemProperty(String propName) {
        if (getParams().containsKey(propName)) {
            return getParams().get(propName).toString().replace("_SP_", " ").replace("_COMMA_", ",").replace("_COMMA_", ",").replace("_SEMICOLON_", ";");
        } else {
            error("No such property: " + propName);
            return null;
        }
    }

    public String getSystemProperty(String propName, String defValue) {
        if (getParams().containsKey(propName)) {
            defValue = getParams().get(propName).toString();
        }
        if ("EMPTY".equals(defValue)) {
            defValue = "";
        }
        return defValue != null ? defValue.replace("_Q_", "'").replace("_QQ_", "\"").replace("_LPAR_", "(").replace("_RPAR_", ")").replace("_SP_", " ")
                .replace("_LOWER_", "<").replace("_GRATER_", ">").replace("_OR_", "|").replace("_COMMA_", ",").replace("_SEMICOLON_", ";") : null;
    }

    public String getSysPropStr(String propName, String defValue) {
        return getSystemProperty(getPropName(propName), defValue);
    }

    public void sleep(float time) {
        sleep((int) time);
    }

    public void sleep(int time) {
        sleep(time, null);
    }

    public void sleep(int time, String msg) {
        if (msg != null) {
            log(msg + " (Sleeping " + time + ")...");
        }
        // log("Sleeping " + time + "...");
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            log("Sleep " + time + "interrupted!");
        }
    }

    public void dumpBaseParameters() {
        log("Current run time: " + currentRunTime());
        log("Base parameters:");
        log("  -e verbose " + Utils.verbose);
        log("  -e appStartSleep " + appStartSleep);
        log("  -e autoSleep " + autoSleep);
        log("  -e retrySleep " + retrySleep);
        log("  -e inputRetries " + inputRetries);
        log("  -e maxRunTime " + maxRunTime);
        log("  -e collectLogcat " + collectLogcat);
        log("  -e collectThermalEvents " + collectThermalEvents);
        log("  -e profiling " + profiling);
        log("  -e screenshotRoot " + screenshotRoot);
        log("  -e gcprofileDir " + gcprofileDir);
        log("  -e dumpDir " + dumpDir);
        log("  -e tearDownCmd " + tearDownCmd);
        log("  -e tearDownCmdOutput " + tearDownCmdOutput);
        log("  -e setUpCmd " + setUpCmd);
        log("  -e setUpCmdOutput " + setUpCmdOutput);
    }

    public void dumpSystemParameters() {
        log("Input parameters:");
        for (String key : getParams().keySet()) {
            log("  -e " + key + " " + getParams().get(key));
        }
        // log("DataDirectory: " + Environment.getDataDirectory());
    }

    public void loadParameters() {
        autoSleep = getSysPropInt("autoSleep", autoSleep);
        appStartSleep = getSysPropInt("appStartSleep", appStartSleep);
        inputRetries = getSysPropInt("inputRetries", inputRetries);
        retrySleep = getSysPropInt("autoSleep", retrySleep);
        maxRunTime = getSysPropInt("maxRunTime", maxRunTime);
        screenshotDelay = getSysPropInt("screenshotDelay", screenshotDelay);
        doubleClickDelay = getSysPropInt("doubleClickDelay", doubleClickDelay);
        takeScreenshots = getSysPropBool("takeScreenshots", takeScreenshots);
        collectLogcat = getSysPropBool("collectLogcat", collectLogcat);
        saveLogcatCopy = getSysPropBool("saveLogcatCopy", saveLogcatCopy);
        collectThermalEvents = getSysPropBool("collectThermalEvents", collectThermalEvents);
        collectGCEvents = getSysPropBool("collectGCEvents", collectGCEvents);
        keepActivities = getSysPropBool("keepActivities", keepActivities);
        dumpParameters = getSysPropBool("dumpParameters", dumpParameters);
        Utils.verbose = getSysPropBool("verbose", Utils.verbose);
        Utils.CSV_SEP = getSysPropStr("CSV_SEP", Utils.CSV_SEP);
        profiling = getSysPropBool("profiling", profiling);
        tryDumpByInstance = getSysPropBool("tryDumpByInstance", tryDumpByInstance);
        logcatOptions = getSysPropStr("logcatOptions", logcatOptions);
        screenshotRoot = new File(getSysPropStr("screenshotRoot", screenshotRoot.getAbsolutePath()));
        gcprofileDir = getSysPropStr("gcprofileDir", gcprofileDir);
        dumpDir = getSysPropStr("dumpDir", dumpDir);
        setUpCmd = getSysPropStr("setUpCmd", setUpCmd);
        setUpCmdOutput = getSysPropStr("setUpCmdOutput", setUpCmdOutput);
        tearDownCmd = getSysPropStr("tearDownCmd", tearDownCmd);
        tearDownCmdOutput = getSysPropStr("tearDownCmdOutput", tearDownCmdOutput);
    }

    @Override
    protected void setUp() throws Exception {
        vlog("setUp " + getScenarioName() + " " + Utils.thrd());
        // checkParams();
        status = "failed";
        loadParameters();
        if (dumpParameters) {
            dumpSystemParameters();
            dumpBaseParameters();
        }
        device = getUiDevice();
        if (setUpCmd != null && setUpCmd.trim().length() > 0) {
            log("Running setUp command...");
            String output = shell(setUpCmd.trim());
            if (setUpCmdOutput != null && setUpCmdOutput.trim().length() > 0) {
                log("[setUpCmd]", "Writing to file " + setUpCmdOutput.trim() + "...");
                writeStringToFile(setUpCmdOutput.trim(), output);
            } else {
                log("[setUpCmd]", output);
            }
        }
    }

    @Override
    protected void tearDown() throws Exception {
        vlog("tearDown " + getScenarioName() + " " + Utils.thrd());
        if (tearDownCmd != null && tearDownCmd.trim().length() > 0) {
            log("Running tearDown command...");
            String output = shell(tearDownCmd.trim());
            if (tearDownCmdOutput != null && tearDownCmdOutput.trim().length() > 0) {
                log("[tearDownCmd]", "Writing to file " + setUpCmdOutput.trim() + "...");
                writeStringToFile(tearDownCmdOutput.trim(), output);
            } else {
                log("[tearDownCmd]", output);
            }
        }
        super.tearDown();
        device.setOrientationNatural();
        device.unfreezeRotation();
        stopLogcat();
    }

    public void operationFailed(String text) {
        if (logOperationFailed)
            log(text);
        throw new RuntimeException(text);
    }

    static public void vlog(String text) {
        Utils.vlog(text);
    }

    static public void log(String text) {
        Utils.log(text);
    }

    static public void log(String prefix, String text) {
        Utils.log(prefix, text);
    }

    static public void log(String text, Throwable e) {
        Utils.log(text, e);
    }

    public boolean findElem(String elem) {
        UiObject uiObject = searchView(elem);
        return uiObject != null && uiObject.exists();
    }

    public UiObject[] searchViews(String elem) {
        UiObject baseElem = searchView(elem);
        UiSelector uiSelector = baseElem.getSelector();
        ArrayList<UiObject> uiObjects = new ArrayList<UiObject>();
        int i = 0;
        while (true) {
            UiObject nextElem = UiObject(uiSelector.instance(i));
            if (nextElem.exists()) {
                uiObjects.add(nextElem);
                i++;
            } else {
                break;
            }
        }
        return uiObjects.toArray(new UiObject[0]);
    }

    public UiObject searchView(String elem) {
        UiObject uiObject;
        int index = elem.lastIndexOf("_#");
        if (index > 0) {
            try {
                // 1-based index
                index = Integer.parseInt(elem.substring(index + 2));
                index--;
                elem = elem.substring(0, elem.lastIndexOf("_#"));
            } catch (Exception e) {
            }
        }
        if (elem.startsWith("id/") && elem.indexOf(":") <= 0) {
            // by relative resource id
            String packageName = device.getCurrentPackageName();
            if (packageName == null || packageName.length() == 0) {
                packageName = getTestPackage();
            }
            while (true) {
                if (packageName != null && packageName.length() > 0) {
                    String resID = packageName + ":" + elem;
                    vlog("searchView: trying resourceId('" + resID + "')...");
                    uiObject = UiObject(UiSelector().resourceId(resID));
                    if (uiObject.exists()) {
                        if (index >= 0) {
                            vlog("searchView: " + resID + " found by resourceId, trying instance=" + index + "...");
                            uiObject = UiObject(uiObject.getSelector().instance(index));
                        } else {
                            vlog("searchView: " + resID + " found by resourceId");
                        }
                        return uiObject;
                    }
                    if (packageName.equals(getTestPackage()) == false) {
                        packageName = getTestPackage();
                        continue;
                    }
                }
                break;
            }
        } else {
            // by full resource id
            vlog("searchView: trying resourceId('" + elem + "')...");
            uiObject = UiObject(UiSelector().resourceId(elem));
            if (uiObject.exists()) {
                if (index >= 0) {
                    vlog("searchView: " + elem + " found by resourceId, trying instance=" + index + "...");
                    uiObject = UiObject(uiObject.getSelector().instance(index));
                } else {
                    vlog("searchView: " + elem + " found by resourceId");
                }
                return uiObject;
            }
            // by class
            vlog("searchView: trying className('" + elem + "')...");
            uiObject = UiObject(UiSelector().className(elem));
            if (uiObject.exists()) {
                if (index >= 0) {
                    vlog("searchView: " + elem + " - found by className, trying instance=" + index + "...");
                    uiObject = UiObject(uiObject.getSelector().instance(index));
                } else {
                    vlog("searchView: " + elem + " - found by className");
                }
                return uiObject;
            }
        }
        // by text
        vlog("searchView: trying text('" + elem + "')...");
        uiObject = UiObject(UiSelector().text(elem));
        if (uiObject.exists()) {
            if (index >= 0) {
                vlog("searchView: " + elem + " - found by text, trying instance=" + index + "...");
                uiObject = UiObject(uiObject.getSelector().instance(index));
            } else {
                vlog("searchView: " + elem + " - found by text");
            }
            return uiObject;
        }
        // by description
        vlog("searchView: trying description('" + elem + "')...");
        uiObject = UiObject(UiSelector().description(elem));
        if (uiObject.exists()) {
            if (index >= 0) {
                vlog("searchView: " + elem + " - found by description, trying instance=" + index + "...");
                uiObject = UiObject(uiObject.getSelector().instance(index));
            } else {
                vlog("searchView: " + elem + " - found by description");
            }
            return uiObject;
        }
        if (index >= 0) {
            vlog("searchView: " + elem + " - not found! instance=" + index);
        } else {
            vlog("searchView: " + elem + " - not found!");
        }
        return uiObject;
    }

    public UiObject searchViewMatches(String elem) {
        UiObject uiObject = searchView(elem);
        if (uiObject.exists())
            return uiObject;
        vlog("searchViewMatches: trying textStartsWith('" + elem + "')...");
        uiObject = UiObject(UiSelector().textStartsWith(elem));
        if (uiObject.exists()) {
            vlog("searchViewMatches: " + elem + " - found by textStartsWith");
            return uiObject;
        }
        vlog("searchViewMatches: trying textContains('" + elem + "')...");
        uiObject = UiObject(UiSelector().textContains(elem));
        if (uiObject.exists()) {
            vlog("searchViewMatches: " + elem + " - found by textContains");
            return uiObject;
        }
        vlog("searchViewMatches: trying textMatches('" + elem + "')...");
        uiObject = UiObject(UiSelector().textMatches(elem));
        if (uiObject.exists()) {
            vlog("searchViewMatches: " + elem + " - found by textMatches");
            return uiObject;
        }
        vlog("searchViewMatches: " + elem + " - not found!");
        return uiObject;
    }

    // Basic UI operations
    public void setText(String elem, final String text) throws UiObjectNotFoundException {
        vlog("setText(\"" + elem + "\", \"" + text + "\")...");
        UiObject uiObject = searchView(elem);
        String oldText = uiObject.getText();
        uiObject.clearTextField();
        uiObject.setText(text);
        String newText = uiObject.getText();
        log("setText result for: " + elem + " new text: " + newText + " old text: " + oldText);
    }

    public String getText(String elem) throws UiObjectNotFoundException {
        vlog("getText(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        String text = uiObject.getText();
        vlog("getText result for: " + elem + " text: " + text);
        return text;
    }

    public String getDesc(String elem) throws UiObjectNotFoundException {
        vlog("getDesc(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        String text = uiObject.getContentDescription();
        vlog("getDesc result for: " + elem + " text: " + text);
        return text;
    }

    public void typeText(String text) {
        vlog("typeText(\"" + text + "\")...");
        KeyCharacterMap keyCharacterMap = KeyCharacterMap.load(KeyCharacterMap.VIRTUAL_KEYBOARD);
        KeyEvent[] events = keyCharacterMap.getEvents(text.toCharArray());
        if (events != null) {
            for (int i = 0; i < events.length; i++) {
                KeyEvent event = events[i];
                // KeyEvent event = KeyEvent.changeTimeRepeat(events[i],
                // SystemClock.uptimeMillis(), 0);
                if (event.getAction() == KeyEvent.ACTION_DOWN) {
                    log("event: " + event + " Characters:" + event.getCharacters() + " Action:" + event.getAction() + " DisplayLabel:"
                            + event.getDisplayLabel() + " RepeatCount:" + event.getRepeatCount());
                    device.pressKeyCode(event.getKeyCode(), event.getMetaState());
                }
            }
        } else {
            log("typeText - events os null");
        }
    }

    public void clickBack() {
        vlog("clickBack()...");
        device.pressBack();
    }

    public void clickHome() {
        vlog("clickHome()...");
        device.pressHome();
    }

    public void clickMenu() {
        vlog("clickMenu()...");
        device.pressMenu();
    }

    public void clickEnter() {
        vlog("clickEnter()...");
        device.pressEnter();
    }

    public void clickTab() {
        vlog("clickTab()...");
        device.pressKeyCode(KeyEvent.KEYCODE_TAB);
    }

    public boolean waitFor(String elem, int retries, int retryDelay) {
        return waitFor(new String[] { elem }, retries, retryDelay, null) >= 0;
    }

    public int waitFor(String[] elems, int retries, int retryDelay) {
        return waitFor(elems, retries, retryDelay, null);
    }

    public interface Callback {
        boolean isStopped(int retry);
    }

    public int waitFor(String elem, int retries, int retryDelay, Callback callback) {
        return waitFor(new String[] { elem }, retries, retryDelay, callback);
    }

    public int waitFor(String[] elems, int retries, int retryDelay, Callback callback) {
        vlog("waitFor( [" + Utils.join(elems, ",") + "], " + retries + ", " + retryDelay + ")...");
        for (int i = 0; i < elems.length; i++) {
            if (findElem(elems[i])) {
                vlog("Element found: #" + i + " - '" + elems[i] + "' (waited 0 ms)");
                return i;
            }
        }
        long startTime = currentRunTime();
        long printTime = startTime;
        String elemNames = Utils.join(elems, ",");
        for (int retry = 1; retry <= retries; retry++) {
            if (currentRunTime() - printTime >= 1000 || retryDelay >= 1000) {
                sleep(retryDelay, "Waiting for elements: " + elemNames + " (retry: " + retry + ")");
                printTime = currentRunTime();
            } else {
                sleep(retryDelay);
            }
            for (int i = 0; i < elems.length; i++) {
                if (findElem(elems[i])) {
                    long time = currentRunTime() - startTime;
                    vlog("Element found: #" + i + " - '" + elems[i] + "' (waited " + time + " ms)");
                    return i;
                }
            }
            if (callback != null && callback.isStopped(retry)) {
                log("Stopped waiting for elements: " + elemNames);
                break;
            }
        }
        log("Elements not found at last: " + elemNames);
        return -1;
    }

    public boolean click(String elem, float dx, float dy) throws UiObjectNotFoundException {
        vlog("click(\"" + elem + "\", " + dx + ", " + dy + ")...");
        UiObject uiObject = searchView(elem);
        Rect rect = uiObject.getVisibleBounds();
        int x = rect.left;
        int y = rect.top;
        x += rect.width() * dx;
        y += rect.height() * dy;
        if (device.click(x, y)) {
            sleep(autoSleep);
            return true;
        }
        return false;
    }

    public boolean click(String elem) throws UiObjectNotFoundException {
        vlog("click(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        if (uiObject.click()) {
            device.waitForIdle();
            sleep(autoSleep);
            return true;
        }
        return false;
    }

    public boolean clickAndWaitForNewWindow(String elem) throws UiObjectNotFoundException {
        vlog("clickAndWaitForNewWindow(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        if (uiObject.clickAndWaitForNewWindow()) {
            sleep(autoSleep);
            return true;
        }
        return false;
    }

    public boolean longClick(String elem) throws UiObjectNotFoundException {
        vlog("longClick(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        if (uiObject.longClick()) {
            sleep(autoSleep);
            return true;
        }
        return false;
    }

    public void doubleClick(int x, int y) {
        vlog("doubleClick(" + x + "," + y + ")...");
        device.click(x, y);
        sleep(doubleClickDelay);
        device.click(x, y);
        sleep(autoSleep);
    }

    public boolean maybeClick(String elem) {
        vlog("maybeClick(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        if (!uiObject.exists()) {
            vlog("Elem does not exist: " + elem);
            return false;
        }
        try {
            uiObject.click();
            sleep(autoSleep);
            return true;
        } catch (UiObjectNotFoundException e1) {
            log("Elem not found: " + elem);
            return false;
        }
    }

    public void exit() {
        vlog("exit()...");
        throw new ExitException();
    }

    public void error(String text) {
        vlog("error(\"" + text + "\")...");
        log("Error: " + text);
        throw new ErrorException();
    }

    public void setCheck(String elem, boolean checked) throws UiObjectNotFoundException {
        vlog("setCheck(\"" + elem + "\", " + checked + ")...");
        if (checked) {
            check(elem);
        } else {
            uncheck(elem);
        }
    }

    public void uncheck(String elem) throws UiObjectNotFoundException {
        vlog("uncheck(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        if (uiObject.isChecked()) {
            log("uncheck: " + elem + " - is checked, unchecking...");
            uiObject.click();
        } else {
            log("uncheck: " + elem + " - already unchecked");
        }
    }

    public void check(String elem) throws UiObjectNotFoundException {
        vlog("check(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        if (!uiObject.isChecked()) {
            vlog("check: " + elem + " - is unchecked, checking...");
            uiObject.click();
        } else {
            vlog("check: " + elem + " - already checked");
        }
    }

    public boolean isEnabled(String elem) throws UiObjectNotFoundException {
        vlog("isEnabled(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        return uiObject.isEnabled();
    }

    public boolean isChecked(String elem) throws UiObjectNotFoundException {
        vlog("isChecked(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        return uiObject.isChecked();
    }

    public boolean isCheckable(String elem) throws UiObjectNotFoundException {
        vlog("isCheckable(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        return uiObject.isCheckable();
    }

    public boolean isSelected(String elem) throws UiObjectNotFoundException {
        vlog("isSelected(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        return uiObject.isSelected();
    }

    public boolean isScrollable(String elem) throws UiObjectNotFoundException {
        vlog("isScrollable(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        return uiObject.isScrollable();
    }

    public boolean isFocused(String elem) throws UiObjectNotFoundException {
        vlog("isFocused(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        return uiObject.isFocused();
    }

    public boolean isFocusable(String elem) throws UiObjectNotFoundException {
        vlog("isFocusable(\"" + elem + "\")...");
        UiObject uiObject = searchView(elem);
        return uiObject.isFocusable();
    }

    public void flingLeft(String elem, int steps) throws UiObjectNotFoundException {
        vlog("flingLeft(\"" + elem + "\", " + steps + ")...");
        UiObject uiObject = searchView(elem);
        flingLeft(uiObject.getVisibleBounds(), steps);
    }

    public void flingLeft(Rect rect, int steps) {
        vlog("flingLeft(" + rect + ", " + steps + ")...");
        int dx = rect.width() / 3;
        if (dx < 1)
            return;
        device.swipe(rect.right - dx, rect.centerY(), rect.left + dx, rect.centerY(), steps);
    }

    public void flingRight(String elem, int steps) throws UiObjectNotFoundException {
        vlog("flingRight(\"" + elem + "\", " + steps + ")...");
        UiObject uiObject = searchView(elem);
        flingRight(uiObject.getVisibleBounds(), steps);
    }

    public void flingRight(Rect rect, int steps) {
        vlog("flingRight(" + rect + ", " + steps + ")...");
        int dx = rect.width() / 3;
        if (dx < 1)
            return;
        device.swipe(rect.left + dx, rect.centerY(), rect.right - dx, rect.centerY(), steps);
    }

    public void flingUp(String elem, int steps) throws UiObjectNotFoundException {
        vlog("flingUp(\"" + elem + "\", " + steps + ")...");
        UiObject uiObject = searchView(elem);
        flingUp(uiObject.getVisibleBounds(), steps);
    }

    public void flingUp(Rect rect, int steps) {
        vlog("flingUp(rect:" + rect + ", " + steps + ")...");
        int dy = rect.height() / 3;
        if (dy < 1)
            return;
        device.swipe(rect.centerX(), rect.bottom - dy, rect.centerX(), rect.top + dy, steps);
    }

    public void flingDown(String elem, int steps) throws UiObjectNotFoundException {
        vlog("flingDown(\"" + elem + "\", " + steps + ")...");
        UiObject uiObject = searchView(elem);
        flingDown(uiObject.getVisibleBounds(), steps);
    }

    public void flingDown(Rect rect, int steps) {
        vlog("flingDown(rect:" + rect + ", " + steps + ")...");
        int dy = rect.height() / 3;
        if (dy < 1)
            return;
        device.swipe(rect.centerX(), rect.top + dy, rect.centerX(), rect.bottom - dy, steps);
    }

    public void zoomIn(UiObject obj, int steps) throws UiObjectNotFoundException {
        vlog("zoomIn(" + obj + ", " + steps + ")...");
        Rect rect = obj.getVisibleBounds();
        int dx = rect.width() / 3;
        if (dx < 1)
            return;
        int dy = rect.height() / 3;
        if (dy < 1)
            return;
        Point startPoint1 = new Point();
        Point startPoint2 = new Point();
        Point endPoint1 = new Point();
        Point endPoint2 = new Point();
        obj.performTwoPointerGesture(startPoint1, startPoint2, endPoint1, endPoint2, steps);
    }

    public void dragCenter(String elem, float dx, float dy) throws UiObjectNotFoundException {
        vlog("dragCenter(\"" + elem + "\", " + dx + ", " + dy + ")...");
        UiObject uiObject = searchView(elem);
        dragCenter(uiObject, dx, dy, 0);
    }

    public void dragCenter(String elem, float dx, float dy, int steps) throws UiObjectNotFoundException {
        vlog("dragCenter(\"" + elem + "\", " + dx + ", " + dy + ", " + steps + ")...");
        UiObject uiObject = searchView(elem);
        dragCenter(uiObject, dx, dy, steps);
    }

    public void drag(String elem, float x0, float y0, float x1, float y1) throws UiObjectNotFoundException {
        vlog("drag(\"" + elem + "\", " + x0 + ", " + y0 + ", " + x1 + ", " + y1 + ")...");
        UiObject uiObject = searchView(elem);
        drag(uiObject, x0, y0, x1, y1, 0);
    }

    public void drag(String elem, float x0, float y0, float x1, float y1, int steps) throws UiObjectNotFoundException {
        vlog("drag(\"" + elem + "\", " + x0 + ", " + y0 + ", " + x1 + ", " + y1 + ", " + steps + ")...");
        UiObject uiObject = searchView(elem);
        drag(uiObject, x0, y0, x1, y1, steps);
    }

    protected void dragCenter(UiObject uiObject, float dx, float dy, int steps) throws UiObjectNotFoundException {
        drag(uiObject, 0.5f, 0.5f, 0.5f + dx, 0.5f + dy, steps);
    }

    protected void drag(UiObject uiObject, float x0, float y0, float x1, float y1, int steps) throws UiObjectNotFoundException {
        Rect rect = uiObject.getVisibleBounds();
        int startX = rect.left;
        int startY = rect.top;
        int endX = rect.left;
        int endY = rect.top;
        startX += rect.width() * x0;
        startY += rect.height() * y0;
        endX += rect.width() * x1;
        endY += rect.height() * y1;
        if (steps <= 0) {
            steps = Math.max(Math.abs(endX - startX), Math.abs(endY - startY)) / 10;
            if (steps < 2)
                steps = 2;
        }
        device.drag(startX, startY, endX, endY, steps);
    }

    public void swipe(String elem, float x0, float y0, float x1, float y1) throws UiObjectNotFoundException {
        vlog("swipe(\"" + elem + "\", " + x0 + ", " + y0 + ", " + x1 + ", " + y1 + ")...");
        UiObject uiObject = searchView(elem);
        swipe(uiObject, x0, y0, x1, y1, 0);
    }

    public void swipe(String elem, float x0, float y0, float x1, float y1, int steps) throws UiObjectNotFoundException {
        vlog("swipe(\"" + elem + "\", " + x0 + ", " + y0 + ", " + x1 + ", " + y1 + ", " + steps + ")...");
        UiObject uiObject = searchView(elem);
        swipe(uiObject, x0, y0, x1, y1, steps);
    }

    protected void swipe(UiObject uiObject, float x0, float y0, float x1, float y1, int steps) throws UiObjectNotFoundException {
        Rect rect = uiObject.getVisibleBounds();
        int startX = rect.left;
        int startY = rect.top;
        int endX = rect.left;
        int endY = rect.top;
        startX += rect.width() * x0;
        startY += rect.height() * y0;
        endX += rect.width() * x1;
        endY += rect.height() * y1;
        if (steps <= 0) {
            steps = Math.max(Math.abs(endX - startX), Math.abs(endY - startY)) / 10;
            if (steps < 2)
                steps = 2;
        }
        vlog("device.swipe(" + rect + " -> " + startX + ", " + startY + ", " + endX + ", " + endY + ", " + steps + ")...");
        device.swipe(startX, startY, endX, endY, steps);
    }

    public void pinchIn(String elem, int percent, int steps) throws UiObjectNotFoundException {
        vlog("pinchIn(\"" + elem + "\", " + percent + ", " + steps + ")...");
        UiObject uiObject = searchView(elem);
        uiObject.pinchIn(percent, steps);
    }

    public void pinchOut(String elem, int percent, int steps) throws UiObjectNotFoundException {
        vlog("pinchOut(\"" + elem + "\", " + percent + ", " + steps + ")...");
        UiObject uiObject = searchView(elem);
        uiObject.pinchOut(percent, steps);
    }

    public void closeParasite() {
        int retries = 5;
        closeParasite(retries, 1000);
    }

    public void closeParasite(int retries, int wait) {
        vlog("Close parasites, retries:" + retries + "...");
        UiObject launcher_not_responding = UiObject(UiSelector().textMatches("Launcher .*"));
        UiObject unfortunately = UiObject(UiSelector().textMatches("Unfortunately, .* has stopped."));
        UiObject internal_problem = UiObject(UiSelector().textMatches("There's an internal problem with your device.*"));
        UiObject OK = UiObject(UiSelector().text("OK"));
        while (true) {
            if (launcher_not_responding.exists()) {
                vlog("FOUND: Launcher isn't responding! (" + retries + ")");
            } else if (unfortunately.exists()) {
                vlog("FOUND: Unfortunately! (" + retries + ")");
            } else if (internal_problem.exists()) {
                vlog("FOUND: internal problem! (" + retries + ")");
            } else if (OK.exists()) {
                vlog("FOUND: OK! (" + retries + ")");
            } else {
                vlog("No parasites found! (" + retries + ")");
                break;
            }
            try {
                OK.click();
                sleep(wait);
                device.waitForIdle();
            } catch (UiObjectNotFoundException e) {
                log("NOT FOUND: OK!");
                break;
            }
            retries--;
            if (retries <= 0)
                break;
        }
    }

    public String exec(String cmd, boolean verbose) {
        return exec(cmd, verbose, null);
    }

    public String exec(String cmd, boolean verbose, String pat) {
        vlog("exec(\"" + cmd + "\", " + verbose + ", " + pat + ")...");
        try {
            return new ProcessHandler(null, cmd, verbose, pat).exec();
        } catch (Exception e) {
            log("Process exec failed!", e);
            operationFailed("Process exec failed: " + cmd);
            return "";
        }
    }

    public String exec(String cmd) {
        vlog("exec(\"" + cmd + "\")...");
        return exec(cmd, Utils.verbose);
    }

    public String shell(String cmd) {
        vlog("shell(\"" + cmd + "\")...");
        return exec(cmd, Utils.verbose);
    }

    static public String extractString(String data, String name, String printName) {
        String[] names = name.split("_#_");
        int pos = 0;
        for (int i = 0; i < names.length; i++) {
            int p = data.indexOf(names[i], pos);
            if (p < 0) {
                if (printName != null)
                    log(printName + ": " + name + " [n/a]");
                return "0";
            }
            pos = p + names[i].length();
        }
        String result = data.substring(pos, data.indexOf("\n", pos)).replace("\r", "").replace("\n", "").trim();
        // log("resul_: " + name + " [" + result + "]");
        result = result.replace(" ", "");
        if (printName != null)
            log(printName + ": " + name + " [" + result + "]");
        return result;
    }

    public long currentTime() {
        return System.currentTimeMillis();
    }

    public boolean hasTime() {
        return currentTime() - startTime < maxRunTime;
    }

    public long currentRunTime() {
        return currentTime() - startTime;
    }

    public void writeStringToFile(String file, String data) {
        log("writeStringToFile: " + file + " bytes: " + data.length());
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(file, true);
            fos.write(data.getBytes("UTF-8"));
            log("writeStringToFile saved bytes: " + data.length());
            log("File size: : " + new File(file).length());
        } catch (Exception e) {
            log("writeStringToFile failed: " + e.getMessage());
            if (Utils.verbose) {
                e.printStackTrace();
            }
        } finally {
            if (fos != null)
                try {
                    fos.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
        }
    }

    protected ProcessHandler logcatHandler;
    protected final StringBuffer logcat = new StringBuffer();

    public void clearLogcat() {
        synchronized (logcat) {
            log("Clearing in-memory Logcat: " + logcat.length() + " chars");
            logcat.setLength(0);
        }
    }

    public String getLogcat() {
        synchronized (logcat) {
            return logcat.toString();
        }
    }

    protected ProcessHandler getLogcatProcessHandler() throws Exception {
        return new ProcessHandler(saveLogcatCopy ? getScenarioName() : null, "logcat " + logcatOptions, Utils.verbose);
    }

    public void startLogcat() {
        if (logcatHandler != null) {
            log("Logcat already started");
            return;
        }
        log("Starting Logcat...");
        try {
            logcatHandler = getLogcatProcessHandler();
        } catch (Exception e) {
            log("Logcat creation failed!", e);
            operationFailed("Logcat creation failed!");
        }
        Thread logcatThread = new Thread() {
            @Override
            public void run() {
                try {
                    log("Logcat thread started");
                    logcatHandler.exec(logcat);
                } catch (Exception e) {
                    log("Logcat exec failed!", e);
                }
            }
        };
        logcatThread.setDaemon(true);
        logcatThread.start();
    }

    public ProcessHandler execAsync(String cmd, String output) {
        log("execAsync: " + cmd + "...");
        ProcessHandler processHandler = null;
        PrintStream fos = null;
        try {
            if (output != null && output.length() > 0) {
                fos = new PrintStream(new FileOutputStream(output, false));
            }
            processHandler = new ProcessHandler(null, cmd, Utils.verbose);
        } catch (Exception e) {
            log("execAsync: ProcessHandler creation failed!", e);
            operationFailed("ProcessHandler creation failed in execAsync!");
            return null;
        }
        final ProcessHandler ph = processHandler;
        final PrintStream fs = fos;
        Thread execThread = new Thread() {
            @Override
            public void run() {
                try {
                    log("execAsync thread started");
                    ph.exec(new Writer() {
                        @Override
                        public void close() throws IOException {
                        }

                        @Override
                        public void flush() throws IOException {
                        }

                        @Override
                        public void write(char[] buf, int offset, int count) throws IOException {
                            throw new IOException("execAsync: should not used");
                        }

                        @Override
                        public void write(String str) throws IOException {
                            if (fs != null) {
                                fs.println(str);
                            }
                        }
                    });
                } catch (Exception e) {
                    log("execAsync: exec failed!", e);
                }
                if (fs != null) {
                    try {
                        fs.close();
                    } catch (Exception e) {
                    }
                }
            }
        };
        execThread.setDaemon(true);
        execThread.start();
        return processHandler;
    }

    public int getPid(String pkg, int retries) throws Exception {
        return getPid(pkg, retries, false);
    }

    public int getPid(String pkg, int retries, boolean findExact) throws Exception {
        log("Detecting pid for package: " + pkg + "...");
        for (int i = 1; i <= retries; i++) {
            vlog("Detecting pid #" + i + "...");
            String output = new ProcessHandler(null, "ps", Utils.verbose).exec();
            // log(output);
            for (String line : output.split("\n")) {
                line = line.trim();
                String values[] = line.split("\\s+");
                if (values.length > 0) {
                    String p = values[values.length - 1];
                    if (p.equals(pkg) || (findExact == false && p.startsWith(pkg + ":"))) {
                        int pid = Integer.valueOf(values[1].trim());
                        log("Process: " + pkg + " pid: " + pid);
                        return pid;
                    }
                }
            }
            sleep(1000);
        }
        return -1;
    }

    public String getPackage(int pid) throws Exception {
        vlog("Detecting package from pid " + pid + "...");
        String output = new ProcessHandler(null, "ps", Utils.verbose).exec();
        // log(output);
        for (String line : output.split("\n")) {
            line = line.trim();
            String values[] = line.split("\\s+");
            if (values.length > 0) {
                String p = values[1];
                if (p.equals(Integer.toString(pid))) {
                    String pkg = values[values.length - 1];
                    log("Process pkg found: " + pkg + " for pid: " + pid);
                    int pos = pkg.indexOf(':');
                    if (pos > 0) {
                        pkg = pkg.substring(0, pos);
                    }
                    return pkg;
                }
            }
        }
        log("Process pkg not found for pid: " + pid + "!");
        if (Utils.verbose) {
            log("ps output: ");
            log(output);
        }
        return null;
    }

    public void startApp(String pkg, String activity) throws Exception {
        startApp(pkg, activity, "");
    }

    public void startApp(String pkg, String activity, String extraOpts) throws Exception {
        device.waitForIdle();
        log("Checking activity: " + pkg + "/" + activity);
        if (UiObject(UiSelector().packageName(pkg)).exists() && extraOpts.indexOf("-S") < 0) {
            log("Using existing activity: " + pkg + "/" + activity);
        } else {
            log("Starting activity: " + pkg + "/" + activity + " with extraOpts: " + extraOpts);
            shell(String.format("am start -a android.intent.action.MAIN -n %s/%s " + extraOpts, pkg, activity));
            detectPid(pkg);
        }
    }

    public void detectPid(String pkg) throws Exception {
        pid = getPid(pkg, 10);
    }

    public void startGCProf() {
        startGCProf(pid);
    }

    public void startGCProf(int pid) {
        if (!profiling)
            return;
        if (pid < 10) {
            log("[startGCProf] invalid pid: " + pid);
            return;
        }
        if (profilingStep == 0) {
            shell("rm -r " + gcprofileDir);
        }
        profilingStep++;
        log("[startGCProf] Recreating GC profile dir: " + gcprofileDir + "...");
        File gcDirFile = new File(gcprofileDir);
        gcDirFile.mkdirs();
        gcDirFile.setReadable(true, false);
        gcDirFile.setWritable(true, false);
        gcDirFile.setExecutable(true, false);
        try {
            shell("kill -10 " + pid);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void finishGCProf(String name) {
        finishGCProf(pid, name);
    }

    public void finishGCProf(int pid, String name) {
        if (!profiling)
            return;
        if (pid < 10) {
            log("[finishGCProf] invalid pid: " + pid);
            return;
        }
        try {
            String targetPackageName = getPackage(pid);
            shell("kill -10 " + pid);
            sleep(300);
            // alloc_free_log_6815_0.csv
            File f = new File(gcprofileDir, "alloc_free_log_" + pid + "_0.csv");
            for (int i = 0; i < 10; i++) {
                if (f.exists()) {
                    break;
                }
                sleep(300);
            }
            if (f.exists()) {
                log("[finishGCProf] alloc_free_log file found at " + f + " - renaming...");
                File n = new File(gcprofileDir, "alloc_free_log_" + pid + "_" + name + ".csv");
                shell("mv " + f + " " + n);
            } else {
                log("[finishGCProf] alloc_free_log file not found at " + f);
                f = new File(dataRoot + targetPackageName, "alloc_free_log_" + pid + "_0.csv");
                if (f.exists()) {
                    log("[finishGCProf] alloc_free_log file found at " + f + " - renaming...");
                    File n = new File(dataRoot + targetPackageName, "gcprofile");
                    n.mkdirs();
                    if (n.exists()) {
                        n = new File(n, "alloc_free_log_" + pid + "_" + name + ".csv");
                        shell("mv " + f + " " + n);
                    } else {
                        log("[finishGCProf] alloc_free_log cannot mkdir at " + n);
                    }
                } else {
                    log("[finishGCProf] alloc_free_log file not found at " + f);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void stopLogcat() {
        ProcessHandler logcat = this.logcatHandler;
        this.logcatHandler = null;
        if (logcat == null) {
            log("No Logcat to stop");
            log("");
            return;
        }
        log("Stopping Logcat...");
        logcat.stop();
    }

    public void traceDump() {
        log("traceDump started");
        try {
            Class<?> VMDebugClass = Class.forName("dalvik.system.VMDebug");
            VMDebugClass.getMethod("dumpHprofData", String.class).invoke(null, "$tracedump");
            log("traceDump finished");
        } catch (Exception e) {
            log("traceDump failed", e);
        }
    }

    int dumpID = 0;

    public void dump() {
        dumpID++;
        String dumpName = getScenarioName() + "-" + pid + "-" + dumpID + ".xml";
        dump(dumpName);
    }

    public void dump(String dumpName) {
        log("DUMP START");
        log("ProductName: " + device.getProductName());
        log("CurrentPackageName: " + device.getCurrentPackageName() + "/" + getTestPackage());
        File dumpFile = new File(dumpDir, dumpName);
        BufferedReader r = null;
        try {
            log("Hierarchy dump to " + dumpFile.getAbsolutePath());
            device.dumpWindowHierarchy(dumpName);
            r = new BufferedReader(new InputStreamReader(new FileInputStream(dumpFile)));
            System.setProperty("org.xml.sax.driver", "org.xmlpull.v1.sax2.Driver");
            XMLReader xmlReader = XMLReaderFactory.createXMLReader();
            xmlReader.setContentHandler(new ContentHandler() {
                int rec = 0;
                ArrayList<String> pads = new ArrayList<String>();

                @Override
                public void startElement(String uri, String localName, String qName, Attributes atts) throws SAXException {
                    rec++;
                    while (pads.size() <= rec) {
                        String s = "";
                        for (int i = 0; i < pads.size(); i++) {
                            s += "  ";
                        }
                        pads.add(s);
                    }
                    if (localName.equals("node")) {
                        log(pads.get(rec) + atts.getValue("class") + " id='" + atts.getValue("resource-id") + "' text='" + atts.getValue("text") + "' desc='"
                                + atts.getValue("content-desc") + "' bounds=" + atts.getValue("bounds"));
                    }
                }

                @Override
                public void endElement(String uri, String localName, String qName) throws SAXException {
                    rec--;
                }

                @Override
                public void startDocument() throws SAXException {
                }

                @Override
                public void startPrefixMapping(String prefix, String uri) throws SAXException {
                }

                @Override
                public void skippedEntity(String name) throws SAXException {
                }

                @Override
                public void setDocumentLocator(Locator locator) {
                }

                @Override
                public void processingInstruction(String target, String data) throws SAXException {
                }

                @Override
                public void ignorableWhitespace(char[] ch, int start, int length) throws SAXException {
                }

                @Override
                public void endPrefixMapping(String prefix) throws SAXException {
                }

                @Override
                public void endDocument() throws SAXException {
                }

                @Override
                public void characters(char[] ch, int start, int length) throws SAXException {
                }
            });
            InputSource is = new InputSource(r);
            is.setEncoding("UTF-8");
            xmlReader.parse(is);
        } catch (Exception e) {
            log("Dump load failed: " + e.getMessage());
            log("Trying dump by instance...");
            if (tryDumpByInstance) {
                dumpByInstance();
            }
        } finally {
            try {
                if (r != null)
                    r.close();
            } catch (Exception e) {
            }
            log("DUMP FINISH");
        }
    }

    public void dumpObjEx(UiObject obj, String pad) {
        if (obj == null) {
            log(pad + "not exists");
        } else if (obj.exists()) {
            try {
                log(pad + obj.getClassName() + " text=" + obj.getText() + " package=" + obj.getPackageName() + " bounds=" + obj.getBounds() + " visible="
                        + obj.getVisibleBounds() + " children=" + obj.getChildCount());
            } catch (Exception e) {
                log(pad + "Exception " + e.getMessage());
            }
        } else {
            log(pad + "null");
        }
    }

    public void dumpObjShort(UiObject obj, String pad) throws UiObjectNotFoundException {
        log(pad + obj.getClassName());
    }

    public void dumpObj(UiObject obj, String pad) throws UiObjectNotFoundException {
        log(pad + obj.getClassName() + " text='" + obj.getText() + "'" + " descr='" + obj.getContentDescription() + "'" + " package='" + obj.getPackageName()
                + "'" + " bounds=" + obj.getBounds() + " visible=" + obj.getVisibleBounds() + " children=" + obj.getChildCount());
    }

    public void dumpLayout(UiObject obj, String pad) {
        try {
            dumpObj(obj, pad);
            int cc = obj.getChildCount();
            for (int i = 0; i < cc; i++) {
                UiObject child = obj.getChild(UiSelector().index(i));
                dumpLayout(child, pad + "  ");
            }
        } catch (UiObjectNotFoundException e) {
        }
    }

    public void dumpByInstance() {
        for (int index = 0; index < 1000; index++) {
            UiObject uiObject = UiObject(UiSelector().instance(index));
            if (!uiObject.exists())
                break;
            try {
                dumpObj(uiObject, "BY INSTANCE [" + index + "] - ");
            } catch (UiObjectNotFoundException e) {
                log("BY INSTANCE [" + index + "] - NotFound");
            }
        }
    }

    public void screenshot(String name) {
        getScreenshot(name + ".png");
    }

    public void getScreenshot(String fileName) {
        if (!takeScreenshots) {
            vlog("takeScreenshots = false for " + fileName);
            return;
        }
        screenshotRoot.mkdirs();
        if (!screenshotRoot.exists()) {
            log("mkdirs failed for screenshotRoot: " + screenshotRoot);
        }
        File scrFile = new File(screenshotRoot, fileName);
        vlog("Saving screenshot to '" + scrFile.getAbsolutePath() + "'");
        // exec("screencap " + scrFile.getAbsolutePath());
        boolean res = device.takeScreenshot(scrFile);
        log("Screenshot " + scrFile.getAbsolutePath() + " taken: " + res);
    }

    public UiDevice device() {
        return device;
    }

    public String getTestPackage() {
        return null;
    }

    public Utils utils() {
        return new Utils();
    }
}
