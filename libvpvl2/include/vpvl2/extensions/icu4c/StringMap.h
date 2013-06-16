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
#ifndef VPVL2_EXTENSIONS_ICU_STRINGMAP_H_
#define VPVL2_EXTENSIONS_ICU_STRINGMAP_H_

#include <map>
#include <vpvl2/extensions/icu4c/String.h>

namespace vpvl2
{
namespace extensions
{
namespace icu4c
{

class VPVL2_API StringMap : public std::map<const UnicodeString, UnicodeString, String::Less> {
public:
    StringMap();
    ~StringMap();

    bool bval(const UnicodeString &key, bool defval) const;
    int ival(const UnicodeString &key, int defval) const;
    double dval(const UnicodeString &key, double defval) const;
    float fval(const UnicodeString &key, float defval) const;
    UnicodeString sval(const UnicodeString &key, const UnicodeString &defval) const;

    inline bool value(const UnicodeString &key, bool defval = false) const {
        return bval(key, defval);
    }
    inline int value(const UnicodeString &key, int defval = 0) const {
        return ival(key, defval);
    }
    inline double value(const UnicodeString &key, double defval = 0.0) const {
        return dval(key, defval);
    }
    inline float value(const UnicodeString &key, float defval = 0.0f) const {
        return fval(key, defval);
    }
    inline UnicodeString value(const UnicodeString &key, const UnicodeString &defval = UnicodeString()) const {
        return sval(key, defval);
    }

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(StringMap)
};

} /* namespace icu */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
