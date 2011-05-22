#ifndef VPVL_IPARSER_H_
#define VPVL_IPARSER_H_

namespace vpvl
{

class IParser
{
public:
    virtual ~IParser() {}
    virtual bool preparse() = 0;
};

} /* namespace vpvl */

#endif
