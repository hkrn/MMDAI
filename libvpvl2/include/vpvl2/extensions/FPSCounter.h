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

#pragma once
#ifndef VPVL2_EXTENSIONS_FPSCOUNTER_H_
#define VPVL2_EXTENSIONS_FPSCOUNTER_H_

#include <vpvl2/Common.h>

namespace vpvl2
{
namespace extensions
{

class FPSCounter VPVL2_DECL_FINAL {
public:
    FPSCounter()
        : m_value(0),
          m_accumulated(0),
          m_updated(0)
    {
    }
    ~FPSCounter() {
        reset();
    }

    void update(int64 elapsed, bool &flushed) {
        if (abs(m_updated - elapsed) > 1000) {
            m_value = m_accumulated;
            m_accumulated = 0;
            m_updated = elapsed;
            flushed = true;
        }
        else {
            flushed = false;
        }
        m_accumulated++;
    }
    void reset() {
        m_value = 0;
        m_accumulated = 0;
        m_updated = 0;
    }
    int value() const {
        return m_value;
    }

private:
    int m_value;
    int m_accumulated;
    int64 m_updated;
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif
