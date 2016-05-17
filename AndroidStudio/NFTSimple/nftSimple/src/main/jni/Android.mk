#--------------------------------------------------------------------------
#
#  NFTSimple
#  ARToolKit for Android
#
#  Disclaimer: IMPORTANT:  This Daqri software is supplied to you by Daqri
#  LLC ("Daqri") in consideration of your agreement to the following
#  terms, and your use, installation, modification or redistribution of
#  this Daqri software constitutes acceptance of these terms.  If you do
#  not agree with these terms, please do not use, install, modify or
#  redistribute this Daqri software.
#
#  In consideration of your agreement to abide by the following terms, and
#  subject to these terms, Daqri grants you a personal, non-exclusive
#  license, under Daqri's copyrights in this original Daqri software (the
#  "Daqri Software"), to use, reproduce, modify and redistribute the Daqri
#  Software, with or without modifications, in source and/or binary forms;
#  provided that if you redistribute the Daqri Software in its entirety and
#  without modifications, you must retain this notice and the following
#  text and disclaimers in all such redistributions of the Daqri Software.
#  Neither the name, trademarks, service marks or logos of Daqri LLC may
#  be used to endorse or promote products derived from the Daqri Software
#  without specific prior written permission from Daqri.  Except as
#  expressly stated in this notice, no other rights or licenses, express or
#  implied, are granted by Daqri herein, including but not limited to any
#  patent rights that may be infringed by your derivative works or by other
#  works in which the Daqri Software may be incorporated.
#
#  The Daqri Software is provided by Daqri on an "AS IS" basis.  DAQRI
#  MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
#  THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE, REGARDING THE DAQRI SOFTWARE OR ITS USE AND
#  OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
#
#  IN NO EVENT SHALL DAQRI BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
#  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
#  MODIFICATION AND/OR DISTRIBUTION OF THE DAQRI SOFTWARE, HOWEVER CAUSED
#  AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
#  STRICT LIABILITY OR OTHERWISE, EVEN IF DAQRI HAS BEEN ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#
#  Copyright 2015 Daqri, LLC.
#  Copyright 2011-2015 ARToolworks, Inc.
#
#  Author(s): Philip Lamb
#
#--------------------------------------------------------------------------

MY_LOCAL_PATH := $(call my-dir)
LOCAL_PATH := $(MY_LOCAL_PATH)

STL_PATH=$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a

# Pull ARToolKit into the build
include $(CLEAR_VARS)
ARTOOLKIT_DIR := $(MY_LOCAL_PATH)/../../../../../../android
ARTOOLKIT_LIBDIR := $(call host-path, $(ARTOOLKIT_DIR)/obj/local/$(TARGET_ARCH_ABI))
define add_artoolkit_module
    include $(CLEAR_VARS)
    LOCAL_MODULE:=$1
    LOCAL_SRC_FILES:=lib$1.a
    include $(PREBUILT_STATIC_LIBRARY)
endef
ARTOOLKIT_LIBS := ar2 kpm util eden argsub_es armulti ar aricp jpeg arvideo arosg zlib png
ARTOOLKIT_LIBS += osgdb_osga osgdb_osg osgdb_ive osgdb_jpeg osgdb_gif gif tiff osgdb_bmp osgdb_tga osgdb_freetype ft2 osgAnimation osgFX \
                  osgParticle osgPresentation osgShadow osgSim osgTerrain osgText osgVolume osgWidget osgViewer osgGA osgDB osgUtil \
                  osgdb_deprecated_osg osgdb_deprecated_osganimation osgdb_deprecated_osgfx  osgdb_deprecated_osgparticle \
                  osgdb_deprecated_osgshadow osgdb_deprecated_osgsim osgdb_deprecated_osgterrain osgdb_deprecated_osgtext \
                  osgdb_deprecated_osgviewer osgdb_deprecated_osgvolume osgdb_deprecated_osgwidget osg OpenThreads \
                  osgManipulator osgdb_curl osgdb_lwo osgdb_tiff osgdb_png
