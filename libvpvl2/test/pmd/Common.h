#include "../Common.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/icu4c/Encoding.h"
#include "vpvl2/extensions/icu4c/String.h"
#include "vpvl2/pmd2/Bone.h"
#include "vpvl2/pmd2/Joint.h"
#include "vpvl2/pmd2/Label.h"
#include "vpvl2/pmd2/Material.h"
#include "vpvl2/pmd2/Model.h"
#include "vpvl2/pmd2/Morph.h"
#include "vpvl2/pmd2/RigidBody.h"
#include "vpvl2/pmd2/Vertex.h"

#include "../mock/Bone.h"
#include "../mock/Joint.h"
#include "../mock/Label.h"
#include "../mock/Material.h"
#include "../mock/Morph.h"
#include "../mock/RigidBody.h"
#include "../mock/Vertex.h"

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::pmd2;

class PMDLanguageTest : public TestWithParam<IEncoding::LanguageType> {};
