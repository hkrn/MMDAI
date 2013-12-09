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
#ifndef VPVL2_EXTENSIONS_STRINGMAP_H_
#define VPVL2_EXTENSIONS_STRINGMAP_H_

#include <vpvl2/config.h>

#include <map>
#include <string>

namespace vpvl2
{
namespace extensions
{

class VPVL2_API StringMap VPVL2_DECL_FINAL : public std::map<const std::string, std::string> {
public:
    StringMap();
    ~StringMap();

    bool booleanValue(const std::string &key, bool defval) const;
    int integerValue(const std::string &key, int defval) const;
    double doubleValue(const std::string &key, double defval) const;
    float floatValue(const std::string &key, float defval) const;
    std::string stringValue(const std::string &key, const std::string &defval) const;
    bool tryGetValue(const std::string &key, std::string &value) const;
    void copyTo(StringMap &value) const;

    inline bool value(const std::string &key, bool defval = false) const {
        return booleanValue(key, defval);
    }
    inline int value(const std::string &key, int defval = 0) const {
        return integerValue(key, defval);
    }
    inline double value(const std::string &key, double defval = 0.0) const {
        return doubleValue(key, defval);
    }
    inline float value(const std::string &key, float defval = 0.0f) const {
        return floatValue(key, defval);
    }
    inline std::string value(const std::string &key, const std::string &defval = std::string()) const {
        return stringValue(key, defval);
    }

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(StringMap)
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif
