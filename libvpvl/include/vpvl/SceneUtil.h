/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#ifndef VPVL_SCENEUTIL_H_
#define VPVL_SCENEUTIL_H_

#include "vpvl/Common.h"

namespace vpvl
{

class VPVL_API SceneUtil
{
public:
    static void perspective(const Scalar &fovy,
                            const Scalar &aspect,
                            const Scalar &znear,
                            const Scalar &zfar,
                            float matrix[16])
    {
        const Scalar &top = znear * btTan(radian(fovy * 0.5f));
        const Scalar &bottom = -top;
        const Scalar &right = aspect * top;
        const Scalar &left = -right;
        frustum(left, right, bottom, top, znear, zfar, matrix);
    }
    static void frustum(const Scalar &left,
                        const Scalar &right,
                        const Scalar &bottom,
                        const Scalar &top,
                        const Scalar &znear,
                        const Scalar &zfar,
                        float matrix[16])
    {
        const float &znear2 = znear * 2.0f;
        const float &r2l = (right - left);
        const float &t2b = (top - bottom);
        const float &f2n = (zfar - znear);
        const float &a = znear2 / r2l;
        const float &b = znear2 / t2b;
        const float &c = (right + left) / r2l;
        const float &d = (top + bottom) / t2b;
        const float &e = -(zfar + znear) / f2n;
        const float &f = -znear2 * zfar / f2n;
        const float m[16] = {
            a, 0, 0, 0,
            0, b, 0, 0,
            c, d, e, -1,
            0, 0, f, 0
        };
        memcpy(matrix, m, sizeof(m));
    }
    static void ortho(const Scalar &left,
                      const Scalar &right,
                      const Scalar &bottom,
                      const Scalar &top,
                      const Scalar &znear,
                      const Scalar &zfar,
                      float matrix[16])
    {
        const float &r2l = (right - left);
        const float &t2b = (top - bottom);
        const float &f2n = (zfar - znear);
        const float &a = 2.0f / r2l;
        const float &b = 2.0f / t2b;
        const float &c = -2.0f / f2n;
        const float &d = (right + left) / r2l;
        const float &e = (top + bottom) / t2b;
        const float &f = (zfar + znear) / f2n;
        const float m[16] = {
            a, 0, 0, d,
            0, b, 0, e,
            0, 0, c, f,
            0, 0, 0, 1
        };
        memcpy(matrix, m, sizeof(m));
    }

private:
    SceneUtil();
    ~SceneUtil();

    VPVL_DISABLE_COPY_AND_ASSIGN(SceneUtil)
};

}

#endif

