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

package com.intel.profilinginstrumentation;

import android.content.Context;
import android.os.RemoteException;
import android.os.SystemClock;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiDevice;
import android.support.test.uiautomator.UiObject2;
import android.test.InstrumentationTestCase;
import android.util.Log;
import android.view.KeyEvent;

import java.util.List;

import java.lang.reflect.Method;

public class ProfilingInstrumentation extends InstrumentationTestCase {
    private UiDevice mDevice;
    private Context mContext;
    private final static String TAG = "Profiling";
    private static final int SECOND_IN_MILLISECOND = 1000;
    private static final int generating_factor = 4;
    private boolean generating_profile = false;

    public void testProfiling() {
        mDevice = UiDevice.getInstance(getInstrumentation());
        mContext = getInstrumentation().getContext();
        mDevice.pressHome();
        killRecentApps();
        playStoreTest();
        youtubeTest();
        mapsTest();
        gmailTest();
        chromeTest();
        killRecentApps();
    }

    private void playStoreTest() {
        launchANewApp("Play Store");
        threadSleep(2);
        UiObject2 searchBox =
            mDevice.findObject(By.res("com.android.vending:id/search_box_idle_text"));
        if (searchBox != null) {
            click("search box", searchBox, 2);
            UiObject2 searchBox_final = mDevice.findObject(By.clazz("android.widget.EditText"));
            clickAndSetText(searchBox_final, "Angry Birds", 2);
            mDevice.pressEnter();
            threadSleep(2);
            UiObject2 suggestions =
                mDevice.findObject(By.clazz("android.support.v7.widget.RecyclerView")
                                     .res("com.android.vending:id/search_results_list"));
            if (suggestions != null) {
                UiObject2 recycler_view = suggestions.getChildren().get(1);
                assert recycler_view != null;
                UiObject2 first_suggestion = recycler_view.getChildren().get(0);
                assert first_suggestion != null;
                click("first suggestion", first_suggestion, 2);
                UiObject2 install =
                    mDevice.findObject(By.clazz("android.widget.Button")
                                         .res("com.android.vending:id/buy_button"));
                assert install != null;
                click("install", install, 2);
                UiObject2 complete_device_setup =
                        mDevice.findObject(By.clazz("android.widget.TextView")
                                .res("com.android.vending:id/alertTitle")
                                .text("Complete account setup"));
                if (complete_device_setup != null) {
                    UiObject2 cont_alert = mDevice.findObject(By.clazz("android.widget.Button")
                            .text("CONTINUE"));
                    if (cont_alert != null){
                        click("continue", cont_alert, 5);
                        UiObject2 skip =
                            mDevice.findObject(By.clazz("android.widget.TextView")
                                                 .res("com.android.vending:id/not_now_button"));
                        if (skip != null) {
                            click("skip", skip, 5);
                        } else {
                            missing("No skip button");
                        }
                    } else {
                        missing("No continue button");
                    }
                } else {
                    Log.i(TAG, "No device setup (expected)");
                }
                UiObject2 acceptButton =
                    mDevice.findObject(By.clazz("android.widget.Button").text("ACCEPT"));
                assert acceptButton != null;
                click("accept", acceptButton, 5);
            } else {
                missing("Unable to find play store suggestions");
            }
        } else {
            missing("Unable to find search box");
        }
    }

