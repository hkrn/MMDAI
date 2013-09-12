CONFIG(debug, debug|release)   {
  BUILD_TYPE = debug
  CONFIG += qml_debug
}
CONFIG(release, debug|release) {
  BUILD_TYPE = release
}

INCLUDEPATH += $${MMDAI_ROOT_PATH}/bullet-src/build-$${BUILD_TYPE}/install-root/include/bullet \
               $${MMDAI_ROOT_PATH}/assimp-src/build-$${BUILD_TYPE}/install-root/include/assimp \
               $${MMDAI_ROOT_PATH}/icu4c-src/build-$${BUILD_TYPE}/install-root/include \
               $${MMDAI_ROOT_PATH}/openal-soft-src/include \
               $${MMDAI_ROOT_PATH}/alure-src/include \
               $${MMDAI_ROOT_PATH}/glew-src/include \
               $${MMDAI_ROOT_PATH}/gli-src \
               $${MMDAI_ROOT_PATH}/glm-src \
               $${MMDAI_ROOT_PATH}/glog-src/build-$${BUILD_TYPE}/install-root/include \
               $${MMDAI_ROOT_PATH}/libvpvl2/build-$${BUILD_TYPE}/include \
               $${MMDAI_ROOT_PATH}/tbb-src/include \
               $${MMDAI_ROOT_PATH}/libvpvl2/include \
               $${MMDAI_ROOT_PATH}/libgizmo-src/inc \

LIBS += -L$${MMDAI_ROOT_PATH}/bullet-src/build-$${BUILD_TYPE}/install-root/lib \
        -L$${MMDAI_ROOT_PATH}/assimp-src/build-$${BUILD_TYPE}/install-root/lib \
        -L$${MMDAI_ROOT_PATH}/icu4c-src/build-$${BUILD_TYPE}/install-root/lib \
        -L$${MMDAI_ROOT_PATH}/glog-src/build-$${BUILD_TYPE}/install-root/lib \
        -L$${MMDAI_ROOT_PATH}/nvFX-src/build-$${BUILD_TYPE}/install-root/lib \
        -L$${MMDAI_ROOT_PATH}/openal-soft-src/build-$${BUILD_TYPE}/install-root/lib \
        -L$${MMDAI_ROOT_PATH}/alure-src/build-$${BUILD_TYPE}/install-root/lib \
        -L$${MMDAI_ROOT_PATH}/libgizmo-src/build-$${BUILD_TYPE} \
        -L$${MMDAI_ROOT_PATH}/libvpvl2/build-$${BUILD_TYPE}/lib \
        -lvpvl2 -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lgizmo -lalure-static -lopenal -lFxLibGL -lFxLib -lFxParser -licuuc \

CONFIG(debug, debug|release):LIBS += -lassimpD
CONFIG(release, debug|release):LIBS += -lassimp

win32 {
  NVIDIA_CG_PATH  = "C:/Program Files (x86)/NVIDIA Corporation/Cg"
  LIBS           += -L$${NVIDIA_CG_PATH}/lib -lcg -lcggl
  INCLUDEPATH    += $${NVIDIA_CG_PATH}/include
  QMAKE_CFLAGS   += /wd4068 /wd4819
  QMAKE_CXXFLAGS += /wd4068 /wd4819
  INCLUDEPATH    += $${MMDAI_ROOT_PATH}/glog-src/src/windows \
                    $${MMDAI_ROOT_PATH}/icu4c-src/include
  LIBS += -L$${MMDAI_ROOT_PATH}/libvpvl2/build-$${BUILD_TYPE}/lib/$${BUILD_TYPE} \
          -L$${MMDAI_ROOT_PATH}/zlib-src/build-$${BUILD_TYPE}/install-root/lib \
          -L$${MMDAI_ROOT_PATH}/regal-src/build/win32/vs2010/Regal/$${BUILD_TYPE}/win32 \
          -L$${MMDAI_ROOT_PATH}/icu4c-src/lib \
          -L$${MMDAI_ROOT_PATH}/glog-src/$${BUILD_TYPE} \
          -llibglog -licuin -licudt -lregal32
  CONFIG(debug, debug|release):LIBS += -lzlibd
  CONFIG(release, debug|release):LIBS += -lzlib
} else {
  macx:LIBS += -L$${MMDAI_ROOT_PATH}/regal-src/lib/darwin
  linux-*:LIBS+= -L$${MMDAI_ROOT_PATH}/regal-src/lib/linux
  LIBS += -L$${MMDAI_ROOT_PATH}/tbb-src/lib \
          -ltbb -lglog -licui18n -licudata -lz -lRegal
}

macx:LIBS += -F/Library/Frameworks -framework OpenCL

RESOURCES += $${MMDAI_ROOT_PATH}/libvpvl2/src/qt/resources/libvpvl2qtcommon.qrc \
             $${VPVM_ROOT_PATH}/libav/libav.qrc \
             $${VPVM_ROOT_PATH}/licenses/licenses.qrc
CONFIG(release, debug|release) { RESOURCES += $${VPVM_ROOT_PATH}/qml/VPVM.qrc }

VER_MAJ = 0
VER_MIN = 31
VER_PAT = 0
