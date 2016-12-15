#!/bin/bash

BASE_DIR=$(cd $(dirname $0); pwd)

fail() {
    echo "$@"
    exit 1
}

log() {
    echo "$@"
}

# $1 - src dir
# $2 - bin dir
# $3 - lib dir
compile_test() {
    local srcdir=$1
    local bindir=$2
    local libdir=$3

    log "Cleaning $bindir ..."
    rm -fr $bindir/*
    mkdir -p $bindir/classes || fail
    log "Generating file list ..."
    files=$bindir/files
    find $srcdir -iname *.java > $files || fail

    log "Compiling java ..."
    local cp="$ANDROID_JAR"
    for p in $(ls $libdir)
    do
        log "  added to classpath: $libdir/$p"
        cp="$cp:$libdir/$p"
    done
    javac -source 1.6 -target 1.6 -d $bindir/classes -cp "$cp" @$files || fail
    log "Compilation done."
}

# $1 - bin dir
dex_test() {
    local bindir=$1
    local libdir=$2
    log "Dexing ..."
    local libs=""
    for p in $(ls $libdir | grep rhino)
    do
        log "  included in dex file: $libdir/$p"
        libs="$libs $libdir/$p"
    done
    dx --dex --debug --output=$bindir/classes.dex $bindir/classes $libs || fail
    dexdump -d $bindir/classes.dex > $bindir/dexdump.log
    log " $bindir/classes.dex created."
    log "Dex done."
}

# $1 - bin dir
# $2 - package name
pack_test() {
    local bindir=$1
    local name=$2
    log "Packaging ..."
    rm -f "$name.apk"
    aapt package -v -f -I ${ANDROID_JAR} -M AndroidManifest.xml -F $name.apk || fail
    aapt add -k $name.apk $bindir/classes.dex || fail
    jarsigner -keystore debug.keystore -storepass android $name.apk androiddebugkey || fail
    log "Packaging done."
}

get_deps() {
    mkdir -p "$BASE_DIR/lib" || fail
    cd "$BASE_DIR/lib" || fail
    [[ -f rhino-1.7.7.1.jar ]] || wget http://central.maven.org/maven2/org/mozilla/rhino/1.7.7.1/rhino-1.7.7.1.jar || fail "Failed to get Rhino!"
    [[ -f uiautomator-4.4.2_r4.jar ]] || wget http://repo.boundlessgeo.com/main/android/test/uiautomator/uiautomator/4.4.2_r4/uiautomator-4.4.2_r4.jar || fail "Failed to get UIAutomator"
    cd "$BASE_DIR"
}

dx --version || fail "Failed to run dx!"
aapt v || fail "Failed to run aapt!"

ANDROID_SDK_ROOT=$(dirname $(dirname $(which dx)))
ANDROID_JAR=$ANDROID_SDK_ROOT/platforms/android-22/android.jar

[[ -f "$ANDROID_JAR" ]] || fail "Required $ANDROID_JAR not found!"

get_deps
compile_test src/main/java bin lib
dex_test bin lib
pack_test bin RoboScript
