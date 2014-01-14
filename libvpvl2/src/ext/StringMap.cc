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

#include <vpvl2/extensions/StringMap.h>
#include <vpvl2/extensions/StringUtil.h>

namespace vpvl2
{
namespace extensions
{

StringMap::StringMap()
    : std::map<const std::string, std::string>()
{
}

StringMap::~StringMap()
{
}

bool StringMap::booleanValue(const std::string &key, bool defval) const
{
    const_iterator it = find(key);
    return it != end() ? StringUtil::toBoolean(it->second) : defval;
}

int StringMap::integerValue(const std::string &key, int defval) const
{
    const_iterator it = find(key);
    return it != end() ? StringUtil::toInt(it->second) : defval;
}
double StringMap::doubleValue(const std::string &key, double defval) const
{
    const_iterator it = find(key);
    return it != end() ? StringUtil::toDouble(it->second) : defval;
}

float StringMap::floatValue(const std::string &key, float defval) const
{
    return float(doubleValue(key, defval));
}

std::string StringMap::stringValue(const std::string &key, const std::string &defval) const
{
    const_iterator it = find(key);
    return it != end() ? it->second : defval;
}

bool StringMap::tryGetValue(const std::string &key, std::string &value) const
{
    const_iterator it = find(key);
    if (it != end()) {
        value = it->second;
        return true;
    }
    else {
        value.assign(std::string());
        return false;
    }
}

void StringMap::copyTo(StringMap &value) const
{
    value.clear();
    for (const_iterator it = begin(); it != end(); ++it) {
        value.insert(std::make_pair(it->first, it->second));
        VPVL2_VLOG(1, it->first << ":" << it->second);
    }
}

} /* namespace extensions */
} /* namespace vpvl2 */
