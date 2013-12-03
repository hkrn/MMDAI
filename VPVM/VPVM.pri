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
        -lvpvl2 -lBulletDynamics -lBulletSoftBody -lBulletCollision -lLinearMath -lgizmo

CONFIG(debug, debug|release):LIBS += -lassimpD
CONFIG(release, debug|release):LIBS += -lassimp

#CONFIG(debug, debug|release):LIBS += -lFxParserD -lFxLibGLD -lFxLibD
#CONFIG(release, debug|release):LIBS += -lFxParser -lFxLibGL -lFxLib

CONFIG(debug, debug|release):LIBS += -lFxParser64D -lFxLibGL64D -lFxLib64D
CONFIG(release, debug|release):LIBS += -lFxParser64 -lFxLibGL64 -lFxLib64

win32 {
  QMAKE_CFLAGS   += /wd4068 /wd4819
  QMAKE_CXXFLAGS += /wd4068 /wd4819
  INCLUDEPATH    += $${MMDAI_ROOT_PATH}/glog-src/src/windows \
                    $${MMDAI_ROOT_PATH}/icu4c-src/include \
　　　　               $${MMDAI_ROOT_PATH}/openal-soft-src/include/AL
  LIBS += -L$${MMDAI_ROOT_PATH}/libvpvl2/build-$${BUILD_TYPE}/lib/$${BUILD_TYPE} \
          -L$${MMDAI_ROOT_PATH}/zlib-src/build-$${BUILD_TYPE}/install-root/lib \
          -L$${MMDAI_ROOT_PATH}/regal-src/build/win32/vs2010/Regal/$${BUILD_TYPE}/win32 \
          -L$${MMDAI_ROOT_PATH}/icu4c-src/lib \
          -L$${MMDAI_ROOT_PATH}/glog-src/$${BUILD_TYPE} \
          -lALURE32-static -lOpenAL32 -llibglog -licuin -licuuc -licudt -lregal32
  CONFIG(debug, debug|release):LIBS += -lzlibd
  CONFIG(release, debug|release):LIBS += -lzlib
} else {
  macx:LIBS += -L$${MMDAI_ROOT_PATH}/regal-src/lib/darwin
  linux-*:LIBS+= -L$${MMDAI_ROOT_PATH}/regal-src/lib/linux
  LIBS += -L$${MMDAI_ROOT_PATH}/tbb-src/lib \
          -lalure-static -lopenal -ltbb -lglog -licui18n -licuuc -licudata -lz # -lRegal
}

macx {
  LIBS += -framework OpenCL
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

RESOURCES += $${MMDAI_ROOT_PATH}/libvpvl2/src/resources/resources.qrc \
             $${VPVM_ROOT_PATH}/licenses/licenses.qrc

!win32 {
  RESOURCES += $${VPVM_ROOT_PATH}/libav/libav.qrc
}

linux {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/lib
  VPVM_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${VPVM_RPATH}\'
  QMAKE_RPATHDIR =
  gst.path = /lib
  gst.files = $$[QT_INSTALL_LIBS]/libqgsttools_p.so.1.0
  iculib.path = /lib
  iculib.files = $$[QT_INSTALL_LIBS]/libicu*.so.51
  qtlib.path = /lib
  qtlib.files = $$[QT_INSTALL_LIBS]/libQt*.so.5
  qtplugins.path = /plugins
  pqtlugins.files = $$[QT_INSTALL_PLUGINS]/*
  INSTALLS += gst iculib qtlib qtplugins
}

CONFIG(release, debug|release) { RESOURCES += $${VPVM_ROOT_PATH}/qml/VPVM.qrc }

VER_MAJ = 0
VER_MIN = 31
VER_PAT = 0
