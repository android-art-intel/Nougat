#
# Copyright (C) 2015 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Define whether you want to do a debug or a release build
BUILD_TYPE ?= debug

# Define for which all configurations you want to build. Only these directories
# along with defaults would get included in apk file. For example with CONFIG
# as ldpi,hdpi only res/drawable, res/drawable-ldpi and res/drawable-hdpi would
# would get included. Other directories like res/drawable-xlarge would not be
# included in apk. Same holds for res/layout and other directories in res
# Could not find a way to specify this in ant build system
CONFIG ?= en

# Specify the target platform. apk would include library from this platform
ANDROID_TARGET ?= 19

# Specify the package with which Manifest would be built. This way you can build
# for free/paid version by just changing this variable, without a need to
# refactor your code with a differenent package name. This option is not
# support in ant build system.
DEPLOY_TYPE ?= free
OPKG ?= com.intel.profilinginstrumentation
ifeq ($(DEPLOY_TYPE), free)
PKG := $(OPKG)
else
ifeq ($(DEPLOY_TYPE), paid)
# You can choose your own convention for paid application
PKG := $(OPKG).paid
endif
endif
PKG_PATH := $(subst .,/,$(OPKG))
VERSION_CODE := 1
VERSION_NAME := 0.1

# Specify a base target name. $(TARGET).apk would be finally built
TARGET := bin/profilinginstrumentation-$(BUILD_TYPE)-$(DEPLOY_TYPE)

# RES_APK_FILE: Apk file for resources
RES_APK_FILE := $(TARGET).ap_
# CLASS_GEN_PATH: This is the place where .class file from .java files
# would be generated using javac complier
CLASS_GEN_PATH := bin/classes
# DEX file
DEX_FILE := $(TARGET).dex
# Since all the java sources needs to be specified together for javac complier
# As a convience a symbolic representation of generated target from java
# sources us kept as timestamp file
SOURCE_TS := bin/timestamp.ts

# Replace SDK_DIR to correct SDK Directory in your system
SDK_DIR := $(ANDROID_BUILD_TOP)/prebuilts/sdk/19
ANDROID_FRAMEWORK := $(SDK_DIR)/android.jar

ifeq ($(BUILD_TYPE), debug)
KEYSTORE := $(HOME)/.android/debug.keystore
PASSWORD := android
ALIAS := androiddebugkey
else
# KEYSTORE, PASSWORD and ALIAS for Release build
KEYSTORE :=
PASSWORD :=
ALIAS :=
endif

# aapt tool for packaging resources and manifest files. This is Android tool
# Add $(SDK_DIR)/tools in your PATH env
AAPT := aapt
AAPT_FLAGS := package -f -m --auto-add-overlay
AAPT_MANIFEST := -M AndroidManifest.xml
# Include modules if there are modules add those -S <res directories> below
AAPT_INCLUDES := -I $(ANDROID_FRAMEWORK)
AAPT_APK_FILE := -F $(RES_APK_FILE)
AAPT_PKG_NAME := --rename-manifest-package $(PKG)
AAPT_VERSION := --version-code $(VERSION_CODE) --version-name $(VERSION_NAME)
AAPT_CONFIG := -c $(CONFIG)

# Automator library
LIBS := $(ANDROID_BUILD_TOP)/prebuilts/misc/common/ub-uiautomator/ub-uiautomator.jar
LIBS_CLASS_PATHS := $(LIBS)

# Java complier and flags. Add javac in your path. Ensure that it is Sun javac
JAVAC := /usr/lib/jvm/java-7-openjdk-amd64/bin/javac
JAVAC_FLAGS := -Xlint:unchecked
JAVAC_CLASSPATH := -cp $(ANDROID_FRAMEWORK):$(LIBS_CLASS_PATHS)

# dx tool is part of android sdk. Include $(SDK_DIR)/tools in your PATH
DEX := dx
DEX_FLAGS := --dex

# apkbuilder packages all the stuffs together. there is a warning that this
# tool is depreciated. Currently can be ignored. In future can write  main
# around ApkBuilder java class. This is TODO
APK_BUILDER := java -cp $(ANDROID_BUILD_TOP)/prebuilts/devtools/tools/lib/sdklib.jar com.android.sdklib.build.ApkBuilderMain
APK_BUILDER_FLAGS := -u

# jarsigner is used to sign the apk with keystore. This is available in standard
# java package
JAR_SIGNER := jarsigner
JAR_SIGNER_FLAGS :=
JAR_SIGNER_KEYSTORE := -keystore $(KEYSTORE)
JAR_SIGNER_PASSWORD := -storepass $(PASSWORD)
JAR_SIGNER_ALIAS := $(ALIAS)

# zipalign aligns the apk package. This is android tool and is present in
# $(SDK_DIR)/tools directory
ZIP_ALIGN := zipalign
ZIP_ALIGN_FLAGS := -f 4

RM := rm
RM_FLAGS := -rf
ZIP := zip
ZIP_FLAGS := -r

# Java sources
SOURCE_PATH := src/com/intel/profilinginstrumentation
SOURCES := \
	ProfilingInstrumentation.java
SOURCES := ${SOURCES:%=$(SOURCE_PATH)/%}

# TARGET APK
TARGET_UNSIGNED_APK := $(TARGET)-unsigned.apk
TARGET_APK := $(TARGET).apk

all: create_dirs $(TARGET_APK)

.PHONY: $(R_JAVA_FILE) $(RES_APK_FILE) create_dirs

# Create target directories for files to be generated
create_dirs:
	@mkdir -p $(CLASS_GEN_PATH)

# Generate R.java and resources apk package files
$(R_JAVA_FILE) $(RES_APK_FILE):
	$(AAPT) $(AAPT_FLAGS) \
            $(AAPT_MANIFEST) \
            $(AAPT_RES_DIRS) \
            $(AAPT_R_FILE) \
            $(AAPT_INCLUDES) \
            $(AAPT_APK_FILE) \
            $(AAPT_PKG_NAME) \
            $(AAPT_VERSION) \
            $(AAPT_CONFIG)

# Generate .class files from .java files using java compiler
$(SOURCE_TS): $(SOURCES) $(LIBS_CLASS_PATH)
	$(JAVAC) $(JAVAC_FLAGS) \
             $(JAVAC_CLASSPATH) \
             -d $(CLASS_GEN_PATH) \
             $(SOURCES) $(R_JAVA_FILE)
	@echo $(shell date) > $@

# Generate dex file from java .class files
$(DEX_FILE): $(SOURCE_TS) $(LIBS)
	$(DEX) $(DEX_FLAGS) --output $(DEX_FILE) $(CLASS_GEN_PATH) $(LIBS)

# Package Resouces, dex file and asset files into a apk.
$(TARGET_UNSIGNED_APK): $(RES_APK_FILE) $(DEX_FILE)
	$(APK_BUILDER) $@ $(APK_BUILDER_FLAGS) -z $(RES_APK_FILE) -f $(DEX_FILE) \

# Build final APK from unsigned APK
# sign it using jarsigner and do a zipalign
$(TARGET_APK): $(TARGET_UNSIGNED_APK)
	$(JAR_SIGNER) $(JAR_SIGNER_FLAGS) \
                  $(JAR_SIGNER_KEYSTORE) \
                  $(JAR_SIGNER_PASSWORD) \
                  $^ \
                  $(JAR_SIGNER_ALIAS)
	$(ZIP_ALIGN) $(ZIP_ALIGN_FLAGS) $^ $@

clean:
	$(RM) $(RM_FLAGS) bin/* gen/*
