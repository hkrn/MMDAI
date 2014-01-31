/**

 Copyright (c) 2010-2014  hkrn

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
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Window 2.1
import QtQuick.Layouts 1.1
import com.github.mmdai.VPMM 1.0 as VPMM

ApplicationWindow {
    readonly property bool isOSX: Qt.platform.os === "osx"
    readonly property int applicationLayoutAnchorMargin : 10
    property bool isFullSceneMode: false
    minimumWidth: 960
    minimumHeight: 620
    title: "%1 - %2".arg(Qt.application.name).arg(scene.project.title)

    function __handleApplicationStateChange() {
        var state = Qt.application.state
        scene.state = state === Qt.ApplicationActive ? scene.lastStateAtSuspend : "suspend"
    }
    function __handleRequestedFileUrlChange() {
        var fileUrl = applicationBootstrapOption.requestedFileUrl, fileUrlString = fileUrl.toString()
        if (/\.(?:pm[xd]|x)$/i.test(fileUrlString)) {
            scene.project.loadModel(fileUrl)
        }
        else if (/\.(?:vmd|mvd)$/.test(fileUrlString)) {
            scene.project.loadMotion(fileUrl, scene.currentModel, VPVM.Project.ModelMotion)
        }
        else if (/\.vpd$/.test(fileUrlString)) {
            scene.project.loadPose(fileUrl, scene.currentModel)
        }
    }
    function updateWindowRect() {
        x = applicationPreference.windowRect.x
        y = applicationPreference.windowRect.y
        width = applicationPreference.windowRect.width
        height = applicationPreference.windowRect.height
    }
    function exitApplication() {
        scene.state = "suspend"
        applicationPreference.windowRectChanged.disconnect(updateWindowRect)
        applicationPreference.windowRect.x = x
        applicationPreference.windowRect.y = y
        applicationPreference.windowRect.width = width
        applicationPreference.windowRect.height = height
        applicationPreference.sync()
        Qt.quit()
    }
    Component.onCompleted: {
        scene.project.initializeOnce()
        updateWindowRect()
        applicationPreference.windowRectChanged.connect(updateWindowRect)
        applicationBootstrapOption.requestedFileUrlChanged.connect(__handleRequestedFileUrlChange)
        Qt.application.stateChanged.connect(__handleApplicationStateChange)
    }

    ApplicationWindow {
        id: progressWindow
        property string text
        property real maximumValue
        property real minimumValue
        width: 350
        height: 80
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: applicationLayoutAnchorMargin
            Layout.alignment: Qt.AlignCenter
            Text {
                Layout.fillWidth: true
                text: progressWindow.text
            }
            ProgressBar {
                Layout.fillWidth: true
                minimumValue: progressWindow.minimumValue
                maximumValue: progressWindow.maximumValue
            }
            Item { Layout.fillHeight: true }
        }
    }
    FontLoader { id: fontAwesome; source: "FontAwesome.%1".arg(isOSX ? "otf" : "ttf") }

    FileDialog {
        id: loadModelDialog
        nameFilters: [
            qsTr("MikuMikuDance Model File (*.pmd *.pmx)")
        ]
        selectExisting: true
        onAccepted: scene.project.loadModel(loadModelDialog.fileUrl)
    }
    Action {
        id: loadModelAction
        text: qsTr("Load Model")
        tooltip: qsTr("Load a model from file. The loaded model will make current.")
        onTriggered: loadModelDialog.open()
    }
    SaveDialog {
        id: saveModelDialog
        nameFilters: loadModelDialog.nameFilters
        title: qsTr("Save Model")
        suffix: "pmx"
    }
    Action {
        id: saveModelAction
        text: qsTr("Save Model")
        tooltip: qsTr("Save the current model to the file.")
        shortcut: "Ctrl+S"
        onTriggered: saveModelDialog.save(saveModelDialog.getPath())
    }
    Action {
        id: saveModelAsAction
        text: qsTr("Save Model As")
        tooltip: qsTr("Save the current model to the specified file.")
        shortcut: "Ctrl+Shift+S"
        function save(fileUrl) {
            var fileUrlString = fileUrl.toString(),
                    indexOf = fileUrlString.lastIndexOf("/"),
                    name = indexOf >= 0 ? fileUrlString.substring(indexOf + 1) : fileUrlString
            if (fileUrlString !== "") {
                progressWindow.text = qsTr("Saving Model %1").arg(name)
                progressWindow.show()
                scene.project.save(fileUrl)
                progressWindow.hide()
            }
        }
        onTriggered: save(saveModelDialog.getPathAs())
    }
    Action {
        id: copyAction
        enabled: scene.project.currentModel
        text: qsTr("&Copy")
        tooltip: qsTr("Copy current selected items.")
        shortcut: "Ctrl+C"
        onTriggered: {}
    }
    Action {
        id: pasteAction
        enabled: scene.project.currentModel
        text: qsTr("&Paste")
        tooltip: qsTr("Paste copied items.")
        shortcut: "Ctrl+V"
        onTriggered: {}
    }
    Action {
        id: cutAction
        enabled: scene.project.currentModel
        text: qsTr("Cu&t")
        tooltip: qsTr("Cut current selected items.")
        shortcut: "Ctrl+X"
        onTriggered: {}
    }
    Action {
        id: undoAction
        enabled: scene.project.canUndo
        text: qsTr("Undo")
        tooltip: qsTr("Undo the previous action.")
        shortcut: "Ctrl+Z"
        onTriggered: scene.project.undo()
    }
    Action {
        id: redoAction
        enabled: scene.project.canRedo
        text: qsTr("Redo")
        tooltip: qsTr("Redo the previous action.")
        shortcut: "Ctrl+Shift+Z"
        onTriggered: scene.project.redo()
    }
    Action {
        id: deleteModelAction
        text: qsTr("Delete Current Model")
        tooltip: qsTr("Delete current model. this will delete model and the bound motions, cannot be undone.")
        enabled: scene.project.currentModel
        onTriggered: scene.project.deleteModel(scene.project.currentModel)
    }
    Action {
        id: openGlobalPreferenceAction
        text: qsTr("Preference")
        tooltip: qsTr("Show global preference dialog.")
        shortcut: "Ctrl+,"
        onTriggered: globalPreferenceWindowLoader.open({ "graphicsDevice": scene.graphicsDevice })
    }
    Action {
        id: openAboutAction
        text: qsTr("About %1").arg(Qt.application.name)
        tooltip: qsTr("Show information dialog of %1.").arg(Qt.application.name)
        onTriggered: aboutWindowLoader.open()
    }
    Action {
        id: openAboutQtAction
        text: qsTr("About Qt 5.2")
        tooltip: qsTr("Show information dialog of Qt.")
        onTriggered: VPMM.UIAuxHelper.openAboutQt()
    }
    Action {
        id: updateApplicationAction
        text: qsTr("Check for Updates...")
        tooltip: qsTr("Check updates of %1.").arg(Qt.application.name)
        onTriggered: VPMM.Updater.checkForUpdate()
    }
    Action {
        id: exitApplicationAction
        text: qsTr("&Exit")
        tooltip: qsTr("Exit this application.")
        shortcut: "Ctrl+Q"
        onTriggered: {
            if (scene.project.dirty) {
                confirmSavingProjectBeforeClosingDialog.open()
            }
            else {
                exitApplication()
            }
        }
    }

    SystemPalette { id: systemPalette }
    color: systemPalette.window
    statusBar: StatusBar {
        visible: !scene.isFullView
        Label {
            id: statusBarLabel
            Layout.fillWidth: true
        }
    }
    menuBar: MenuBar {
        Menu {
            id: fileMenu
            title: isOSX ? qsTr("File") : qsTr("&File")
            MenuItem { action: loadModelAction }
            MenuSeparator {}
            MenuItem { action: saveModelAction }
            MenuSeparator { visible: exitApplicationMenuItem.visible }
            MenuItem {
                id: exitApplicationMenuItem
                visible: !isOSX
                action: exitApplicationAction
            }
        }
        Menu {
            id: editMenu
            title: isOSX ? qsTr("Edit") : qsTr("&Edit")
            MenuItem { action: copyAction }
            MenuItem { action: pasteAction }
            MenuItem { action: cutAction }
            MenuSeparator {}
            MenuItem { id: undoMenuItem; action: undoAction; enabled: false }
            MenuItem { id: redoMenuItem; action: redoAction; enabled: false }
        }
        Menu {
            id: modelMenu
            title: isOSX ? qsTr("Model") : qsTr("&Model")
            MenuItem { action: deleteModelAction }
        }
        Menu {
            id: windowMenu
            title: isOSX ? qsTr("Window") : qsTr("&Window")
            MenuSeparator {}
            MenuItem { action: openAboutQtAction }
        }
        Menu {
            id: helpMenu
            title: isOSX ? qsTr("Help") : qsTr("&Help")
            MenuItem { action: openGlobalPreferenceAction }
            MenuSeparator { visible: !isOSX }
            MenuItem { visible: VPMM.Updater.available; action: updateApplicationAction }
            MenuItem { action: openAboutAction }
        }
    }

    ListModel {
        id: verticesModel
        function indexOf(value) {
            if (value) {
                for (var i = 0, l = count, uuid = value.uuid; i < l; i++) {
                    var item = get(i).item
                    if (item && item.uuid === uuid) {
                        return i
                    }
                }
            }
            return 0
        }
        function __handleCurrentModelChanged() {
            clear()
            var model = scene.project.currentModel
            if (model) {
                var vertices = model.allVertices
                for (var i in vertices) {
                    var vertex = vertices[i]
                    append({ "item": vertex, "text": vertex.name })
                }
            }
        }
        Component.onCompleted: scene.project.currentModelChanged.connect(__handleCurrentModelChanged)
    }
    ListModel {
        id: materialsModel
        function indexOf(value) {
            if (value) {
                for (var i = 0, l = count, uuid = value.uuid; i < l; i++) {
                    var item = get(i).item
                    if (item && item.uuid === uuid) {
                        return i
                    }
                }
            }
            return 0
        }
        function __handleCurrentModelChanged() {
            clear()
            var model = scene.project.currentModel
            if (model) {
                var materials = model.allMaterials
                append({ "item": null, "text": qsTr("None") })
                for (var i in materials) {
                    var material = materials[i]
                    append({ "item": material, "text": material.name })
                }
            }
        }
        Component.onCompleted: scene.project.currentModelChanged.connect(__handleCurrentModelChanged)
    }
    ListModel {
        id: bonesModel
        function indexOf(value) {
            if (value) {
                for (var i = 0, l = count, uuid = value.uuid; i < l; i++) {
                    var item = get(i).item
                    if (item && item.uuid === uuid) {
                        return i
                    }
                }
            }
            return 0
        }
        function __handleCurrentModelChanged() {
            clear()
            var model = scene.project.currentModel
            if (model) {
                var bones = model.allBones
                append({ "item": null, "text": qsTr("None") })
                for (var i in bones) {
                    var bone = bones[i]
                    append({ "item": bone, "text": bone.name })
                }
            }
        }
        Component.onCompleted: scene.project.currentModelChanged.connect(__handleCurrentModelChanged)
    }
    ListModel {
        id: labelsModel
        function indexOf(value) {
            if (value) {
                for (var i = 0, l = count, uuid = value.uuid; i < l; i++) {
                    var item = get(i).item
                    if (item && item.uuid === uuid) {
                        return i
                    }
                }
            }
            return 0
        }
        function __handleCurrentModelChanged() {
            clear()
            var model = scene.project.currentModel
            if (model) {
                var labels = model.allLabels
                append({ "item": null, "text": qsTr("None") })
                for (var i in labels) {
                    var label = labels[i]
                    append({ "item": label, "text": label.name })
                }
            }
        }
        Component.onCompleted: {
            scene.project.currentModelChanged.connect(__handleCurrentModelChanged)
        }
    }
    ListModel {
        id: morphsModel
        function indexOf(value) {
            if (value) {
                for (var i = 0, l = count, uuid = value.uuid; i < l; i++) {
                    var item = get(i).item
                    if (item && item.uuid === uuid) {
                        return i
                    }
                }
            }
            return 0
        }
        function __handleCurrentModelChanged() {
            clear()
            var model = scene.project.currentModel
            if (model) {
                var morphs = model.allMorphs
                append({ "item": null, "text": qsTr("None") })
                for (var i in morphs) {
                    var morph = morphs[i]
                    append({ "item": morph, "text": morph.name })
                }
            }
        }
        Component.onCompleted: scene.project.currentModelChanged.connect(__handleCurrentModelChanged)
    }
    ListModel {
        id: rigidBodiesModel
        function indexOf(value) {
            if (value) {
                for (var i = 0, l = count, uuid = value.uuid; i < l; i++) {
                    var item = get(i).item
                    if (item && item.uuid === uuid) {
                        return i
                    }
                }
            }
            return 0
        }
        function __handleCurrentModelChanged() {
            clear()
            var model = scene.project.currentModel
            if (model) {
                var bodies = model.allRigidBodies
                append({ "item": null, "text": qsTr("None") })
                for (var i in bodies) {
                    var body = bodies[i]
                    append({ "item": body, "text": body.name })
                }
            }
        }
        Component.onCompleted: scene.project.currentModelChanged.connect(__handleCurrentModelChanged)
    }
    ListModel {
        id: jointsModel
        function indexOf(value) {
            if (value) {
                var uuid = value.uuid
                for (var i = 0, l = count; i < l; i++) {
                    var item = get(i).item
                    if (item && item.uuid === uuid) {
                        return i
                    }
                }
            }
            return 0
        }
        function __handleCurrentModelChanged() {
            clear()
            var model = scene.project.currentModel
            if (model) {
                var joints = model.allJoints
                append({ "item": null, "text": qsTr("None") })
                for (var i in joints) {
                    var joint = joints[i]
                    append({ "item": joint, "text": joint.name })
                }
            }
        }
        Component.onCompleted: scene.project.currentModelChanged.connect(__handleCurrentModelChanged)
    }
    ListModel {
        id: softBodiesModel
        function __handleCurrentModelChanged() {
            var model = scene.project.currentModel
            if (model) {
            }
        }
        Component.onCompleted: scene.project.currentModelChanged.connect(__handleCurrentModelChanged)
    }

    ListModel {
        id: uvTypeModel
        ListElement { text: "Default"; value: 0 }
        ListElement { text: "UVA1"; value: 1 }
        ListElement { text: "UVA2"; value: 2 }
        ListElement { text: "UVA3"; value: 3 }
        ListElement { text: "UVA4"; value: 4 }
    }
    ListModel {
        id: transformTypeModel
        ListElement { text: "BDEF1"; value: 0 }
        ListElement { text: "BDEF2"; value: 1 }
        ListElement { text: "BDEF4"; value: 3 }
        ListElement { text: "SDEF";  value: 2 }
        ListElement { text: "QDEF";  value: 4 }
        function indexOf(type) {
            switch (type) {
            case VPMM.Vertex.Bdef1:
            default:
                return 0
            case VPMM.Vertex.Bdef2:
                return 1
            case VPMM.Vertex.Bdef4:
                return 2
            case VPMM.Vertex.Sdef:
                return 3
            case VPMM.Vertex.Qdef:
                return 4
            }
        }
    }
    ListModel {
        id: languageModel
        ListElement { text: "Japanese" }
        ListElement { text: "English" }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        Layout.fillHeight: true
        Rectangle {
            id: modelViewPanel
            Layout.minimumWidth: 370
            Layout.fillHeight: true
            color: systemPalette.window
            width: 370
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Layout.alignment: Qt.AlignCenter
                    Label {
                        visible: objectType.visible
                        text: qsTr("Type")
                    }
                    ComboBox {
                        id: objectType
                        visible: stackView.depth === 1
                        model: [
                            { "text": qsTr("Vertex"), "path": "VertexView.qml", "reference": verticesModel },
                            { "text": qsTr("Material"), "path": "MaterialView.qml", "reference": materialsModel },
                            { "text": qsTr("Bone"), "path": "BoneView.qml", "reference": bonesModel },
                            { "text": qsTr("Morph"), "path": "MorphView.qml", "reference": morphsModel },
                            { "text": qsTr("Label"), "path": "LabelView.qml", "reference": labelsModel },
                            { "text": qsTr("RigidBody"), "path": "RigidBodyView.qml", "reference": rigidBodiesModel },
                            { "text": qsTr("Joint"), "path": "JointView.qml", "reference": jointsModel },
                            { "text": qsTr("SoftBody"), "path": "SoftBodyView.qml", "reference": softBodiesModel }
                        ]
                        onCurrentIndexChanged: objectListView.model = model[currentIndex].reference
                    }
                    Button {
                        visible: stackView.depth > 1
                        text: "Back To List"
                        onClicked: stackView.pop(null)
                    }
                }
                StackView {
                    id: stackView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    initialItem: Item {
                        width: parent.width
                        height: parent.height
                        TableView {
                            id: objectListView
                            anchors.fill: parent
                            frameVisible: false
                            sortIndicatorVisible: false
                            TableViewColumn {
                                role: "index"
                                title: "Index"
                                resizable: true
                            }
                            TableViewColumn {
                                role: "text"
                                title: "Name"
                                resizable: true
                            }
                            onDoubleClicked: {
                                var targetObject = model.get(row).item,
                                        path = objectType.model[objectType.currentIndex].path
                                var arguments = {
                                    "item": Qt.resolvedUrl(path),
                                    "properties": { "targetObject": targetObject }
                                }
                                stackView.push(arguments)
                            }
                        }
                    }
                    delegate: StackViewDelegate {
                        function transitionFinished(properties) {
                            properties.exitItem.opacity = 1
                        }
                        property Component pushTransition: StackViewTransition {
                            PropertyAnimation {
                                target: enterItem
                                property: "opacity"
                                from: 0
                                to: 1
                            }
                            PropertyAnimation {
                                target: exitItem
                                property: "opacity"
                                from: 1
                                to: 0
                            }
                        }
                    }
                }
            }
        }
        SplitView {
            orientation: Qt.Vertical
            Scene {
                id: scene
                Layout.fillWidth: true
                Layout.fillHeight: true
                offsetX: modelViewPanel.width
                offsetY: statusBar.height * statusBar.visible + propertyPanel.height * propertyPanel.visible
            }
            Rectangle {
                id: propertyPanel
                visible: false
                Layout.minimumHeight: 240
                Layout.maximumHeight: 400
                height: 240
                color: systemPalette.window
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    RowLayout {
                        Label {
                            text: qsTr("Type")
                        }
                        ComboBox {
                            id: modelVersionComboBox
                            function indexOf(value) {
                                var result = model.filter(function(element){ return element.value === value })
                                return result.length > 0 ? result[0].value : -1
                            }
                            model: [
                                { "text": "PMD 1.0", "value": 1.0 },
                                { "text": "PMX 2.0", "value": 2.0 },
                                { "text": "PMX 2.1", "value": 2.1 }
                            ]
                            currentIndex: scene.project.currentModel ? indexOf(scene.project.currentModel.version) : 0
                        }
                        Binding {
                            target: scene.project.currentModel
                            property: "version"
                            value: modelVersionComboBox.model[modelVersionComboBox.currentIndex].value
                            when: modelVersionComboBox.hovered
                        }
                        Label {
                            text: qsTr("Encoding")
                        }
                        ComboBox {
                            id: modelEncodingTypeComboBox
                            function indexOf(value) {
                                var result = model.filter(function(element){ return element.value === value })
                                return result.length > 0 ? result[0].value : -1
                            }
                            model: [
                                { "text": "Shift_JIS", "value": VPMM.Model.ShiftJIS },
                                { "text": "UTF-8", "value": VPMM.Model.UTF8 },
                                { "text": "UTF-16", "value": VPMM.Model.UTF16 }
                            ]
                            currentIndex: scene.project.currentModel ? indexOf(scene.project.currentModel.encodingType) : 0
                        }
                        Binding {
                            target: scene.project.currentModel
                            property: "encodingType"
                            value: modelEncodingTypeComboBox.model[modelEncodingTypeComboBox.currentIndex].value
                            when: modelEncodingTypeComboBox.hovered
                        }
                        Label {
                            text: qsTr("Additional UV")
                        }
                        SpinBox {
                            id: modelUVASpinBox
                            minimumValue: 0
                            maximumValue: 4
                            value: scene.project.currentModel ? scene.project.currentModel.numUVA : 0
                        }
                        Binding {
                            target: scene.project.currentModel
                            property: "uavCount"
                            value: modelUVASpinBox.value
                            when: modelUVASpinBox.hovered
                        }
                    }
                    TextField {
                        Layout.fillWidth: true
                        placeholderText: qsTr("Input Model Name Here")
                        text: scene.project.currentModel ? scene.project.currentModel.name : ""
                    }
                    TextArea {
                        Layout.fillWidth: true
                        text: scene.project.currentModel ? scene.project.currentModel.comment : ""
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }
}