LOCAL_PATH := $(ARTOOLKIT_LIBDIR)
$(foreach module,$(ARTOOLKIT_LIBS),$(eval $(call add_artoolkit_module,$(module))))

LOCAL_PATH := $(MY_LOCAL_PATH)

# Android arvideo depends on CURL.
CURL_DIR := $(ARTOOLKIT_DIR)/jni/curl
CURL_LIBDIR := $(call host-path, $(CURL_DIR)/libs/$(TARGET_ARCH_ABI))
define add_curl_module
    include $(CLEAR_VARS)
    LOCAL_MODULE:=$1
    #LOCAL_SRC_FILES:=lib$1.so
    #include $(PREBUILT_SHARED_LIBRARY)
    LOCAL_SRC_FILES:=lib$1.a
    include $(PREBUILT_STATIC_LIBRARY)
endef
#CURL_LIBS := curl ssl crypto
CURL_LIBS := curl
LOCAL_PATH := $(CURL_LIBDIR)
$(foreach module,$(CURL_LIBS),$(eval $(call add_curl_module,$(module))))

LOCAL_PATH := $(MY_LOCAL_PATH)
include $(CLEAR_VARS)

# ARToolKit libs use lots of floating point, so don't compile in thumb mode.
LOCAL_ARM_MODE := arm

LOCAL_PATH := $(MY_LOCAL_PATH)
LOCAL_MODULE := NFTSimpleNative
LOCAL_SRC_FILES := NFTSimpleNative.cpp \
                   ../../../../../../examples/nftSimple/ARMarkerNFT.cpp \
                   ../../../../../../examples/nftSimple/TrackingSub.cpp \
                   ../../../../../../examples/nftSimple/NFTSimple.cpp \
                   ../../../../../../examples/nftSimple/VirtualEnvironment.cpp

# Make sure DEBUG is defined for debug builds. (NDK already defines NDEBUG for release builds.)
ifeq ($(APP_OPTIM),debug)
    LOCAL_CPPFLAGS += -DDEBUG
endif

LOCAL_C_INCLUDES += $(ARTOOLKIT_DIR)/../include/android \
                    $(ARTOOLKIT_DIR)/../include \
                    $(ARTOOLKIT_DIR)/../examples/nftSimple

#LOCAL_LDLIBS +=  -lgnustl_static
#LOCAL_LDLIBS += -L$(STL_PATH)  -lsupc++
LOCAL_LDLIBS += -L$(STL_PATH) -lgnustl_static -lsupc++
LOCAL_LDLIBS += -llog -lGLESv1_CM -lz -lGLESv2
LOCAL_WHOLE_STATIC_LIBRARIES += ar
LOCAL_STATIC_LIBRARIES += ar2 arosg kpm util eden argsub_es armulti aricp cpufeatures jpeg arvideo zlib png
LOCAL_STATIC_LIBRARIES += osgdb_osga osgdb_osg osgdb_ive osgdb_jpeg osgdb_gif gif tiff osgdb_bmp osgdb_tga osgdb_freetype ft2 osgAnimation osgFX \
                          osgParticle osgPresentation osgShadow osgSim osgTerrain osgText osgVolume osgWidget osgViewer osgGA osgDB osgUtil \
                          osgdb_deprecated_osg osgdb_deprecated_osganimation osgdb_deprecated_osgfx  osgdb_deprecated_osgparticle \
                          osgdb_deprecated_osgshadow osgdb_deprecated_osgsim osgdb_deprecated_osgterrain osgdb_deprecated_osgtext \
                          osgdb_deprecated_osgviewer osgdb_deprecated_osgvolume osgdb_deprecated_osgwidget osg OpenThreads \
                          osgManipulator osgdb_curl osgdb_lwo osgdb_tiff osgdb_png
#benet-delete: osgdb_tiff osgdb_png
#LOCAL_SHARED_LIBRARIES += $(CURL_LIBS)
LOCAL_STATIC_LIBRARIES += $(CURL_LIBS)

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)

