﻿
// Type
// bit_depth
// knn_radius

#ifndef clamp
#define clamp(x, low, high) (((x) <= (high)) ? (((x) >= (low)) ? (x) : (low)) : (high))
#endif

float lerpf(float v0, float v1, float t) {
    float tmp = (1.0-t)*v0;
    return tmp + t*v1;
}

__kernel void kernel_denoise_knn(
    __global uchar *restrict pDst,
    const int dstPitch, const int dstWidth, const int dstHeight,
    __read_only image2d_t src,
    const float strength, const float lerpC, const float weight_threshold, const float lerp_threshold) {
    const float knn_window_area = (float)((2 * knn_radius + 1) * (2 * knn_radius + 1));
    const float inv_knn_window_area = 1.0f / knn_window_area;
    const int ix = get_global_id(0);
    const int iy = get_global_id(1);
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
    if (ix < dstWidth && iy < dstHeight) {
        const float sx = (float)ix + 0.5f;
        const float sy = (float)iy + 0.5f;

        float fCount = 0.0f;
        float sumWeights = 0.0f;
        float sum = 0.0f;
        float center = (float)read_imagef(src, sampler, (int2)(sx, sy)).x;

        #pragma unroll
        for (float i = -knn_radius; i <= knn_radius; i++) {
            #pragma unroll
            for (float j = -knn_radius; j <= knn_radius; j++) {
                float clrIJ = (float)read_imagef(src, sampler, (int2)(sx+i, sy+j)).x;
                float distanceIJ = (center - clrIJ) * (center - clrIJ);

                float weightIJ = native_exp(-(distanceIJ * strength + (i * i + j * j) * inv_knn_window_area));

                sum += clrIJ * weightIJ;

                sumWeights += weightIJ;

                fCount += (weightIJ > weight_threshold) ? inv_knn_window_area : 0;
            }
        }
        float lerpQ = (fCount > lerp_threshold) ? lerpC : 1.0f - lerpC;

        __global Type *ptr = (__global Type *)(pDst + iy * dstPitch + ix * sizeof(Type));
        ptr[0] = (Type)clamp(lerpf(sum * native_recip(sumWeights), center, lerpQ) * (float)((1<<bit_depth)-1), 0.0f, (1<<bit_depth) - 0.1f);
    }
}
