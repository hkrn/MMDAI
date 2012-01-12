float4 matrixMultVector4(float16 *m, float4 *v);
float4 matrixMultVector3(float16 *m, float4 *v);

float4 matrixMultVector4(float16 *m, float4 *v)
{
    return (float4)(
       m->s0 * v->x + m->s1 * v->y + m->s2 * v->z + m->s3 * v->w,
       m->s4 * v->x + m->s5 * v->y + m->s6 * v->z + m->s7 * v->w,
       m->s8 * v->x + m->s9 * v->y + m->sa * v->z + m->sb * v->w,
       m->sc * v->x + m->sd * v->y + m->se * v->z + m->sf * v->w
    );
}

float4 matrixMultVector3(float16 *m, float4 *v)
{
    return (float4)(
       m->s0 * v->x + m->s1 * v->y + m->s2 * v->z,
       m->s4 * v->x + m->s5 * v->y + m->s6 * v->z,
       m->s8 * v->x + m->s9 * v->y + m->sa * v->z,
       0.0f
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
                     const int nvertices,
                     const int strideSize,
                     const int offsetPosition,
                     const int offsetNormal,
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
        vertices[strideOffset + offsetPosition] = s * v2 + weight * v1;
        vertices[strideOffset + offsetNormal] = s * n2 + weight * n1;
    }
}

