static float4
matrixMultVector4(const float16 *m, const float4 *v)
{
    return (float4)(
       dot(m->s048c, *v) + m->s3,
       dot(m->s159d, *v) + m->s7,
       dot(m->s26ae, *v) + m->sb,
       1.0
    );
}

static float4
matrixMultVector3(const float16 *m, const float4 *v)
{
    return (float4)(
       dot((float4)(m->s048, 0.0), *v),
       dot((float4)(m->s159, 0.0), *v),
       dot((float4)(m->s26a, 0.0), *v),
       0.0
    );
}

static void
transformVertexBdef1(const float16 *transform,
                     const float4 *inPosition,
					 const float4 *inNormal,
					 __global float4 *outPosition,
					 __global float4 *outNormal)
{
    *outPosition = matrixMultVector4(transform, inPosition);
    *outNormal   = matrixMultVector3(transform, inNormal);
}

static void
transformVertexBdef2(const float16 *transformA,
					 const float16 *transformB,
                     const float4 *inPosition,
					 const float4 *inNormal,
					 const float weight,
					 __global float4 *outPosition,
					 __global float4 *outNormal)
{
    const float4 v1 = matrixMultVector4(transformA, inPosition);
    const float4 v2 = matrixMultVector4(transformB, inPosition);
    const float4 n1 = matrixMultVector3(transformA, inNormal);
    const float4 n2 = matrixMultVector3(transformB, inNormal);
    const float s = 1.0f - weight;
    *outPosition = s * v2 + weight * v1;
    *outNormal   = s * n2 + weight * n1;
}

__kernel void
performSkinning2(const __global float16 *localMatrices,
                 const __global float4 *boneWeights,
                 const __global int4 *boneIndices,
                 const __global float *materialEdgeSize,
                 const float4 lightDirection,
                 const float edgeScaleFactor,
                 const int nvertices,
                 const int strideSize,
                 const int offsetPosition,
                 const int offsetNormal,
                 const int offsetMorphDelta,
                 const int offsetEdgeVertex,
                 __global float4 *vertices)
{
    int id = get_global_id(0);
    if (id < nvertices) {
        const int strideOffset = strideSize * id;
        __global float4 *positionPtr = &vertices[strideOffset + offsetPosition];
        __global float4 *normalPtr = &vertices[strideOffset + offsetNormal];
        const float4 position4 = *positionPtr + vertices[strideOffset + offsetMorphDelta];
        const float4 normal4 = *normalPtr;
        const float vertexId = position4.w;
        const float edgeSize = normal4.w * materialEdgeSize[id] * edgeScaleFactor;
        const float4 position = (float4)(position4.xyz, 1.0);
        const float4 normal = (float4)(normal4.xyz, 0.0);
        const int4 boneIndex = boneIndices[id];
        const float4 weight = boneWeights[id];
        if (boneIndex.w >= 0) { // bdef4
            float16 transform1 = localMatrices[boneIndex.x];
            float16 transform2 = localMatrices[boneIndex.y];
            float16 transform3 = localMatrices[boneIndex.z];
            float16 transform4 = localMatrices[boneIndex.w];
            float4 v1 = matrixMultVector4(&transform1, &position);
            float4 v2 = matrixMultVector4(&transform2, &position);
            float4 v3 = matrixMultVector4(&transform3, &position);
            float4 v4 = matrixMultVector4(&transform4, &position);
            float4 n1 = matrixMultVector3(&transform1, &normal);
            float4 n2 = matrixMultVector3(&transform2, &normal);
            float4 n3 = matrixMultVector3(&transform3, &normal);
            float4 n4 = matrixMultVector3(&transform4, &normal);
            float4 position2 = weight.x * v1 + weight.y * v2 + weight.z * v3 + weight.w * v4;
            float4 normal2 = weight.x * n1 + weight.y * n2 + weight.z * n3 + weight.w * n4;;
            *positionPtr = position2;
            *normalPtr = normal2;
        }
        else if (boneIndex.y >= 0) { // bdef2 or sdef2
            const float w = weight.x;
            if (w == 1.0) {
                const float16 transform = localMatrices[boneIndex.x];
                transformVertexBdef1(&transform, &position, &normal, positionPtr, normalPtr);
            }
            else if (w == 0.0) {
                const float16 transform = localMatrices[boneIndex.y];
                transformVertexBdef1(&transform, &position, &normal, positionPtr, normalPtr);
            }
            else {
                const float16 transformA = localMatrices[boneIndex.x];
                const float16 transformB = localMatrices[boneIndex.y];
                transformVertexBdef2(&transformA, &transformB, &position, &normal, w, positionPtr, normalPtr);
            }
        }
        else { // bdef1
            const float16 transform = localMatrices[boneIndex.x];
            transformVertexBdef1(&transform, &position, &normal, positionPtr, normalPtr);
        }
        vertices[strideOffset + offsetPosition].w = vertexId;
        vertices[strideOffset + offsetEdgeVertex] = *positionPtr + *normalPtr * edgeSize;
    }
}

