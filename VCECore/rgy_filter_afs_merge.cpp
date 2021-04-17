﻿// -----------------------------------------------------------------------------------------
// NVEnc by rigaya
// -----------------------------------------------------------------------------------------
//
// The MIT License
//
// Copyright (c) 2014-2016 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

#include <map>
#include <array>
#include <cmath>
#include "convert_csp.h"
#include "vce_filter_afs.h"

#define MERGE_BLOCK_INT_X  (32) //work groupサイズ(x) = スレッド数/work group
#define MERGE_BLOCK_Y       (8) //work groupサイズ(y) = スレッド数/work group
#define MERGE_BLOCK_LOOP_Y  (1) //work groupのy方向反復数


RGY_ERR RGYFilterAfs::build_merge_scan() {
    if (!m_mergeScan) {
        const auto options = strsprintf("-D Type=uint -D MERGE_BLOCK_INT_X=%d -D MERGE_BLOCK_Y=%d -D MERGE_BLOCK_LOOP_Y=%d",
            MERGE_BLOCK_INT_X, MERGE_BLOCK_Y, MERGE_BLOCK_LOOP_Y);
        m_mergeScan = m_cl->buildResource(_T("VCE_FILTER_AFS_MERGE_CL"), _T("EXE_DATA"), options.c_str());
        if (!m_mergeScan) {
            AddMessage(RGY_LOG_ERROR, _T("failed to load VCE_FILTER_AFS_MERGE_CL\n"));
            return RGY_ERR_OPENCL_CRUSH;
        }
    }
    return RGY_ERR_NONE;
}

template<typename Type>
RGY_ERR run_merge_scan(uint8_t *dst,
    uint8_t *sp0, uint8_t *sp1,
    const int srcWidth, const int srcPitch, const int srcHeight,
    unique_ptr<RGYCLBuf> &count_stripe, const VppAfs *pAfsPrm, RGYOpenCLQueue &queue, std::vector<RGYOpenCLEvent> &wait_event, RGYOpenCLEvent &event,
    RGYOpenCLProgram *mergeScan, RGYOpenCLContext *cl) {
    const RGYWorkSize local(MERGE_BLOCK_INT_X, MERGE_BLOCK_Y);
    const RGYWorkSize global(divCeil<int>(srcWidth, sizeof(Type)), divCeil<int>(srcHeight, MERGE_BLOCK_LOOP_Y));
    const auto grid_count = global.groups(local).total();
    if (!count_stripe || count_stripe->size() < grid_count * sizeof(int)) {
        count_stripe = cl->createBuffer(grid_count * sizeof(int), CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR);
        if (!count_stripe) {
            return RGY_ERR_MEMORY_ALLOC;
        }
    }
    const uint32_t scan_left   = pAfsPrm->clip.left / sizeof(Type);
    const uint32_t scan_width  = (srcWidth - pAfsPrm->clip.left - pAfsPrm->clip.right) / sizeof(Type);
    const uint32_t scan_top    = pAfsPrm->clip.top;
    const uint32_t scan_height = srcHeight - pAfsPrm->clip.top - pAfsPrm->clip.bottom;

    return mergeScan->kernel("kernel_afs_merge_scan").config(queue, local, global, wait_event, &event).launch(
        (cl_mem)dst, (cl_mem)count_stripe->mem(), (cl_mem)sp0, (cl_mem)sp1,
        divCeil<int>(srcWidth, sizeof(uint32_t)), divCeil<int>(srcPitch, sizeof(uint32_t)), srcHeight,
        pAfsPrm->tb_order ? 1 : 0,
        scan_left, scan_top, scan_width, scan_height
    );
}

RGY_ERR RGYFilterAfs::merge_scan(AFS_STRIPE_DATA *sp, AFS_SCAN_DATA *sp0, AFS_SCAN_DATA *sp1, unique_ptr<RGYCLBuf> &count_stripe, const RGYFilterParamAfs *pAfsParam, RGYOpenCLQueue &queue, std::vector<RGYOpenCLEvent> wait_event, RGYOpenCLEvent &event) {
    auto err = run_merge_scan<uint32_t>(
        sp->map->frame.ptr[0], sp0->map->frame.ptr[0], sp1->map->frame.ptr[0],
        sp1->map->frame.width, sp1->map->frame.pitch[0], sp1->map->frame.height,
        count_stripe, &pAfsParam->afs, queue, wait_event, event,
        m_mergeScan.get(), m_cl.get());
    if (err != RGY_ERR_NONE) {
        return err;
    }
    return RGY_ERR_NONE;
}
