/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

import QtQuick 2.2
import com.github.mmdai.VPVM 1.0 as VPVM

Grid {
    id: transformHandleSet
    readonly property string fontFamilyName: "FontAwesome"
    property int iconPointSize : 48
    signal axisTypeSet(int value)
    signal beginTranslate(real delta)
    signal translate(real delta)
    signal endTranslate()
    signal beginRotate(real delta)
    signal rotate(real delta)
    signal endRotate()
    columns: 3
    rows: 2
    function toggle(bone) {
        if (bone) {
            var movableStateString = bone.movable ? "enabled" : "disabled"
            translateX.state = translateY.state = translateZ.state = movableStateString
            var rotateableStateString = bone.rotateable ? "enabled" : "disabled"
            rotateX.state = rotateY.state = rotateZ.state = rotateableStateString
        }
        else {
            translateX.state = translateY.state = translateZ.state = "disabled"
            rotateX.state = rotateY.state = rotateZ.state = "disabled"
        }
    }
    TranslationButton {
        id: translateX
        font { family: transformHandleSet.fontFamilyName; pointSize: transformHandleSet.iconPointSize; }
        axisType: VPVM.Model.AxisX
        axisColor: "red"
        onAxisTypeSet: transformHandleSet.axisTypeSet(value)
        onBeginTranslate: transformHandleSet.beginTranslate(delta)
        onTranslate: transformHandleSet.translate(delta)
        onEndTranslate: transformHandleSet.endTranslate()
    }
    TranslationButton {
        id: translateY
        font { family: transformHandleSet.fontFamilyName; pointSize: transformHandleSet.iconPointSize }
        axisType: VPVM.Model.AxisY
        axisColor: "green"
        onAxisTypeSet: transformHandleSet.axisTypeSet(value)
        onBeginTranslate: transformHandleSet.beginTranslate(delta)
        onTranslate: transformHandleSet.translate(delta)
        onEndTranslate: transformHandleSet.endTranslate()
    }
    TranslationButton {
        id: translateZ
        font { family: transformHandleSet.fontFamilyName; pointSize: transformHandleSet.iconPointSize }
        axisType: VPVM.Model.AxisZ
        axisColor: "blue"
        onAxisTypeSet: transformHandleSet.axisTypeSet(value)
        onBeginTranslate: transformHandleSet.beginTranslate(delta)
        onTranslate: transformHandleSet.translate(delta)
        onEndTranslate: transformHandleSet.endTranslate()
    }
    RotationButton {
        id: rotateX
        font { family: transformHandleSet.fontFamilyName; pointSize: transformHandleSet.iconPointSize }
        axisType: VPVM.Model.AxisX
        axisColor: "red"
        onAxisTypeSet: transformHandleSet.axisTypeSet(value)
        onBeginRotate: transformHandleSet.beginRotate(delta)
        onRotate: transformHandleSet.rotate(delta)
        onEndRotate: transformHandleSet.endRotate()
    }
    RotationButton {
        id: rotateY
        font { family: transformHandleSet.fontFamilyName; pointSize: transformHandleSet.iconPointSize }
        axisType: VPVM.Model.AxisY
        axisColor: "green"
        onAxisTypeSet: transformHandleSet.axisTypeSet(value)
        onBeginRotate: transformHandleSet.beginRotate(delta)
        onRotate: transformHandleSet.rotate(delta)
        onEndRotate: transformHandleSet.endRotate()
    }
    RotationButton {
        id: rotateZ
        font { family: transformHandleSet.fontFamilyName; pointSize: transformHandleSet.iconPointSize }
        axisType: VPVM.Model.AxisZ
        axisColor: "blue"
        onAxisTypeSet: transformHandleSet.axisTypeSet(value)
        onBeginRotate: transformHandleSet.beginRotate(delta)
        onRotate: transformHandleSet.rotate(delta)
        onEndRotate: transformHandleSet.endRotate()
    }
}
