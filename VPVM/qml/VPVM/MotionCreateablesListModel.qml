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
import com.github.mmdai.VPVM 1.0
import "FontAwesome.js" as FontAwesome

ListModel {
    id: motionCreateablesListModel
    property var __actions : ({})
    function __handlePrevious(row) {
        if (timeline.restoreEditMotionState()) {
            timelineView.state =  "editMotion"
        }
        else {
            timelineView.state = "initialState"
        }
    }
    function __handleCamera(row) {
        var motion = row.motion
        console.assert(motion)
        scene.currentMotion = motion
        sceneTabView.currentIndex = sceneTabView.cameraTabIndex
        timeline.assignCamera(motion, scene.camera)
        timelineView.state = "editMotion"
    }
    function __handleLight(row) {
        var motion = row.motion
        console.assert(motion)
        scene.currentMotion = motion
        sceneTabView.currentIndex = sceneTabView.lightTabIndex
        timeline.assignLight(motion, scene.light)
        timelineView.state = "editMotion"
    }
    function __handlePlus(row) {
        addModelAction.trigger()
    }
    function __handleModel(row) {
        var model = row.model, motion = model.childMotion
        console.assert(model)
        scene.currentModel = model
        scene.currentMotion = motion
        sceneTabView.currentIndex = sceneTabView.modelTabIndex
        timeline.assignModel(model)
        if (motion) {
            timelineView.state = "editMotion"
        }
    }
    function handleCurrentItem(index) {
        var row = get(index), iconText = row.iconText
        var action = (__actions[iconText] || __handleModel)
        action(row)
    }
    function updateModels() {
        clear()
        var models = scene.project.availableModels
        if (models.length > 0) {
            append({ "name": qsTr("Back Previous Motion"), "iconText": FontAwesome.Icon.CircleArrowLeft, "favicon": null })
        }
        append({ "name": qsTr("Add Asset/Model"), "iconText": FontAwesome.Icon.PlusSign, "favicon": "" })
        append({ "name": qsTr("Camera"), "iconText": FontAwesome.Icon.Camera, "favicon": "", "motion": scene.camera.motion })
        append({ "name": qsTr("Light"), "iconText": "\uf0eb", favicon: "", "motion": scene.light.motion }) // LightBulb
        var allModels = [], i
        for (i in models) {
            allModels.push(models[i])
        }
        allModels.sort(function(a, b) { return a.orderIndex - b.orderIndex })
        for (i in allModels) {
            var model = allModels[i], item = {
                "name": model.name,
                "iconText": FontAwesome.Icon.User,
                "favicon": model.faviconUrl.toString(),
                "model": model,
                "motion": null,
                "category": qsTr("Model")
            }
            append(item)
        }
    }
    Component.onCompleted: {
        __actions[FontAwesome.Icon.CircleArrowLeft] = __handlePrevious
        __actions[FontAwesome.Icon.Camera] = __handleCamera
        __actions["\uf0eb"] = __handleLight
        __actions[FontAwesome.Icon.PlusSign] = __handlePlus
        updateModels()
        scene.project.initializeOnce()
    }
}
