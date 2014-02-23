#include "../Common.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/icu4c/Encoding.h"
#include "vpvl2/extensions/icu4c/String.h"
#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"

#include "../mock/Bone.h"
#include "../mock/Joint.h"
#include "../mock/Label.h"
#include "../mock/Material.h"
#include "../mock/Morph.h"
#include "../mock/RigidBody.h"
#include "../mock/Vertex.h"

using namespace std::tr1;
using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::pmx;

static inline void SetVertex(Vertex &vertex, Vertex::Type type, const Array<Bone *> &bones)
{
    vertex.setOrigin(Vector3(0.01, 0.02, 0.03));
    vertex.setNormal(Vector3(0.11, 0.12, 0.13));
    vertex.setTextureCoord(Vector3(0.21, 0.22, 0.0));
    vertex.setOriginUV(0, Vector4(0.31, 0.32, 0.33, 0.34));
    vertex.setType(type);
    vertex.setEdgeSize(0.1);
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        vertex.setBoneRef(i, bones[i]);
        vertex.setWeight(i, 0.2 + 0.1 * i);
    }
    vertex.setSdefC(Vector3(0.41, 0.42, 0.43));
    vertex.setSdefR0(Vector3(0.51, 0.52, 0.53));
    vertex.setSdefR1(Vector3(0.61, 0.62, 0.63));
}

class PMXFragmentTest : public TestWithParam<vsize> {};

class PMXFragmentWithUVTest : public TestWithParam< tuple<vsize, pmx::Morph::Type > > {};

class PMXLanguageTest : public TestWithParam<IEncoding::LanguageType> {};
