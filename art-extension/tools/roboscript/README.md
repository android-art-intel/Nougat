# RoboScript for Android

RoboScript for Android is a UI automation framework based on UIAutomator and Mozilla's script engine Rhino. It supports scripting capabilities: the automation scenario can be written on JavaScript.  

### Build

System requirements: Linux

Required tools in the PATH: bash, wget, JDK 1.6+ (javac, jarsigner), Android SDK (dx, dexdump, aapt)

Run following script to make *RoboScript.apk*:
 
    $ ./build.sh

Optionally setup HTTP proxy for wget, See wget manual for details:
    [https://www.gnu.org/software/wget/manual/wget.html#Proxies]

### Run 

Run following script with automation defined in the JavaScript file:

    $ ./roboscript <options> <JS-script-file>
    Options:
      --log-source | -l
        Output combined JS source 
      --dump-params | -p
        Output general scenario and framework parameters 
      --take-screenshots | -s
        Allow screenshot operations
      --verbose | -v
        Output more details
      --dump-on-error | -e
        Output UI tree on scenario failure 
      --inline | -i
        Execute JS commands directly without source script

Example, switch airplane mode ON:

    $ ./roboscript -s AirplaneMode.js

Example, run script commands dump and log directly:

    $ ./roboscript -i 'dump(); log("Done!")' 

### Automated scenario 

The automate scenario script is a simple JS-based script which defines the steps and actions which should be applied on Android device. It includes the input actions such as click and text type on the UI elements specified by selectors as well as operations like shell execs, getting screenshots, etc. 

Example, AirplaneMode.js:

    // take first screenshot
    screenshot("airplane-start")
    newValue = getSystemProperty("airplane", "ON")
    // start Settings activity
    shell('am start -n com.android.settings/.Settings')
    // click button: More
    click('More')
    // access UI switch element
    elem = byResourceId('android:id/switchWidget')
    curValue = elem.getText()
    // check the current element text value
    if (curValue != newValue) {
        click('android:id/switchWidget')
        curValue = elem.getText()
        log("Airplane: " + curValue + " (updated)")
    } else {
        log("Airplane: " + curValue + " (not changed)")
    }
    // take the last screenshot
    screenshot("airplane-end")
    