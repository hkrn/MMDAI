static float4 matrixMultVector4(float16 *m, float4 *v)
{
    return (float4)(
       dot(m->s048c, *v) + m->s3,
       dot(m->s159d, *v) + m->s7,
       dot(m->s26ae, *v) + m->sb,
       1.0
    );
}

static float4 matrixMultVector3(float16 *m, float4 *v)
{
    return (float4)(
       dot((float4)(m->s048, 0.0), *v),
       dot((float4)(m->s159, 0.0), *v),
       dot((float4)(m->s26a, 0.0), *v),
       0.0
    );
}

__kernel
void updateBoneMatrices(const __global float16 *boneMatrices,
                        const __global float16 *originMatrices,
                        const int nbones,
                        __global float16 *outputMatrices)
{
    int id = get_global_id(0);
    if (id < nbones)
        outputMatrices[id] = boneMatrices[id] * originMatrices[id];
}

__kernel
void performSkinning(const __global float16 *skinningMatrices,
                     const __global float *weights,
                     const __global int *bone1Indices,
                     const __global int *bone2Indices,
                     const float4 lightDirection,
                     const int nvertices,
                     const int strideSize,
                     const int offsetPosition,
                     const int offsetNormal,
                     const int offsetToonTexCoord,
                     const int offsetEdge,
                     __global float4 *vertices)
{
    int id = get_global_id(0);
    if (id < nvertices) {
        int strideOffset = strideSize * id;
        float4 position = vertices[strideOffset + offsetPosition];
        float4 normal = vertices[strideOffset + offsetNormal];
        float weight = weights[id];
        int bone1 = bone1Indices[id];
        int bone2 = bone2Indices[id];
        float16 transform1 = skinningMatrices[bone1];
        float16 transform2 = skinningMatrices[bone2];
        float4 v1 = matrixMultVector4(&transform1, &position);
        float4 v2 = matrixMultVector4(&transform2, &position);
        float4 n1 = matrixMultVector3(&transform1, &normal);
        float4 n2 = matrixMultVector3(&transform2, &normal);
        float s = 1.0f - weight;
        float4 position2 = s * v2 + weight * v1;
        float4 normal2 = s * n2 + weight * n1;
        vertices[strideOffset + offsetPosition] = position2;
        vertices[strideOffset + offsetNormal] = normal2;
        vertices[strideOffset + offsetToonTexCoord].w = 0.5 + dot(lightDirection, normal2) * 0.5;
        vertices[strideOffset + offsetEdge] = position2 + normal2 * vertices[strideOffset + offsetEdge];
    }
}

