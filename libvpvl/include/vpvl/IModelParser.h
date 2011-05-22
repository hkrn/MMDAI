#ifndef VPVL_IMODELPARSER_H_
#define VPVL_IMODELPARSER_H_

#include "vpvl/IParser.h"
#include "vpvl/IModel.h"

namespace vpvl
{

class IModelParser : IParser
{
public:
    virtual IModel *parse() = 0;
};

} /* namespace vpvl */

#endif
