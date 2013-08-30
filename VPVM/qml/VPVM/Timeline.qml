//  Timeline.js v0.1 / 2011-05-01
//  A compact JavaScript animation library with a GUI timeline for fast editing.
//  by Marcin Ignac (http://marcinignac.com)
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

//
// modified by hkrn
//
// Licensed under MIT license same as above
//

import QtQuick 2.1
import QtQuick.Layouts 1.0
import "FontAwesome.js" as FontAwesome

FocusScope {
    id: timeline

    property color  backgroundFillColor: "#FFFFFF"
    property color  selectedLabelFillColor: "#FFFFFF"
    property color  selectedLabelStrokeColor: "#000000"
    property color  objectLabelFillColor: "#FFFFFF"
    property color  objectLabelStrokeColor: "#000000"
    property color  propertyLabelFillTextColor: "#555555"
    property color  rombusBaseFillColor: "#999999"
    property color  rombusBaseStrokeColor: "#666666"
    property color  rombusSelectedStrokeColor: "#FF0000"
    property color  rombusFillColor: "#DDDDDD"
    property color  buttonBaseFillColor: "#DDDDDD"
    property color  buttonSymbolStrokeColor: "#777777"
    property color  labelPanelStrokeColor: "#000000"
    property color  bottomTrackLineStrokeColor: "#FFFFFF"
    property color  timeScaleFillColor: "#666666"
    property color  timeScaleStrokeColor: "#999999"
    property color  timeTickerStrokeColor: "#FF0000"
    property color  tracksScrollBarFillColor: "#DDDDDD"
    property color  tracksScrollBarStrokeColor: "#999999"
    property color  timeScrollBarFillColor: "#DDDDDD"
    property color  timeScrollBarStrokeColor: "#999999"
    property color  headerBorderStrokeColor: "#000000"
    property int    fontPointsize: 10
    property int    iconPointSize: 14
    property string fontFamily: "sans-serif"
    property string fontPointSizeText: fontPointsize + "px"
    property string iconPointSizeText: iconPointSize + "px"

    property int  __trackLabelWidth: 108
    property int  __trackLabelHeight: 20
    property int  __tracksScrollBarWidth: 16
    property int  __tracksScrollBarHeight: 0
    property int  __tracksScrollThumbHeight: 0
    property int  __tracksScrollThumbMinimumHeight : 20
    property int  __timeScrollBarWidth: 0
    property int  __timeScrollBarHeight: 16
    property int  __timeScrollThumbWidth: 0
    property int  __timeScrollThumbMinimumWidth : 10
    property int  __headerHeight: 30
    property real framesPerSecond : 30
    property real timeSeconds: 0
    property int  timeIndex: 0
    property real durationSeconds: 600
    property int  durationTimeIndex: durationSeconds * framesPerSecond
    property real __timeScrollThumbPos: 0
    property real __tracksScrollThumbPos: 0
    property real __timeScrollX: 0
    property real timeScaleFactor: 1.0
    property real maximumTimeScaleFactor: 1.0
    property real minimumTimeScaleFactor: 0.01
    property real __trackWidth: 200 * timeScaleFactor
    property real __tracksScrollY: 0
    property bool hasSelectedKeyframes: __selectedKeyframes.length > 0
    property bool __draggingTime: false
    property bool __draggingTracksScrollThumb: false
    property bool __draggingTimeScrollThumb: false
    property bool __draggingKeyframes: false
    property bool __draggingTimeScale: false
    property var  __selectedKeyframes: []
    property var  __tracks: []
    property var  __target2tracks: ({})
    property bool __cancelKeyClick: false
    property real __timeScrollThumbDragOffset: 0
    property real __tracksScrollThumbDragOffset: 0
    property alias backgroundImageSource : backgroundImage.source

    signal keyframeWillAdd(var opaque, int timeIndex);
    signal keyframesDidSelect(var keyframes);
    signal draggingKeyframesDidBegin(int timeIndex)
    signal draggingKeyframesDidCommit(var keyframes, int timeIndex)
    signal opaqueObjectDidSelect(var opaque);
    signal timelineWillHide();
    signal timelineWillPlay();
    signal timelineWillPause();
    signal timelineWillStop();

    onDurationTimeIndexChanged: {
        durationSeconds = durationTimeIndex / framesPerSecond
        canvas.requestPaint()
    }
    onTimeIndexChanged: {
        timeSeconds = timeIndex / framesPerSecond
        canvas.requestPaint()
    }
    onDurationSecondsChanged: durationTimeIndex = durationSeconds * framesPerSecond
    onTimeSecondsChanged: timeIndex = timeSeconds * framesPerSecond

    function sortTrackKeyframes(keyframes) {
        keyframes.sort(function(a, b) { return a.timeIndex - b.timeIndex; });
    }
    function toggleLockMotionTrack(opaque) {
        var track = findTrack(opaque)
        if (track) {
            var motionTrack = track.parentMotionTrack
            if (motionTrack) {
                motionTrack.locked = motionTrack.locked ? false : true
                canvas.requestPaint()
            }
        }
    }
    function toggleVisibleMotionTrack(opaque) {
        var track = findTrack(opaque)
        if (track) {
            var motionTrack = track.parentMotionTrack
            if (motionTrack) {
                motionTrack.visible = motionTrack.visible ? false : true
                canvas.requestPaint()
            }
        }
    }
    function addKeyframe(keyframe) {
        console.assert(keyframe)
        var track = findTrack(keyframe.opaque)
        if (track) {
            var trackKeyframes = track.keyframes
            trackKeyframes.push(keyframe)
            sortTrackKeyframes(trackKeyframes)
            canvas.requestPaint()
        }
    }
    function removeKeyframe(keyframe) {
        console.assert(keyframe)
        var track = findTrack(keyframe.opaque)
        if (track) {
            var index = track.keyframes.indexOf(keyframe)
            track.keyframes.splice(index, 1)
            canvas.requestPaint()
        }
    }
    function replaceKeyframe(dst, src) {
        var track = findTrack(dst.opaque)
        if (track) {
            var index = track.keyframes.indexOf(src)
            track.keyframes.splice(index, 1, dst)
            canvas.requestPaint()
        }
    }

    function __assignTrack(track, motionTrack) {
        var numKeyframes = motionTrack.length
        track.keyframes = []
        track.parentMotionTrack = motionTrack
        for (var i = 0; i < numKeyframes; i++) {
            var keyframe = motionTrack.findKeyframeAt(i)
            track.keyframes.push(keyframe)
        }
        sortTrackKeyframes(track.keyframes)
    }
    function __assignBoneTracks(bones, motion) {
        for (var i in bones) {
            var bone = bones[i],
                    track = timeline.findTrack(bone),
                    motionTrack = motion.findBoneMotionTrack(bone.name)
            if (track && motionTrack) {
                __assignTrack(track, motionTrack)
            }
        }
    }
    function __assignMorphTracks(morphs, motion) {
        for (var i in morphs) {
            var morph = morphs[i],
                    track = timeline.findTrack(morph),
                    motionTrack = motion.findMorphMotionTrack(morph.name)
            if (track && motionTrack) {
                __assignTrack(track, motionTrack)
            }
        }
    }
    function __assignModelMotion(labels, motion) {
        console.assert(labels && motion)
        for (var i in labels) {
            var label = labels[i], bones = label.bones, morphs = label.morphs
            if (bones.length > 0) {
                __assignBoneTracks(bones, motion)
            }
            else if (morphs.length > 0) {
                __assignMorphTracks(morphs, motion)
            }
        }
        canvas.requestPaint();
    }
    function assignModel(model) {
        console.assert(model)
        var motion = model.childMotion, labels = model.availableLabels, anims = []
        for (var i in labels) {
            var label = labels[i], bones = label.bones, morphs = label.morphs, opaqueObjects = null
            if (bones.length > 0) {
                opaqueObjects = bones
            }
            else if (morphs.length > 0) {
                opaqueObjects = morphs
            }
            if (opaqueObjects != null) {
                for (var j in opaqueObjects) {
                    var opaqueObject = opaqueObjects[j]
                    var animation = {
                        "target": label,
                        "label": label,
                        "propertyName": opaqueObject.name,
                        "opaqueObject": opaqueObject
                    }
                    anims.push(animation)
                }
            }
        }
        __initializeTracks(anims)
        __assignModelMotion(labels, motion)
    }
    function assignCamera(motion, camera) {
        console.assert(motion && camera)
        var anims = []
        var label = { "name": qsTr("Camera"), "index": 0 }
        var animation = {
            "target": label,
            "label": label,
            "propertyName": camera.name,
            "opaqueObject": camera
        }
        anims.push(animation)
        __initializeTracks(anims)
        var track = timeline.findTrack(camera)
        if (track) {
            __assignTrack(track, camera.track)
        }
    }
    function assignLight(motion, light) {
        console.assert(motion && light)
        var anims = []
        var label = { "name": qsTr("Light"), "index": 0 }
        var animation = {
            "target": label,
            "label": label,
            "propertyName": light.name,
            "opaqueObject": light
        }
        anims.push(animation)
        __initializeTracks(anims)
        var track = timeline.findTrack(light)
        if (track) {
            __assignTrack(track, light.track)
        }
    }

    function findTrack(target) {
        return __target2tracks[target];
    }
    function timeSecondsToX(time) {
        var animationEnd = durationSeconds, timelineTrackLabelWidth = __trackLabelWidth,
                visibleTime = timeline.xToTimeSeconds(canvas.width - timelineTrackLabelWidth - __tracksScrollBarWidth) - timeline.xToTimeSeconds(20); // 50 to get some additional space
        if (visibleTime < animationEnd) {
            time -= (animationEnd - visibleTime) * __timeScrollX;
        }
        return timelineTrackLabelWidth + time * __trackWidth + 10;
    }
    function xToTimeSeconds(x) {
        var animationEnd = durationSeconds, timelineTrackLabelWidth = __trackLabelWidth,
                visibleTime = (canvas.width - timelineTrackLabelWidth - __tracksScrollBarWidth - 20) / __trackWidth,
                timeShift = Math.max(0, (animationEnd - visibleTime) * __timeScrollX);
        return (x - timelineTrackLabelWidth - 10) / __trackWidth + timeShift;
    }
    function getVisibleTracks() {
        var visibleTracks = [], tracks = __tracks, numTracks = tracks.length;
        for (var i = 0; i < numTracks; i++) {
            var track = tracks[i];
            if (track.type === "property" && track.parent.collapsed) {
                continue;
            }
            visibleTracks.push(track);
        }
        /* padding bottom label */
        visibleTracks.push({ "type": null });
        return visibleTracks;
    }
    function getTrackAt(mouseY) {
        var visibleTracks = getVisibleTracks(),
                scrollY = __tracksScrollY * (visibleTracks.length * __trackLabelHeight - canvas.height + __headerHeight),
                clickedTrackNumber = Math.floor((mouseY - __headerHeight + scrollY) / __trackLabelHeight);
        if (clickedTrackNumber >= 0 && clickedTrackNumber >= visibleTracks.length) {
            return null;
        }
        return visibleTracks[clickedTrackNumber];
    }
    function getPropertyTrackAt(mouseY) {
        var track = getTrackAt(mouseY);
        if (track) {
            return track.type === "property" ? track : null;
        }
        return null;
    }
    function getObjectTrackAt(mouseY) {
        var track = getTrackAt(mouseY);
        if (track) {
            return track.type === "object" ? track : null;
        }
        return null;
    }
    function findKeyframeByTimeIndex(keyframes, value) {
        var low = 0, high = keyframes.length, keyframe;
        if (high === 0) {
            return null
        }
        while (low < high) {
            var mid = (low + high) >>> 1;
            keyframe = keyframes[mid];
            if (keyframe.timeIndex < value) {
                low = mid + 1
            }
            else {
                high = mid
            }
        }
        keyframe = keyframes[low];
        return keyframe.timeIndex === value ? keyframe : null;
    }
    function selectKeyframesAtCurrentTimeIndex() {
        selectKeyframesByTimeIndex(timeIndex);
    }
    function selectKeyframesByTimeIndex(timeIndex) {
        var tracks = __tracks, numTracks = tracks.length, selectedKeyframes = [];
        for (var i = 0; i < numTracks; i++) {
            var track = tracks[i],
                    keyframes = track.keyframes || [],
                    keyframe = findKeyframeByTimeIndex(keyframes, timeIndex)
            if (keyframe) {
                selectedKeyframes.push(keyframe)
            }
        }
        __selectedKeyframes = selectedKeyframes;
        keyframesDidSelect(selectedKeyframes)
        canvas.requestPaint()
    }
    function selectRange(timeIndexFrom, timeIndexTo, visibleOnly) {
        var tracks = visibleOnly ? getVisibleTracks() : __tracks, numTracks = tracks.length, selectedKeyframes = []
        for (var i = 0; i < numTracks; i++) {
            var track = tracks[i], keyframes = track.keyframes || [], numKeyframes = keyframes.length
            for (var j = 0; j < numKeyframes; j++) {
                var keyframe = keyframes[j]
                if (keyframe.timeIndex >= timeIndexFrom && keyframe.timeIndex <= timeIndexTo) {
                    selectedKeyframes.push(keyframe)
                }
            }
        }
        __selectedKeyframes = selectedKeyframes
        keyframesDidSelect(selectedKeyframes)
        canvas.requestPaint()
    }
    function selectKeyframesAt(mouseX, mouseY, isCtrl) {
        var selectedTrack = timeline.getPropertyTrackAt(mouseY);
        if (!selectedTrack) {
            return;
        }
        var keyframesInSelectedTrack = selectedTrack.keyframes,
                numKeyframesInSelectedTrack = keyframesInSelectedTrack.length,
                timelineTrackLabelHeight = __trackLabelHeight * 0.3,
                selectedKeyframes = [];
        if (isCtrl) {
            selectedKeyframes.push(__selectedKeyframes)
        }
        else {
            __selectedKeyframes = [];
        }
        for (var i = 0; i < numKeyframesInSelectedTrack; i++) {
            var keyframe = keyframesInSelectedTrack[i],
                    time = keyframe.time,
                    x = timeline.timeSecondsToX(time);
            if (x >= mouseX - timelineTrackLabelHeight && x <= mouseX + timelineTrackLabelHeight) {
                selectedKeyframes.push(keyframe);
                break;
            }
        }
        if (!isCtrl || (isCtrl && selectedKeyframes.length > 0)) {
            __selectedKeyframes = selectedKeyframes;
            keyframesDidSelect(selectedKeyframes)
            canvas.requestPaint()
        }
    }
    function __initializeTracks(anims) {
        __tracks = [];
        var animation, tracks = null, track = null, label = null,
                i = 0, j = 0, numTracks = 0, test = 0,
                numAnimations = anims.length;
        for (i = 0; i < numAnimations; i++) {
            var objectTrack = null, propertyTrack = null;
            tracks = __tracks;
            numTracks = tracks.length;
            animation = anims[i];
            for (j = 0; j < numTracks; j++) {
                track = tracks[j];
                if (track.type === "object" && track.target === animation.target) {
                    objectTrack = track;
                }
                if (track.type === "property" && track.target === animation.target && track.propertyName === animation.propertyName) {
                    propertyTrack = track;
                }
            }
            if (!objectTrack) {
                label = animation.label;
                objectTrack = {
                    "type": "object",
                    "id": "t%1".arg(label.index),
                    "name": label.name,
                    "target": label,
                    "collapsed": true,
                    "selected": false,
                    "propertyTracks": []
                };
                __tracks.push(objectTrack);
                __target2tracks[label] = objectTrack;
            }
            if (!propertyTrack) {
                var opaqueObject = animation.opaqueObject;
                label = animation.label;
                propertyTrack = {
                    "type": "property",
                    "id": "%1_o%2".arg(objectTrack.id).arg(opaqueObject.index),
                    "name": opaqueObject.name,
                    "propertyName": opaqueObject.name,
                    "opaqueObject": opaqueObject,
                    "selected": false,
                    "target": label,
                    "parent": objectTrack,
                    "parentMotionTrack": null,
                    "keyframes": []
                }
                // find place to insert
                var parentObjectTrack = null, nextObjectTrack = null;
                tracks = __tracks;
                numTracks = tracks.length;
                for (var k = 0; k < numTracks; k++) {
                    track = tracks[k];
                    if (track.type === "object") {
                        if (parentObjectTrack && !nextObjectTrack) {
                            nextObjectTrack = track;
                        }
                        if (track.target === propertyTrack.target) {
                            parentObjectTrack = track;
                        }
                    }
                }
                if (nextObjectTrack) {
                    //add ad the end of this object property tracks, just before next one
                    var nextTrackIndex = __tracks.indexOf(nextObjectTrack);
                    __tracks.splice(nextTrackIndex, 0, propertyTrack);
                }
                else {
                    //add to end of all track
                    __tracks.push(propertyTrack);
                }
                __target2tracks[opaqueObject] = propertyTrack
                parentObjectTrack.propertyTracks.push(propertyTrack);
            }
        }
        canvas.requestPaint()
    }

    function markKeyframeAdded(mouseX, mouseY) {
        var selectedPropertyTrack = timeline.getPropertyTrackAt(mouseY);
        if (selectedPropertyTrack && xToTimeSeconds(mouseX) > 0) {
            keyframeWillAdd(selectedPropertyTrack.opaqueObject, timeIndex)
        }
    }
    function markTrackSelected(opaque) {
        resetSelectedTrack(opaque)
        var track = findTrack(opaque)
        if (track) {
            track.selected = true
        }
    }
    function resetSelectedTrack(opaque) {
        if (opaque && opaque.parentLabel) {
            var model = opaque.parentLabel.parentModel,
                    uuid = opaque.uuid,
                    isBone = model.findBoneByUuid(uuid) !== null,
                    isMorph = model.findMorphByUuid(uuid) !== null,
                    tracks = __tracks, numTracks = tracks.length
            for (var i in tracks) {
                var track = tracks[i], opaqueObject = track.opaqueObject
                if (opaqueObject) {
                    var opaqueObjectUuid = opaqueObject.uuid
                    if ((isBone && model.findBoneByUuid(opaqueObjectUuid) !== null) ||
                            (isMorph && model.findMorphByUuid(opaqueObjectUuid) !== null)) {
                        track.selected = false
                    }
                }
            }
        }
    }
    function refresh() {
        canvas.requestPaint()
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true
        property rect selectRegion : Qt.rect(0, 0, 0, 0)
        function scrollTracks(y) {
            __tracksScrollThumbPos = Math.max(0, y - __headerHeight - __tracksScrollThumbDragOffset);
            updateScrollTracks()
        }
        function scrollTracksOffsetDelta(offset) {
            var visibleTracks = getVisibleTracks()
            var value = offset / (visibleTracks.length * __trackLabelHeight)
            __tracksScrollThumbPos = Math.max(__tracksScrollThumbPos + offset, 0)
            updateScrollTracks()
        }
        function updateScrollTracks() {
            if (__tracksScrollThumbPos + __tracksScrollThumbHeight > __tracksScrollBarHeight) {
                __tracksScrollThumbPos = Math.max(0, __tracksScrollBarHeight - __tracksScrollThumbHeight);
            }
            if (__tracksScrollBarHeight - __tracksScrollThumbHeight > 0) {
                __tracksScrollY = __tracksScrollThumbPos / (__tracksScrollBarHeight - __tracksScrollThumbHeight);
            }
            else {
                __tracksScrollY = 0;
            }
            requestPaint()
        }
        function scrollTimeSeconds(x) {
            __timeScrollThumbPos = Math.max(0, x - __trackLabelWidth - __timeScrollThumbDragOffset);
            updateScrollTime()
        }
        function scrollTimeIndexOffsetDelta(value) {
            scrollTimeSecondsOffsetDelta(value / framesPerSecond)
        }
        function scrollTimeSecondsOffsetDelta(value) {
            var delta = value / timeSecondsToX(durationSeconds)
            __timeScrollThumbPos = Math.max(__timeScrollThumbPos + delta, 0)
            updateScrollTime()
        }
        function updateScrollTime() {
            if (__timeScrollThumbPos + __timeScrollThumbWidth > __timeScrollBarWidth) {
                __timeScrollThumbPos = Math.max(0, __timeScrollBarWidth - __timeScrollThumbWidth);
            }
            if (__timeScrollBarWidth - __timeScrollThumbWidth > 0) {
                __timeScrollX = __timeScrollThumbPos / (__timeScrollBarWidth - __timeScrollThumbWidth);
            }
            else {
                __timeScrollX = 0;
            }
            requestPaint()
        }
        MouseArea {
            anchors.fill: parent
            function handleMouseMove(x, y) {
                if (__draggingTracksScrollThumb) {
                    canvas.scrollTracks(y)
                }
                if (__draggingTimeScrollThumb) {
                    canvas.scrollTimeSeconds(x)
                }
                if (__draggingTime) {
                    var timelineTimeSeconds = Math.max(timeline.xToTimeSeconds(x), 0)
                    timeline.timeSeconds = timelineTimeSeconds;
                    if (x > canvas.width - __tracksScrollBarWidth) {
                        canvas.scrollTimeSecondsOffsetDelta(1)
                    }
                    else if (x < __trackLabelWidth) {
                        canvas.scrollTimeSecondsOffsetDelta(-1)
                    }
                    else {
                        canvas.requestPaint()
                    }
                }
                if (__draggingKeyframes) {
                    var selectedKeyframes = __selectedKeyframes,
                            numSelectedKeyframes = selectedKeyframes.length,
                            minimumTimeIndex = 1.0 / timeline.framesPerSecond;
                    for(var i = 0; i < numSelectedKeyframes; i++) {
                        var draggedKeyframe = selectedKeyframes[i], parentTrack = draggedKeyframe.parentTrack;
                        if (draggedKeyframe.time > 0 && !parentTrack.locked) {
                            draggedKeyframe.time = Math.max(minimumTimeIndex, timeline.xToTimeSeconds(x));
                        }
                    }
                    __cancelKeyClick = true;
                    __timeScrollThumbPos = __timeScrollX * (__timeScrollBarWidth - __timeScrollThumbWidth);
                    canvas.requestPaint()
                }
                if (__draggingTimeScale) {
                    var timelineTrackLabelWidth = __trackLabelWidth;
                    timeScaleFactor = Math.max(minimumTimeScaleFactor, Math.min(maximumTimeScaleFactor, (timelineTrackLabelWidth - x) / timelineTrackLabelWidth));
                    canvas.requestPaint()
                }
                if (canvas.selectRegion != Qt.rect(0, 0, 0, 0)) {
                    canvas.selectRegion.width = x - canvas.selectRegion.x
                    canvas.selectRegion.height = y - canvas.selectRegion.y
                    console.log(canvas.selectRegion)
                }
            }
            function scrollX(x) {
                var timelineTrackLabelWidth = __trackLabelWidth,
                        timeScrollThumbPos = __timeScrollThumbPos;
                if (x >= timelineTrackLabelWidth + timeScrollThumbPos && x <= timelineTrackLabelWidth + timeScrollThumbPos + __timeScrollThumbWidth) {
                    __timeScrollThumbDragOffset = x - timelineTrackLabelWidth - timeScrollThumbPos;
                    __draggingTimeScrollThumb = true;
                }
            }
            function scrollY(y) {
                var timelineHeaderHeight = __headerHeight,
                        timelineTracksScrollThumbPos = __tracksScrollThumbPos;
                if (y >= timelineHeaderHeight + timelineTracksScrollThumbPos && y <= timelineHeaderHeight + timelineTracksScrollThumbPos + __tracksScrollThumbHeight) {
                    __tracksScrollThumbDragOffset = y - timelineHeaderHeight - timelineTracksScrollThumbPos;
                    __draggingTracksScrollThumb = true;
                }
            }
            onClicked: {
                var x = mouse.x, y = mouse.y, headerHeight = __headerHeight;
                if (x < __trackLabelWidth && y > headerHeight) {
                    var propertyTrack = getPropertyTrackAt(y)
                    if (propertyTrack) {
                        var opaque = propertyTrack.opaqueObject
                        resetSelectedTrack(opaque)
                        opaqueObjectDidSelect(opaque);
                    }
                }
                if (__selectedKeyframes.length > 0 && !__cancelKeyClick) {
                    // timeline.showKeyEditDialog(event.pageX, event.pageY);
                }
            }
            onDoubleClicked: {
                var x = mouse.x, y = mouse.y;
                if (x <= __trackLabelWidth) {
                    var track = getObjectTrackAt(y);
                    if (track) {
                        track.collapsed = track.collapsed ? false : true;
                        canvas.requestPaint();
                    }
                }
                else if (x > __trackLabelWidth) {
                    var headerHeight = __headerHeight;
                    if (y < headerHeight) {
                        // timeline
                        var timeStr = prompt("Enter time") || "00:00:00";
                        var timeArr = timeStr.split(":");
                        var seconds = 0;
                        var minutes = 0;
                        var hours = 0;
                        if (timeArr.length > 0) {
                            seconds = parseInt(timeArr[timeArr.length-1]);
                        }
                        if (timeArr.length > 1) {
                            minutes = parseInt(timeArr[timeArr.length-2]);
                        }
                        if (timeArr.length > 2) {
                            hours = parseInt(timeArr[timeArr.length-3]);
                        }
                        timeline.timeSeconds = timeline.totalTime = hours * 60 * 60 + minutes * 60 + seconds;
                    }
                    else if (__selectedKeyframes.length == 0 && y > headerHeight && y < canvas.height - __timeScrollBarHeight) {
                        timeline.markKeyframeAdded(x, y);
                    }
                }
            }
            onPressed: {
                var x = mouse.x, y = mouse.y,
                        timelineTrackLabelWidth = __trackLabelWidth,
                        timeScrollHeight = __timeScrollBarHeight;
                if (x > canvas.width - __tracksScrollBarWidth && y > __headerHeight) {
                    // tracks scroll
                    scrollY(y);
                }
                else if (x > timelineTrackLabelWidth) {
                    if (y > canvas.height - timeScrollHeight) {
                        // time scroll
                        scrollX(x);
                    }
                    else if (y < __headerHeight) {
                        // timeline
                        __draggingTime = true;
                        handleMouseMove(x, y);
                    }
                    else if (y > __headerHeight && y < canvas.height - timeScrollHeight) {
                        // keyframes
                        var isCtrl = (mouse.modifiers & Qt.ControlModifier) === Qt.ControlModifier;
                        timeline.selectKeyframesAt(x, y, isCtrl);
                        if (__selectedKeyframes.length > 0) {
                            __draggingKeyframes = true;
                            var timeIndex = xToTimeSeconds(x) * framesPerSecond
                            draggingKeyframesDidBegin(timeIndex)
                        }
                        __cancelKeyClick = false;
                    }
                    else {
                        // begin dragging region
                        canvas.selectRegion = Qt.rect(x, y, 1, 1)
                    }
                }
                else if (x < timelineTrackLabelWidth && y > canvas.height - timeScrollHeight) {
                    // time scale
                    timeScaleFactor = Math.max(minimumTimeScaleFactor, Math.min(maximumTimeScaleFactor, (timelineTrackLabelWidth - x) / timelineTrackLabelWidth));
                    __draggingTimeScale = true;
                    // timeline.save();
                }
                forceActiveFocus()
            }
            onPositionChanged: handleMouseMove(mouse.x, mouse.y)
            onReleased: {
                if (__draggingKeyframes) {
                    __draggingKeyframes = false;
                    var timeIndex = Math.max(xToTimeSeconds(mouse.x) * framesPerSecond, 0)
                    draggingKeyframesDidCommit(__selectedKeyframes, timeIndex)
                }
                __draggingTime = false;
                __draggingTracksScrollThumb = false;
                __draggingTimeScale = false;
                __draggingTimeScrollThumb = false;
                canvas.selectRegion = Qt.rect(0, 0, 0, 0)
                canvas.requestPaint();
            }
            onWheel: {
                var delta = wheel.pixelDelta,
                        deltaX = delta.x, deltaY = delta.y,
                        isOSX = Qt.platform.os === "osx"
                if (deltaX !== 0) {
                    canvas.scrollTimeSecondsOffsetDelta(deltaX * (isOSX ? -1 : 1))
                }
                else if (deltaY !== 0) {
                    canvas.scrollTracksOffsetDelta(deltaY * (isOSX ? -1 : 1))
                }
            }
        }
        Keys.onPressed: {
            var selectedKeyframes = __selectedKeyframes, key = event.key, newSelectedKeyframe = null, delta = 0;
            if (selectedKeyframes.length === 1) {
                var keyframe = selectedKeyframes[0];
                var track = findTrack(keyframe.opaque);
                var index = track.keyframes.indexOf(keyframe);
                if (key === Qt.Key_Left && index > 0) {
                    var previousKeyframe = track.keyframes[index - 1];
                    newSelectedKeyframe = previousKeyframe;
                }
                else if (key === Qt.Key_Right && (index + 1) < track.keyframes.length) {
                    var nextKeyframe = track.keyframes[index + 1];
                    newSelectedKeyframe =  nextKeyframe;
                }
                if (newSelectedKeyframe) {
                    __selectedKeyframes = [ newSelectedKeyframe ];
                    timeSeconds = newSelectedKeyframe.timeIndex / framesPerSecond
                }
                canvas.requestPaint()
            }
            else if (key === Qt.Key_Left) {
                delta = (event.modifiers & Qt.ShiftModifier) ? -5 : -1
            }
            else if (key === Qt.Key_Right) {
                delta = (event.modifiers & Qt.ShiftModifier) ? 5 : 1
            }
            if (delta !== 0) {
                timeSeconds = Math.min(Math.max((timeIndex + delta) / framesPerSecond, 0), durationSeconds)
                scrollTimeIndexOffsetDelta(delta)
            }
        }

        function drawTrack(ctx, track, y, visibleTime) {
            var xshift = 5,
                    canvasWidth = canvas.width,
                    timelineTrackLabelWidth = __trackLabelWidth,
                    trackKeyframes = track.keyframes,
                    trackLabelHeight = __trackLabelHeight,
                    halfTrackLabelHeight = trackLabelHeight * 0.5,
                    fontAwesomeFont = [ iconPointSize, fontAwesome.name ].join(" "),
                    defaultFont = [ fontPointSizeText, fontFamily ].join(" "),
                    parentMotionTrack = track.parentMotionTrack,
                    trackType = track.type,
                    iconText, measuredWidth;

            // draw rectangle of label and track region
            ctx.font = fontAwesomeFont;
            if (trackType === "property" && track.selected) {
                ctx.fillStyle = selectedLabelFillColor
                propertyLabelFillTextColor = selectedLabelStrokeColor
            }
            else {
                ctx.fillStyle = backgroundFillColor
                propertyLabelFillTextColor = propertyLabelFillTextColor
            }
            ctx.fillRect(0, y - trackLabelHeight + 1, canvasWidth, trackLabelHeight)

            if (trackType === "object") {
                // object track header background
                drawRect(ctx, 0, y - trackLabelHeight + 1, timelineTrackLabelWidth, trackLabelHeight - 1, objectLabelFillColor);
                ctx.fillStyle = objectLabelStrokeColor;
                iconText = (track.collapsed ? "\uf152" : "\uf150");
                measuredWidth = ctx.measureText(iconText).width;
                ctx.fillText(iconText, xshift, y - trackLabelHeight / 4);
                xshift += measuredWidth + 5;
            }
            else if (parentMotionTrack) {
                // property track
                ctx.fillStyle = propertyLabelFillTextColor;
                iconText = parentMotionTrack.visible ? "\uf06e" : "\uf070"; // icon-eye-open and icon-eye-close
                measuredWidth = ctx.measureText(iconText).width;
                ctx.fillText(iconText, xshift, y - trackLabelHeight / 4);
                xshift += measuredWidth + 5;
                iconText = parentMotionTrack.locked ? "\uf023" : "\uf09c"; // icon-lock and icon-unlock
                ctx.fillText(iconText, xshift, y - trackLabelHeight / 4);
                xshift += measuredWidth + 5
            }
            else {
                ctx.fillStyle = propertyLabelFillTextColor;
                xshift += 15
            }
            ctx.font = defaultFont

            //bottom track line
            drawLine(ctx, 0, y, canvasWidth, y, bottomTrackLineStrokeColor);
            //draw track label
            ctx.fillText(track.name, xshift, y - trackLabelHeight / 4);

            // if it's property track then draw animations
            if (trackType === "property") {
                // memoize
                var animationEnd = durationSeconds,
                        trackWidth = __trackWidth,
                        timeScrollX = __timeScrollX,
                        timeShift = Math.max(0, (animationEnd - visibleTime) * timeScrollX),
                        numKeyframes = trackKeyframes.length,
                        selectedKeyframes = __selectedKeyframes,
                        delta = 0;
                if (visibleTime < animationEnd) {
                    delta = (animationEnd - visibleTime) * timeScrollX
                }
                function memoizedTimeToX(time) {
                    return timelineTrackLabelWidth + (time - delta) * trackWidth + 10;
                }
                var timeScrollableX = timeScrollX + canvasWidth;
                for (var i = 0; i < numKeyframes; i++) {
                    var keyframe = trackKeyframes[i],
                            selected = (selectedKeyframes.indexOf(keyframe) > -1),
                            first = (i === 0), last = (i === numKeyframes - 1),
                            x = memoizedTimeToX(keyframe.time), y2 = y - halfTrackLabelHeight;
                    if (x >= timeScrollX && x < timeScrollableX) {
                        drawRombus(ctx, x, y2, halfTrackLabelHeight, halfTrackLabelHeight, rombusBaseFillColor, true, true, selected ? rombusSelectedStrokeColor : rombusBaseStrokeColor);
                        drawRombus(ctx, x, y2, halfTrackLabelHeight, halfTrackLabelHeight, rombusFillColor, !first, !last);
                    }
                }
            }
        }

        function drawLine(ctx, x1, y1, x2, y2, color) {
            ctx.strokeStyle = color;
            ctx.beginPath();
            ctx.moveTo(x1 + 0.5, y1 + 0.5);
            ctx.lineTo(x2 + 0.5, y2 + 0.5);
            ctx.stroke();
        }
        function drawRect(ctx, x, y, w, h, color) {
            ctx.fillStyle = color;
            ctx.fillRect(x, y, w, h);
        }
        function drawCenteredRect(ctx, x, y, w, h, color) {
            ctx.fillStyle = color;
            ctx.fillRect(x - w / 2, y - h / 2, w, h);
        }
        function drawRombus(ctx, x, y, w, h, color, drawLeft, drawRight, strokeColor) {
            ctx.fillStyle = color;
            if (strokeColor) {
                ctx.lineWidth = 2;
                ctx.strokeStyle = strokeColor;
                ctx.beginPath();
                ctx.moveTo(x, y - h / 2);
                ctx.lineTo(x + w / 2, y);
                ctx.lineTo(x, y + h/2);
                ctx.lineTo(x - w / 2, y);
                ctx.lineTo(x, y - h / 2);
                ctx.stroke();
                ctx.lineWidth = 1;
            }
            if (drawLeft) {
                ctx.beginPath();
                ctx.moveTo(x, y - h / 2);
                ctx.lineTo(x - w / 2, y);
                ctx.lineTo(x, y + h / 2);
                ctx.fill();
            }
            if (drawRight) {
                ctx.beginPath();
                ctx.moveTo(x, y - h / 2);
                ctx.lineTo(x + w / 2, y);
                ctx.lineTo(x, y + h / 2);
                ctx.fill();
            }
        }
        renderStrategy: Canvas.Cooperative
        onCanvasSizeChanged: requestPaint()
        onPaint: {
            var ctx = getContext("2d"), canvasWidth = canvas.width, canvasHeight = canvas.height,
                    timelineTrackLabelWidth = __trackLabelWidth,
                    timelineHeaderHeight = __headerHeight;

            __timeScrollBarWidth = canvasWidth - timelineTrackLabelWidth - __tracksScrollBarWidth;
            var animationEnd = durationSeconds;
            var visibleTime = timeline.xToTimeSeconds(canvasWidth - timelineTrackLabelWidth - __tracksScrollBarWidth) - timeline.xToTimeSeconds(0); //100 to get some space after lask key
            var timeScrollRatio = Math.max(0, Math.min(visibleTime/animationEnd, 1));
            __timeScrollThumbWidth = Math.max(timeScrollRatio * __timeScrollBarWidth, __timeScrollThumbMinimumWidth);
            if (__timeScrollThumbPos + __timeScrollThumbWidth > __timeScrollBarWidth) {
                __timeScrollThumbPos = Math.max(0, __timeScrollBarWidth - __timeScrollThumbWidth);
            }

            // tracks area clipping path
            ctx.clearRect(0, 0, canvasWidth, canvasHeight);
            ctx.save();
            ctx.font = "%1 %2".arg(fontPointSizeText).arg(fontFamily)

            /*
            ctx.beginPath();
            ctx.moveTo(0, timelineHeaderHeight + 1);
            ctx.lineTo(w, timelineHeaderHeight + 1);
            ctx.lineTo(w, h - timeline.timeScrollHeight);
            ctx.lineTo(0, h - timeline.timeScrollHeight);
            ctx.clip();
            */

            var tracks = __tracks,
                    numTracks = tracks.length,
                    trackLabelHeight = __trackLabelHeight,
                    visibleTracks = getVisibleTracks(),
                    track,
                    i = 0;
            var numVisibleTracks = visibleTracks.length;
            for (i = 0; i < numVisibleTracks; i++) {
                track = visibleTracks[i];
                var yshift = timelineHeaderHeight + trackLabelHeight * (i + 1);
                var scrollY = __tracksScrollY * (numVisibleTracks * trackLabelHeight - canvasHeight + timelineHeaderHeight);
                yshift -= scrollY;
                if (track.type === null || yshift < timelineHeaderHeight) {
                    continue;
                }
                drawTrack(ctx, track, yshift, visibleTime);
            }
            ctx.restore();

            __tracksScrollBarHeight = canvasHeight - timelineHeaderHeight - __timeScrollBarHeight;
            var totalTracksHeight = numVisibleTracks * __trackLabelHeight;
            var tracksScrollRatio = __tracksScrollBarHeight / totalTracksHeight;
            __tracksScrollThumbHeight = Math.min(Math.max(__tracksScrollBarHeight * tracksScrollRatio, __tracksScrollThumbMinimumHeight), __tracksScrollBarHeight);

            // end of label panel
            drawLine(ctx, timelineTrackLabelWidth, 0, timelineTrackLabelWidth, canvasHeight, labelPanelStrokeColor);

            // timeline
            var timelineStart = 0, timelineEnd = 10, lastTimeLabelX = 0;

            ctx.fillStyle = timeScaleFillColor;
            var x = timeline.timeSecondsToX(0), sec = timelineStart;
            while (x < canvasWidth) {
                x = timeline.timeSecondsToX(sec);
                drawLine(ctx, x, 0, x, timelineHeaderHeight * 0.3, timeScaleStrokeColor);
                var minutes = Math.floor(sec / 60),
                        seconds = sec % 60,
                        time = minutes + ":" + ((seconds < 10) ? "0" : "") + seconds;
                if (x - lastTimeLabelX > 30) {
                    ctx.fillText(time, x - 6, timelineHeaderHeight * 0.8);
                    lastTimeLabelX = x;
                }
                sec += 1;
            }

            // time ticker
            var currentTimeX = timeline.timeSecondsToX(timeline.timeSeconds)
            drawLine(ctx, currentTimeX, 0, currentTimeX, canvasHeight, timeTickerStrokeColor);

            //time scale
            for (i = 2; i < 20; i++) {
                var f = 1.0 - (i * i) / 361;
                drawLine(ctx, 7 + f * (timelineTrackLabelWidth - 10), canvasHeight - __timeScrollBarHeight + 4, 7 + f * (timelineTrackLabelWidth - 10), canvasHeight - 3, timeScaleStrokeColor);
            }
            var timeScale = 1.0 - timeScaleFactor;
            ctx.fillStyle = timeScaleFillColor;
            ctx.beginPath();
            ctx.moveTo(7  + timeScale * (timelineTrackLabelWidth - 10), canvasHeight - 7);
            ctx.lineTo(11 + timeScale * (timelineTrackLabelWidth - 10), canvasHeight - 1);
            ctx.lineTo(3  + timeScale * (timelineTrackLabelWidth - 10), canvasHeight - 1);
            ctx.fill();

            // tracks scrollbar
            var tracksScrollX = canvasWidth - __tracksScrollBarWidth,
                    tracksScrollY = timelineHeaderHeight + 1,
                    tracksScrollWidth = __tracksScrollBarWidth;
            drawRect(ctx, tracksScrollX, tracksScrollY, tracksScrollWidth, __tracksScrollBarHeight, tracksScrollBarFillColor);
            if (__tracksScrollThumbHeight < __tracksScrollBarHeight) {
                drawRect(ctx, tracksScrollX, tracksScrollY + __tracksScrollThumbPos, tracksScrollWidth, __tracksScrollThumbHeight, tracksScrollBarStrokeColor);
            }

            // time scrollbar
            var timeScrollHeight = __timeScrollBarHeight;
            drawRect(ctx, timelineTrackLabelWidth, canvasHeight - timeScrollHeight, canvasWidth - timelineTrackLabelWidth - __tracksScrollBarWidth, timeScrollHeight, timeScrollBarFillColor);
            if (__timeScrollThumbWidth < __timeScrollBarWidth) {
                drawRect(ctx, timelineTrackLabelWidth + 1 + __timeScrollThumbPos, canvasHeight - timeScrollHeight, __timeScrollThumbWidth, timeScrollHeight, timeScrollBarStrokeColor);
            }

            // header borders
            drawLine(ctx, 0, 0, canvasWidth, 0, headerBorderStrokeColor);
            drawLine(ctx, 0, timelineHeaderHeight, canvasWidth, timelineHeaderHeight, headerBorderStrokeColor);
            drawLine(ctx, 0, canvasHeight - timeScrollHeight, timelineTrackLabelWidth, canvasHeight - timeScrollHeight, headerBorderStrokeColor);
            drawLine(ctx, timelineTrackLabelWidth, canvasHeight - timeScrollHeight - 1, timelineTrackLabelWidth, canvasHeight, headerBorderStrokeColor);
        }
        RowLayout {
            id: timelineButtons
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: 5
            property int rectWidth: 20
            property int rectHeight: 20
            Rectangle {
                width: parent.rectWidth
                height: parent.rectHeight
                border { color: "black"; width: 1 }
                Text {
                    anchors.centerIn: parent
                    font { family: fontAwesome.name; pointSize: iconPointSize }
                    text: FontAwesome.Icon.CircleArrowLeft
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: timelineWillHide()
                }
            }
            Rectangle {
                width: parent.rectWidth
                height: parent.rectHeight
                border { color: "black"; width: 1 }
                Text {
                    anchors.centerIn: parent
                    font { family: fontAwesome.name; pointSize: iconPointSize }
                    text: FontAwesome.Icon.Play
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: timelineWillPlay()
                }
            }
            Rectangle {
                width: parent.rectWidth
                height: parent.rectHeight
                border { color: "black"; width: 1 }
                Text {
                    anchors.centerIn: parent
                    font { family: fontAwesome.name; pointSize: iconPointSize }
                    text: FontAwesome.Icon.Pause
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: timelineWillPause()
                }
            }
            Rectangle {
                width: parent.rectWidth
                height: parent.rectHeight
                border { color: "black"; width: 1 }
                Text {
                    anchors.centerIn: parent
                    font { family: fontAwesome.name; pointSize: iconPointSize }
                    text: FontAwesome.Icon.Stop
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: timelineWillStop()
                }
            }
        }
        Image {
            id: backgroundImage
            fillMode: Image.Pad
            x: parent.width - sourceSize.width
            y: parent.height - sourceSize.height
            opacity: 0.5
            visible: false
            z: parent.z - 1
            asynchronous: true
            onStatusChanged: visible = status === Image.Ready
        }
    }
}
