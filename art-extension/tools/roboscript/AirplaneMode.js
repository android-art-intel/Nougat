screenshot("airplane-start")

newValue = getSystemProperty("airplane", "ON")

shell('am start -n com.android.settings/.Settings')
click('More')
elem = byResourceId('android:id/switchWidget')
curValue = elem.getText()
if (curValue != newValue) {
    click('android:id/switchWidget')
    curValue = elem.getText()
    log("Airplane: " + curValue + " (updated)")
} else {
    log("Airplane: " + curValue + " (not changed)")
}

screenshot("airplane-end")
