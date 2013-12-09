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

#pragma once
#ifndef VPVL2_EXTENSIONS_STRINGUTIL_H_
#define VPVL2_EXTENSIONS_STRINGUTIL_H_

#include <vpvl2/config.h>

namespace vpvl2
{
namespace extensions
{

class VPVL2_API StringUtil VPVL2_DECL_FINAL {
public:
    static bool toBoolean(const std::string &value) {
        return value == "true" || value == "1" || value == "y" || value == "yes";
    }
    static int toInt(const std::string &value, int def = 0) {
        int v = int(strtol(value.c_str(), 0, 10));
        return v != 0 ? v : def;
    }
    static double toDouble(const std::string &value, double def = 0.0) {
        double v = strtod(value.c_str(), 0);
        return v != 0.0 ? float(v) : def;
    }

private:
    VPVL2_MAKE_STATIC_CLASS(StringUtil)
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif
