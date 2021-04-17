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

#include "rgy_filter.h"
#include "rgy_prm.h"
#include <array>

class RGYFilterParamWarpsharp : public RGYFilterParam {
public:
    VppWarpsharp warpsharp;
    RGYFilterParamWarpsharp() : warpsharp() {};
    virtual ~RGYFilterParamWarpsharp() {};
    virtual tstring print() const override { return warpsharp.print(); };
};

class RGYFilterWarpsharp : public RGYFilter {
public:
    RGYFilterWarpsharp(shared_ptr<RGYOpenCLContext> context);
    virtual ~RGYFilterWarpsharp();
    virtual RGY_ERR init(shared_ptr<RGYFilterParam> pParam, shared_ptr<RGYLog> pPrintMes) override;
protected:
    virtual RGY_ERR run_filter(const FrameInfo *pInputFrame, FrameInfo **ppOutputFrames, int *pOutputFrameNum, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event) override;
    virtual void close() override;

    RGY_ERR checkParam(const std::shared_ptr<RGYFilterParamWarpsharp> prm);

    RGY_ERR procPlaneSobel(FrameInfo *pOutputPlane, const FrameInfo *pInputPlane, const float threshold, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);
    RGY_ERR procPlaneBlur(FrameInfo *pOutputPlane, const FrameInfo *pInputPlane, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);
    RGY_ERR procPlaneWarp(FrameInfo *pOutputPlane, const FrameInfo *pInputMask, const FrameInfo *pInputPlaneImg, const float depth, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);
    RGY_ERR procPlaneDowscale(FrameInfo *pOutputPlane, const FrameInfo *pInputPlane, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);
    virtual RGY_ERR procPlane(FrameInfo *pOutputPlane, FrameInfo *pMaskPlane0, FrameInfo *pMaskPlane1, const FrameInfo *pInputPlane, const FrameInfo *pInputPlaneImg, const float threshold, const float depth, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);
    virtual RGY_ERR procFrame(FrameInfo *pOutputPlane, const FrameInfo *pInputPlane, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);

    bool m_bInterlacedWarn;
    unique_ptr<RGYOpenCLProgram> m_warpsharp;
    unique_ptr<RGYCLFrame> m_srcImage;
    std::array<std::unique_ptr<RGYCLFrame>, 2> m_mask;
};
