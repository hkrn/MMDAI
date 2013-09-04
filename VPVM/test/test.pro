cache()

MMDAI_ROOT_PATH = ../..
VPVM_ROOT_PATH = ..
INCLUDEPATH += $${VPVM_ROOT_PATH} $${MMDAI_ROOT_PATH}/libvpvl2/test
include("$${VPVM_ROOT_PATH}/VPVM.pri")

QT       += qml quick widgets testlib

TARGET = tst_VPVMLogicTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_VPVMProjectTest.cc \
    $${VPVM_ROOT_PATH}/ProjectProxy.cc \
    $${VPVM_ROOT_PATH}/ModelProxy.cc \
    $${VPVM_ROOT_PATH}/MotionProxy.cc \
    $${VPVM_ROOT_PATH}/Util.cc \
    $${VPVM_ROOT_PATH}/BoneRefObject.cc \
    $${VPVM_ROOT_PATH}/MorphRefObject.cc \
    $${VPVM_ROOT_PATH}/CameraRefObject.cc \
    $${VPVM_ROOT_PATH}/LabelRefObject.cc \
    $${VPVM_ROOT_PATH}/BoneKeyframeRefObject.cc \
    $${VPVM_ROOT_PATH}/MorphKeyframeRefObject.cc \
    $${VPVM_ROOT_PATH}/BaseKeyframeRefObject.cc \
    $${VPVM_ROOT_PATH}/BoneMotionTrack.cc \
    $${VPVM_ROOT_PATH}/MorphMotionTrack.cc \
    $${VPVM_ROOT_PATH}/BaseMotionTrack.cc \
    $${VPVM_ROOT_PATH}/LightRefObject.cc \
    $${VPVM_ROOT_PATH}/Preference.cc \
    $${VPVM_ROOT_PATH}/CameraKeyframeRefObject.cc \
    $${VPVM_ROOT_PATH}/LightKeyframeRefObject.cc \
    $${VPVM_ROOT_PATH}/CameraMotionTrack.cc \
    $${VPVM_ROOT_PATH}/LightMotionTrack.cc \
    $${VPVM_ROOT_PATH}/WorldProxy.cc

HEADERS += \
    $${VPVM_ROOT_PATH}/ProjectProxy.h \
    $${VPVM_ROOT_PATH}/ModelProxy.h \
    $${VPVM_ROOT_PATH}/MotionProxy.h \
    $${VPVM_ROOT_PATH}/Util.h \
    $${VPVM_ROOT_PATH}/BoneRefObject.h \
    $${VPVM_ROOT_PATH}/MorphRefObject.h \
    $${VPVM_ROOT_PATH}/CameraRefObject.h \
    $${VPVM_ROOT_PATH}/LabelRefObject.h \
    $${VPVM_ROOT_PATH}/BoneKeyframeRefObject.h \
    $${VPVM_ROOT_PATH}/MorphKeyframeRefObject.h \
    $${VPVM_ROOT_PATH}/BaseKeyframeRefObject.h \
    $${VPVM_ROOT_PATH}/BoneMotionTrack.h \
    $${VPVM_ROOT_PATH}/MorphMotionTrack.h \
    $${VPVM_ROOT_PATH}/BaseMotionTrack.h \
    $${VPVM_ROOT_PATH}/LightRefObject.h \
    $${VPVM_ROOT_PATH}/Preference.h \
    $${VPVM_ROOT_PATH}/CameraKeyframeRefObject.h \
    $${VPVM_ROOT_PATH}/LightKeyframeRefObject.h \
    $${VPVM_ROOT_PATH}/CameraMotionTrack.h \
    $${VPVM_ROOT_PATH}/LightMotionTrack.h \
    $${VPVM_ROOT_PATH}/WorldProxy.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"