    private void recentApps() {
        try {
            mDevice.pressHome();
            threadSleep(2);
            mDevice.pressRecentApps();
            threadSleep(2);
            mDevice.click(mDevice.getDisplayHeight() / 2,
                          mDevice.getDisplayWidth() / 2);
            threadSleep(2);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private void killRecentApps() {
        Log.i(TAG, "Killing recent apps");
        try {
            mDevice.pressHome();
            threadSleep(2);
            mDevice.pressRecentApps();
            threadSleep(2);
            UiObject2 clear = mDevice.findObject(By.text("CLEAR ALL"));
            if (clear != null) {
                click("clear", clear, 2);
            }
            UiObject2 dismiss = mDevice.findObject(By.descContains("Dismiss"));
            while (dismiss != null) {
                Log.i(TAG, "About to " + dismiss.getContentDescription());
                click("dismiss", dismiss, 1);
                clear = mDevice.findObject(By.text("CLEAR ALL"));
                if (clear != null) {
                    click("clear", clear, 2);
                }
                dismiss = mDevice.findObject(By.descContains("Dismiss"));
            }
        } catch (RemoteException e) {
          e.printStackTrace();
        }
        threadSleep(1);
    }

    private void missing(String message) {
        Log.i(TAG, message);
        // Dump all clickable resources.
        for (UiObject2 obj : mDevice.findObjects(By.clickable(true))) {
            Log.i(TAG, "class: " + obj.getClass() + ", res: " + obj.getResourceName());
        }
    }

    private void launchANewApp(String appName) {
        threadSleep(1);
        Log.i(TAG, "Launch: " + appName);
        mDevice.pressHome();
        threadSleep(2);
        UiObject2 apps = mDevice.findObject(By.descContains("Apps"));
        click("apps", apps, 3);
        // Ensure we are looking at apps, not widgets.
        UiObject2 force_apps =
            mDevice.findObject(By.clazz("android.widget.TextView").text("APPS"));
        assert force_apps != null;
        click("Force to Apps", force_apps, 2);
        UiObject2 appToBeClicked = mDevice.findObject(By.text(appName));
        if (appToBeClicked != null) {
            appToBeClicked.click();
        }
        while (appToBeClicked == null) {
            Log.i(TAG, "launchANewApp: failed to find app");
            swipeLeft();
            force_apps = mDevice.findObject(By.clazz("android.widget.TextView").text("APPS"));
            assert force_apps != null;
            click("Force to Apps(in loop)", force_apps, 2);
            appToBeClicked = mDevice.findObject(By.text(appName));
            if (appToBeClicked != null) {
                Log.i(TAG, "Found app: " + appToBeClicked.getContentDescription());
                appToBeClicked.click();
                break;
            }
        }
        threadSleep(3);
    }

    private void gmailTest() {
        launchANewApp("Gmail");
        threadSleep(5);
        UiObject2 compose = mDevice.findObject(By.descContains("Compose"));
        click("compose", compose, 5);
        UiObject2 to = mDevice.findObject(By.res("com.google.android.gm:id/to"));
        assert to != null;
        click("to", to, 2);
        clickAndSetText(to, "mendell27.test@gmail.com", 2);

        UiObject2 subject = mDevice.findObject(By.text("Subject"));
        clickAndSetText(subject, "Another test of the emergency email sending system :-)", 1);

        UiObject2 message_text = mDevice.findObject(By.text("Compose email"));
        clickAndSetText(message_text, "Hello!", 2);

        UiObject2 sendButton = mDevice.findObject(By.descContains("Send"));
        click("send", sendButton, 10);

        UiObject2 deleteButoon = mDevice.findObject(By.descContains("Delete"));
        click("delete", deleteButoon, 2);
    }

    private void chromeTest() {
        launchANewApp("Chrome");
        // Do we have a  'install accept and continue' popup?
        UiObject2 accept_continue = mDevice.findObject(By.clazz("android.widget.Button").res("com.android.chrome:id/terms_accept"));
        if (accept_continue != null) {
            click("Accept & Continue", accept_continue, 2);
        }
        // Do we have a  'Hi' popup?
        UiObject2 ok_got_it =
            mDevice.findObject(By.clazz("android.widget.Button")
                                 .res("com.android.chrome:id/positive_button"));
        if (ok_got_it != null) {
            click("OK got it", ok_got_it, 2);
        }
        // Open ibm.com.
        UiObject2 searchBox = mDevice.findObject(By.clazz("android.widget.EditText"));
        click("search box", searchBox, 2);
        searchBox = mDevice.findObject(By.clazz("android.widget.EditText"));
        click("search box 2", searchBox, 2);
        searchBox.clear();
        // This one doesn't seem to like setText!
        enterString("www.ibm.com\n", false);
        threadSleep(8);

        // Open recent tabs.
        UiObject2 options2 = mDevice.findObject(By.descContains("More options"));
        click("options 2", options2, 2);
        UiObject2 newTab2 = mDevice.findObject(By.text("Recent tabs"));
        click("new tab 2", newTab2, 2);
        UiObject2 tab =
            mDevice.findObject(By.clazz("android.widget.TextView").clickable(true));
        click("tab", tab, 2);

        // Open tesla.com.
        UiObject2 options = mDevice.findObject(By.descContains("More options"));
        click("options", options, 2);
        UiObject2 newTab = mDevice.findObject(By.text("New tab"));
        click("new tab", newTab, 3);
        UiObject2 searchBox2 = mDevice.findObject(By.clazz("android.widget.EditText"));
        click("search box 2", searchBox2, 2);
        searchBox2 = mDevice.findObject(By.clazz("android.widget.EditText"));
        if (searchBox2 != null) {
            clickAndSetText(searchBox2, "www.tesla.com", 2);
            mDevice.pressEnter();
            threadSleep(25, false);
        }
    }

    private void mapsTest() {
        launchANewApp("Maps");
        Log.i(TAG, "google maps");
        // Do we have a  'Welcome, accept and continue' popup?
        UiObject2 accept_continue = mDevice.findObject(By.clazz("android.widget.TextView").text("ACCEPT & CONTINUE"));
        if (accept_continue != null) {
            click("Accept & Continue", accept_continue, 15);
        }
        UiObject2 side_menu =
            mDevice.findObject(By.clazz("android.widget.Button")
                                 .res("com.google.android.apps.maps:id/tutorial_side_menu_got_it"));
        if (side_menu != null) {
            click("Side menu (GOT IT)", side_menu, 2);
        }
        UiObject2 searchBox = mDevice.findObject(By.clazz("android.widget.EditText"));
        if (searchBox != null) {
            click("search box", searchBox, 5);
            enterString("3600 Juliette Lane Santa Clara CA", false);
            threadSleep(4);
            checkForGotIt();

            UiObject2 loc = mDevice.findObject(By.clazz("android.widget.RelativeLayout"));
            if (loc != null) {
                click("loc", loc, 4);
                for (int i = 0; i < 10; i++) {
                    checkForGotIt();
                    swipeLeft();
                    checkForGotIt();
                    swipeDown();
                }
            } else {
                missing("Unable to find location box");
            }
        } else {
            missing("Unable to find search box");
        }
        threadSleep(2);
        mDevice.pressHome();
    }

    private void checkForGotIt() {
        UiObject2 pull_up = mDevice.findObject(By.clazz("android.widget.Button")
                .res("com.google.android.apps.maps:id/tutorial_pull_up_got_it"));
        if (pull_up != null) {
            click("Pull up (GOT IT)", pull_up, 2);
        }
    }

    private void youtubeTest() {
        launchANewApp("YouTube");
        Log.i(TAG, "youtube");
        threadSleep(20, false);
        // Do we have a  'install new version' popup?
        UiObject2 new_version = mDevice.findObject(By.clazz("android.widget.TextView")
                                                     .res("com.google.android.youtube:id/later_button"));
        if (new_version != null) {
            click("Later", new_version, 2);
        }
        UiObject2 channels = mDevice.findObject(By.descContains("Subscriptions"));
        click("channels", channels, 2);
        UiObject2 video =
          mDevice.findObject(By.res("com.google.android.youtube:id/event_item").clickable(true));
        if (video != null) {
            click("video", video, 5);
        }
        mDevice.pressEnter();
        Log.i(TAG, "pressed back");
        UiObject2 player =
            mDevice.findObject(By.res("com.google.android.youtube:id/player_fragment")
                                 .clickable(true));
        click("player", player, 2);
        UiObject2 minimize = mDevice.findObject(By.descContains("Minimize").clickable(true));
        click("minimize", minimize, 2);
        UiObject2 search =
          mDevice.findObject(By.res("com.google.android.youtube:id/menu_search")
                               .clickable(true));
        click("search", search, 2);
        UiObject2 searchBox = mDevice.findObject(By.clazz("android.widget.EditText"));
        if (searchBox != null) {
            enterString("gangnam style\n", false);
            threadSleep(1);
            mDevice.pressEnter();
            threadSleep(4);
        }

        // Find the right video.
        UiObject2 psy_video =
          mDevice.findObject(By.clazz("android.widget.TextView")
                               .textContains("PSY- Gangnam Style"));
        if (psy_video != null) {
            psy_video.click();
            // We only want some of the video.
            threadSleep(25, false);
        } else {
            missing("Unable to find PSY video");
        }
    }

    private void threadSleep(long seconds) {
        if (generating_profile) {
            seconds *= generating_factor;
        }
        Log.i(TAG, "Sleep " + seconds + " seconds");
        SystemClock.sleep(seconds * SECOND_IN_MILLISECOND);
    }

    private void threadSleep(long seconds, boolean change_for_generate) {
        if (change_for_generate && generating_profile) {
            seconds *= generating_factor;
        }
        Log.i(TAG, "Sleep " + seconds + " seconds");
        SystemClock.sleep(seconds * SECOND_IN_MILLISECOND);
    }

    private void threadSleepMS(long milli_seconds) {
        try {
            Thread.sleep(milli_seconds);
        } catch (InterruptedException e) {
        }
    }


    private void click(String descriptor, UiObject2 obj, long sleep) {
        if (obj == null) {
            missing("missing click object " + descriptor);
            return;
        }
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

    private void swipeLeft() {
        Log.i(TAG, "Swipe left");
        mDevice.swipe(mDevice.getDisplayWidth() / 2,
                      mDevice.getDisplayHeight() / 2,
                      0,
                      mDevice.getDisplayHeight() / 2,
                      10);
        threadSleep(1, false);
    }

    private void swipeDown() {
        Log.i(TAG, "Swipe down");
        mDevice.swipe(mDevice.getDisplayWidth() / 2,
                      mDevice.getDisplayHeight() / 2,
                      mDevice.getDisplayWidth() / 2,
                      mDevice.getDisplayHeight(),
                      10);
        threadSleep(1, false);
    }

    private void swipeUp() {
        Log.i(TAG, "Swipe up");
        mDevice.swipe(mDevice.getDisplayWidth() / 2,
                      mDevice.getDisplayHeight() / 2,
                      mDevice.getDisplayWidth() / 2,
                      0,
                      10);
        threadSleep(1, false);
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
                    case '\n':
                        threadSleepMS(500);
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
