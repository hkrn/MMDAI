/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* headers */

#ifndef MMDAI_ISCENEEVENTHANDLER_H_
#define MMDAI_ISCENEEVENTHANDLER_H_

namespace MMDAI {

/* event names */

class ISceneEventHandler
{
public:
    static const char *kModelAddCommand;
    static const char *kModelChangeCommand;
    static const char *kModelDeleteCommand;
    static const char *kMotionAddCommand;
    static const char *kMotionChangeCommand;
    static const char *kMotionDeleteCommand;
    static const char *kMoveStartCommand;
    static const char *kMoveStopCommand;
    static const char *kTurnStartCommand;
    static const char *kTurnStopCommand;
    static const char *kRotateStartCommand;
    static const char *kRotateStopCommand;
    static const char *kStageCommand;
    static const char *kFloorCommand;
    static const char *kBackgroundCommand;
    static const char *kLightColorCommand;
    static const char *kLightDirectionCommand;
    static const char *kLipSyncStartCommand;
    static const char *kLipSyncStopCommand;
    static const char *kCameraCommand;

    static const char *kModelAddEvent;
    static const char *kModelChangeEvent;
    static const char *kModelDeleteEvent;
    static const char *kMotionAddEvent;
    static const char *kMotionChangeEvent;
    static const char *kMotionDeleteEvent;
    static const char *kMotionLoopEvent;
    static const char *kMoveStartEvent;
    static const char *kMoveStopEvent;
    static const char *kTurnStartEvent;
    static const char *kTurnStopEvent;
    static const char *kRotateStartEvent;
    static const char *kRotateStopEvent;
    static const char *kStageEvent;
    static const char *kFloorEvent;
    static const char *kBackgroundEvent;
    static const char *kLightColorEvent;
    static const char *kLightDirectionEvent;
    static const char *kLipSyncStartEvent;
    static const char *kLipSyncStopEvent;
    static const char *kKeyEvent;

    virtual ~ISceneEventHandler() {}

    virtual void handleEventMessage(const char *eventType, int argc, ...) = 0;
};

} /* namespace */

#endif // MMDAI_ISCENEEVENTHANDLER_H_

