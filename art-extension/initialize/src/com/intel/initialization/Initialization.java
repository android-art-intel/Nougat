/*
 * Copyright (C) 2015 Intel Corporation
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

package com.intel.initialization;

import android.content.Context;
import android.os.RemoteException;
import android.os.SystemClock;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiDevice;
import android.support.test.uiautomator.UiObject2;
import android.support.test.uiautomator.Until;
import android.test.InstrumentationTestCase;
import android.util.Log;
import android.view.KeyEvent;

import java.lang.reflect.Method;
import java.util.List;

public class Initialization extends InstrumentationTestCase {
    private UiDevice mDevice;
    private Context mContext;
    private final static String TAG = "Initialization";
    private final static String WIFI = "EmployeeHotspot";
    private static final int SECOND_IN_MILLISECOND = 1000;

    public void testInitialization() {
        mDevice = UiDevice.getInstance(getInstrumentation());
        mContext = getInstrumentation().getContext();
        ensureScreenOn();
        welcomeScreen();
        setUpYourDevice();
        networkConnection(WIFI);
        configureNetwork();
        waitUntilVerifyAccount();
        enterEmail();
        enterPassword();
        acceptSigningIn();
        googleServices();
        protectYourTablet();
        anythingElse();
        gotIt();
    }

    private void ensureScreenOn() {
        try {
            if (!mDevice.isScreenOn()) {
                mDevice.wakeUp();
                SystemClock.sleep(250);
            }
        } catch (RemoteException e) {
        }
    }

    private void welcomeScreen() {
        UiObject2 get_started =
            mDevice.findObject(By.clazz("android.widget.Button")
                                 .res("com.google.android.setupwizard:id/start"));
        if (get_started != null) {
            click("get started button", get_started, 2);
        } else {
            Log.i(TAG, "Unable to find get started button");
        }
    }

    private void setUpYourDevice() {
        UiObject2 flow_choice =
            mDevice.findObject(By.clazz("android.widget.LinearLayout")
                                 .res("com.google.android.setupwizard:id/flow_choice_new"));
        if (flow_choice == null) {
            Log.i(TAG, "Unable to find set up as new choice LinearList");
            return;
        }
        click("Set up as new choice", flow_choice, 8);

        // Look for all the networks.
        UiObject2 see_all =
            mDevice.findObject(By.clazz("android.widget.LinearLayout")
                                 .res("com.google.android.setupwizard:id/network_see_all_wifi"));
        if (see_all == null) {
            Log.i(TAG, "Unable to find See all Wi-Fi networks LinearList");
            return;
        }
        click("See all networks", see_all, 8);
    }

    private void networkConnection(String network) {
        UiObject2 net =
            mDevice.findObject(By.clazz("android.widget.TextView").text(network));
        if (net != null) {
            // Move up to parent
            click("Network: " + network, net.getParent(), 8);
        } else {
            Log.i(TAG, "Unable to find Network" + network);
        }
    }

    private void configureNetwork() {
        UiObject2 phase2 =
            mDevice.findObject(By.clazz("android.widget.Spinner")
                                 .res("com.android.settings:id/phase2"));
        if (phase2 == null) {
            Log.i(TAG, "Unable to find phase2 as Spinner");
            return;
        }
        phase2.clickAndWait(Until.newWindow(), 2000);
        threadSleep(1);
        UiObject2 phase2_list = mDevice.findObject(By.clazz("android.widget.ListView"));
        if (phase2_list == null) {
            Log.i(TAG, "Unable to find phase2 ListView");
            return;
        }
        click("MSCHAPV2", phase2_list.getChildren().get(1), 2);
        // Now the CA certificate
        UiObject2 ca_cert =
            mDevice.findObject(By.clazz("android.widget.Spinner")
                                 .res("com.android.settings:id/ca_cert"));
        if (ca_cert == null) {
            return;
        }
        ca_cert.clickAndWait(Until.newWindow(), 2000);
        threadSleep(1);
        UiObject2 ca_cert_list = mDevice.findObject(By.clazz("android.widget.ListView"));
        if (ca_cert_list == null) {
            Log.i(TAG, "Unable to find ca_cert find ListView");
            return;
        }
        click("Do not validate", ca_cert_list.getChildren().get(2), 2);

        UiObject2 identity =
            mDevice.findObject(By.clazz("android.widget.EditText")
                                 .res("com.android.settings:id/identity"));
        if (identity == null) {
            Log.i(TAG, "Unable to find identify EditText");
            return;
        }
        clickAndSetText(identity, "hs_11492374_2", 2);
        mDevice.pressEnter();
        mDevice.pressEnter();

        UiObject2 password =
            mDevice.findObject(By.clazz("android.widget.EditText")
                                 .res("com.android.settings:id/password"));
        if (password == null) {
            Log.i(TAG, "Unable to find password EditText");
            return;
        }
        clickAndSetText(password, "snafu000", 2);

        UiObject2 connect =
            mDevice.findObject(By.clazz("android.widget.Button")
                                 .res("android:id/button1"));
        if (connect == null) {
            Log.i(TAG, "Unable to find connect button1");
            return;
        }
        click("Connect", connect, 2);
    }

    private void waitUntilVerifyAccount() {
        // We need to wait until the "Verify your account screen is presented.  Shouldn't take too long.
        UiObject2 verify =
            mDevice.findObject(By.clazz("android.view.View").desc("Verify your account"));
        int max_checks = 40;
        while (verify == null && max_checks > 0) {
            // We are waiting for the screen to show up.
            Log.i(TAG, "Didn't match 'Verify your account'");
            checkForAsynchronousPopups();
            threadSleep(2);
            verify = mDevice.findObject(By.clazz("android.view.View").desc("Verify your account"));
            max_checks--;
        }
        threadSleep(1);
        if (verify == null) {
            Log.i(TAG, "Unable to find Verify your account; continuing anyways");
        }
    }

    private void checkForAsynchronousPopups() {
        final int max_unseen_count = 2;
        int unseen_count = 0;
        while (unseen_count != max_unseen_count) {
            unseen_count = 0;
            List<UiObject2> vending = mDevice.findObjects(By.pkg("com.android.vending"));
            if (vending != null) {
                boolean found = false;
                for (int i = 0; i < vending.size(); i++) {
                    UiObject2 u = vending.get(i);
                    String res = u.getResourceName();
                    if (res != null && res.contains("negative_button")) {
                        click("decline button", u, 2);
                        found = true;
                    }
                }
                if (!found) {
                    unseen_count++;
                }
            } else {
                unseen_count++;
            }

            UiObject2 yes =
                mDevice.findObject(By.clazz("com.google.android.gms", "android.widget.Button")
                                     .res("android:id/button1"));
            if (yes != null) {
                click("YES", yes, 5);
            } else {
                unseen_count++;
            }
        }

    }

    private void enterEmail() {
        checkForAsynchronousPopups();
        UiObject2 email =
            mDevice.findObject(By.clazz("android.widget.EditText")
                                 .res("identifierId"));
        if (email != null) {
            click("email text", email, 1);
            email = mDevice.findObject(By.clazz("android.widget.EditText")
                                         .res("identifierId"));
            if (email != null) {
                email.clear();
            }
            enterString("mendell27.test@gmail.com\n", false);
            threadSleep(2);
        } else {
            Log.i(TAG, "Unable to find email text widget");
        }
    }

    private void enterPassword() {
        checkForAsynchronousPopups();
        UiObject2 password =
            mDevice.findObject(By.clazz("android.widget.EditText")
                                 .res("password"));
        if (password != null) {
            click("password text", password, 1);
            password = mDevice.findObject(By.clazz("android.widget.EditText").res("password"));
            if (password != null) {
                password.clear();
            }
            enterString("snafu000\n", false);
            threadSleep(8);
        } else {
            Log.i(TAG, "Unable to find password text widget");
        }
    }

    private void acceptSigningIn() {
        checkForAsynchronousPopups();
        UiObject2 accept = mDevice.findObject(By.clazz("android.widget.Button").desc("ACCEPT"));
        if (accept != null) {
            click("ACCEPT", accept, 5);
        } else {
            Log.i(TAG, "Unable to find ACCEPT button");
        }
    }

    private void googleServices() {
        checkForAsynchronousPopups();
        swipeUp();
        UiObject2 next =
            mDevice.findObject(By.clazz("android.widget.Button")
                                 .res("com.google.android.gms:id/suw_navbar_next"));
        if (next != null) {
            click("NEXT", next, 10);
        } else {
            Log.i(TAG, "Unable to find NEXT button");
        }
    }

    private void protectYourTablet() {
        checkForAsynchronousPopups();
        UiObject2 not_now =
            mDevice.findObject(By.clazz("android.widget.TextView").text("Not now"));
        if (not_now == null) {
            Log.i(TAG, "Unable to find 'Not now'");
            return;
        }
        // Move up to parent, which is clickable.
        click("'Not now'", not_now.getParent(), 2);

        // Handle pop-up.
        List<UiObject2> settings = mDevice.findObjects(By.pkg("com.android.settings"));
        if (settings != null) {
            for (int i = 0; i < settings.size(); i++) {
                UiObject2 u = settings.get(i);
                Log.i(TAG, i + " class: " + u.getClassName() + " res: " + u.getResourceName());
            }
        }
        UiObject2 skip_anyway =
            mDevice.findObject(By.clazz("android.widget.Button").text("SKIP ANYWAY"));
        if (skip_anyway == null) {
            Log.i(TAG, "Unable to find 'Skip anyway'");
            return;
        }
        click("'Skip anywys'", skip_anyway, 8);
    }

    private void anythingElse() {
        checkForAsynchronousPopups();
        UiObject2 set_up_later =
            mDevice.findObject(By.clazz("android.widget.LinearLayout")
                                 .res("com.google.android.setupwizard:id/suggested_actions_done"));
        if (set_up_later != null) {
            click("'Set up later'", set_up_later, 3);
        } else {
            Log.i(TAG, "Unable to find 'Set up later' layout");
        }
    }

    private void gotIt() {
        checkForAsynchronousPopups();
        UiObject2 ok = mDevice.findObject(By.clazz("android.widget.Button").text("OK"));
        if (ok != null) {
            click("OKAY", ok, 3);
        } else {
            Log.i(TAG, "Unable to find 'OKAY' button");
        }

    }

    private void threadSleep(long seconds) {
        Log.i(TAG, "Sleep " + seconds + " seconds");
        SystemClock.sleep(seconds * SECOND_IN_MILLISECOND);
    }

    private void click(String descriptor, UiObject2 obj, long sleep) {
        while (obj != null && !obj.isClickable()) {
            Log.e(TAG, descriptor + " is not clickable, class = " +
                       obj.getClassName() + " res = " + obj.getResourceName());
            obj = obj.getParent();
        }
        if (obj != null) {
            Log.i(TAG, "click on: " + descriptor);
            obj.click();
            threadSleep(sleep);
        } else {
            Log.i(TAG, "click: object was null: " + descriptor);
        }
    }

    private void swipeUp() {
        Log.i(TAG, "Swipe up");
        mDevice.swipe(mDevice.getDisplayWidth() / 2,
                      mDevice.getDisplayHeight() / 2,
                      mDevice.getDisplayWidth() / 2,
                      0,
                      10);
        threadSleep(1);
    }

    private void clickAndSetText(UiObject2 obj, String text, int seconds) {
        if (obj != null) {
            Log.i(TAG, "Click and set Text to: " + text);
            click("obj", obj, 2);
            obj.setText(text);
            threadSleep(seconds);
        } else {
            Log.i(TAG, "clickAndSetText: object was null, text was: " + text);
        }
    }

    private void enterString(String s, boolean is_search) {
        Log.i(TAG, "About to enter string: " + s);
        for (char ch : s.toCharArray()) {
            if (ch >= 'a' && ch <= 'z') {
                mDevice.pressKeyCode(ch - 'a' + KeyEvent.KEYCODE_A);
            } else if (ch >= 'A' && ch <= 'Z') {
                mDevice.pressKeyCode(ch - 'A' + KeyEvent.KEYCODE_A,
                                     KeyEvent.META_SHIFT_LEFT_ON);
            } else if (ch >= '0' && ch <= '9') {
                mDevice.pressKeyCode(ch - '0' + KeyEvent.KEYCODE_0);
            } else {
                switch (ch) {
                    case ' ':
                        mDevice.pressKeyCode(KeyEvent.KEYCODE_SPACE);
                        break;
                    case '@':
                        mDevice.pressKeyCode(KeyEvent.KEYCODE_AT);
                        break;
                    case '\n':
                        threadSleep(1);
                        mDevice.pressKeyCode(
                            is_search ? KeyEvent.KEYCODE_SEARCH
                                      : KeyEvent.KEYCODE_ENTER);
                        break;
                    case '.':
                        mDevice.pressKeyCode(KeyEvent.KEYCODE_PERIOD);
                        break;
                    default:
                        Log.e(TAG, "Unknown char to enter: '" + ch + "'");
                        break;
                }
            }
        }
    }
}
