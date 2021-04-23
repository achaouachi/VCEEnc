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

#ifndef __RGY_OPENCL_H__
#define __RGY_OPENCL_H__

#include "rgy_version.h"

#if ENABLE_OPENCL

#define CL_TARGET_OPENCL_VERSION 210
#include <CL/opencl.h>
#include <CL/cl_dx9_media_sharing.h>
#include <CL/cl_d3d11.h>
#include <unordered_map>
#include <vector>
#include <array>
#include <memory>
#include <typeindex>
#include "rgy_osdep.h"
#include "rgy_log.h"
#include "rgy_util.h"

#ifndef CL_EXTERN
#define CL_EXTERN extern
#endif

#define RGYDefaultQueue 0

CL_EXTERN void *(CL_API_CALL *f_clGetExtensionFunctionAddressForPlatform)(cl_platform_id  platform, const char *funcname);

CL_EXTERN cl_int (CL_API_CALL* f_clGetPlatformIDs)(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms);
CL_EXTERN cl_int (CL_API_CALL* f_clGetPlatformInfo) (cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clGetDeviceIDs) (cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices);
CL_EXTERN cl_int (CL_API_CALL* f_clGetDeviceInfo) (cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_int (CL_API_CALL *f_clGetDeviceIDsFromDX9MediaAdapterKHR)(cl_platform_id platform, cl_uint num_media_adapters, cl_dx9_media_adapter_type_khr *media_adapter_type, void *media_adapters, cl_dx9_media_adapter_set_khr     media_adapter_set, cl_uint                          num_entries, cl_device_id *devices, cl_uint *num_devices);
CL_EXTERN cl_int (CL_API_CALL *f_clGetDeviceIDsFromD3D11KHR)(cl_platform_id platform, cl_d3d11_device_source_khr d3d_device_source, void *d3d_object, cl_d3d11_device_set_khr d3d_device_set, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices);

CL_EXTERN cl_context (CL_API_CALL* f_clCreateContext) (const cl_context_properties * properties, cl_uint num_devices, const cl_device_id * devices, void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *), void * user_data, cl_int * errcode_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clReleaseContext) (cl_context context);
CL_EXTERN cl_command_queue (CL_API_CALL* f_clCreateCommandQueue)(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int * errcode_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clGetCommandQueueInfo)(cl_command_queue command_queue, cl_command_queue_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clReleaseCommandQueue) (cl_command_queue command_queue);

CL_EXTERN cl_program(CL_API_CALL* f_clCreateProgramWithSource) (cl_context context, cl_uint count, const char **strings, const size_t *lengths, cl_int *errcode_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clBuildProgram) (cl_program program, cl_uint num_devices, const cl_device_id *device_list, const char *options, void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), void* user_data);
CL_EXTERN cl_int (CL_API_CALL* f_clGetProgramBuildInfo) (cl_program program, cl_device_id device, cl_program_build_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clGetProgramInfo)(cl_program program, cl_program_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clReleaseProgram) (cl_program program);

CL_EXTERN cl_mem (CL_API_CALL* f_clCreateBuffer) (cl_context context, cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret);
CL_EXTERN cl_mem (CL_API_CALL* f_clCreateImage)(cl_context context, cl_mem_flags flags, const cl_image_format *image_format, const cl_image_desc *image_desc, void *host_ptr, cl_int *errcode_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clReleaseMemObject) (cl_mem memobj);
CL_EXTERN cl_int (CL_API_CALL* f_clGetMemObjectInfo)(cl_mem memobj, cl_mem_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clGetImageInfo)(cl_mem memobj, cl_mem_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_kernel (CL_API_CALL* f_clCreateKernel) (cl_program program, const char *kernel_name, cl_int *errcode_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clReleaseKernel) (cl_kernel kernel);
CL_EXTERN cl_int (CL_API_CALL* f_clSetKernelArg) (cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value);
CL_EXTERN cl_int (CL_API_CALL* f_clEnqueueNDRangeKernel)(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event * event);
CL_EXTERN cl_int(CL_API_CALL* f_clEnqueueTask) (cl_command_queue command_queue, cl_kernel kernel, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

CL_EXTERN cl_int(CL_API_CALL* f_clEnqueueReadBuffer) (cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t size, void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL* f_clEnqueueReadBufferRect)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, const size_t *buffer_offset, const size_t *host_offset, const size_t *region, size_t buffer_row_pitch, size_t buffer_slice_pitch, size_t host_row_pitch, size_t host_slice_pitch, void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL* f_clEnqueueWriteBuffer) (cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t size, const void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueWriteBufferRect)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, const size_t *buffer_offset, const size_t *host_offset, const size_t *region, size_t buffer_row_pitch, size_t buffer_slice_pitch, size_t host_row_pitch, size_t host_slice_pitch, const void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueCopyBuffer)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, size_t src_offset, size_t dst_offset, size_t size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueCopyBufferRect)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, const size_t *src_origin, const size_t *dst_origin, const size_t *region, size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch, size_t dst_slice_pitch, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueReadImage)(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read, const size_t origin[3], const size_t region[3], size_t row_pitch, size_t slice_pitch, void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueWriteImage)(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t origin[3], const size_t region[3], size_t input_row_pitch, size_t input_slice_pitch, const void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueCopyImage)(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image, const size_t src_origin[3], const size_t dst_origin[3], const size_t region[3], cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueCopyImageToBuffer)(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer, const size_t src_origin[3], const size_t region[3], size_t dst_offset, cl_uint num_events_in_wait_list, const cl_event * event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueCopyBufferToImage)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image, size_t src_offset, const size_t dst_origin[3], const size_t region[3], cl_uint num_events_in_wait_list, const cl_event * event_wait_list, cl_event *event);
CL_EXTERN void *(CL_API_CALL *f_clEnqueueMapBuffer)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags, size_t offset, size_t size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event, cl_int *errcode_ret);
CL_EXTERN void *(CL_API_CALL *f_clEnqueueMapImage)(cl_command_queue  command_queue, cl_mem image, cl_bool blocking_map, cl_map_flags map_flags, const size_t origin[3], const size_t region[3], size_t *image_row_pitch, size_t *image_slice_pitch, cl_uint  num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event, cl_int *errcode_ret);
CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueUnmapMemObject)(cl_command_queue command_queue, cl_mem memobj, void *mapped_ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

CL_EXTERN cl_int(CL_API_CALL *f_clWaitForEvents)(cl_uint num_events, const cl_event *event_list);
CL_EXTERN cl_int(CL_API_CALL *f_clGetEventInfo)(cl_event event, cl_event_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_event(CL_API_CALL *f_clCreateUserEvent)(cl_context context, cl_int *errcode_ret);
CL_EXTERN cl_int(CL_API_CALL *f_clRetainEvent)(cl_event event);
CL_EXTERN cl_int(CL_API_CALL *f_clReleaseEvent)(cl_event event);
CL_EXTERN cl_int(CL_API_CALL *f_clSetUserEventStatus)(cl_event event, cl_int execution_status);
CL_EXTERN cl_int(CL_API_CALL *f_clGetEventProfilingInfo)(cl_event event, cl_profiling_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);

CL_EXTERN cl_int(CL_API_CALL *f_clFlush)(cl_command_queue command_queue);
CL_EXTERN cl_int(CL_API_CALL *f_clFinish)(cl_command_queue command_queue);

CL_EXTERN cl_int(CL_API_CALL *f_clGetKernelSubGroupInfo)(cl_kernel kernel, cl_device_id device, cl_kernel_sub_group_info param_name, size_t input_value_size, const void *input_value, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_int(CL_API_CALL *f_clGetKernelSubGroupInfoKHR)(cl_kernel kernel, cl_device_id device, cl_kernel_sub_group_info param_name, size_t input_value_size, const void *input_value, size_t param_value_size, void *param_value, size_t *param_value_size_ret);

#define clGetExtensionFunctionAddressForPlatform f_clGetExtensionFunctionAddressForPlatform

#define clGetPlatformIDs f_clGetPlatformIDs
#define clGetPlatformInfo f_clGetPlatformInfo
#define clGetDeviceIDs f_clGetDeviceIDs
#define clGetDeviceInfo f_clGetDeviceInfo
#define clGetDeviceIDsFromDX9MediaAdapterKHR f_clGetDeviceIDsFromDX9MediaAdapterKHR
#define clGetDeviceIDsFromD3D11KHR f_clGetDeviceIDsFromD3D11KHR

#define clCreateContext f_clCreateContext
#define clReleaseContext f_clReleaseContext
#define clCreateCommandQueue f_clCreateCommandQueue
#define clGetCommandQueueInfo f_clGetCommandQueueInfo
#define clReleaseCommandQueue f_clReleaseCommandQueue

#define clCreateProgramWithSource f_clCreateProgramWithSource
#define clBuildProgram f_clBuildProgram
#define clGetProgramBuildInfo f_clGetProgramBuildInfo
#define clGetProgramInfo f_clGetProgramInfo
#define clReleaseProgram f_clReleaseProgram

#define clCreateBuffer f_clCreateBuffer
#define clCreateImage f_clCreateImage
#define clReleaseMemObject f_clReleaseMemObject
#define clGetMemObjectInfo f_clGetMemObjectInfo
#define clGetImageInfo f_clGetImageInfo
#define clCreateKernel f_clCreateKernel
#define clReleaseKernel f_clReleaseKernel
#define clSetKernelArg f_clSetKernelArg
#define clEnqueueNDRangeKernel f_clEnqueueNDRangeKernel
#define clEnqueueTask f_clEnqueueTask

#define clEnqueueReadBuffer f_clEnqueueReadBuffer
#define clEnqueueReadBufferRect f_clEnqueueReadBufferRect
#define clEnqueueWriteBuffer f_clEnqueueWriteBuffer
#define clEnqueueWriteBufferRect f_clEnqueueWriteBufferRect
#define clEnqueueCopyBuffer f_clEnqueueCopyBuffer
#define clEnqueueCopyBufferRect f_clEnqueueCopyBufferRect

#define clEnqueueReadImage f_clEnqueueReadImage
#define clEnqueueWriteImage f_clEnqueueWriteImage
#define clEnqueueCopyImage f_clEnqueueCopyImage
#define clEnqueueCopyImageToBuffer f_clEnqueueCopyImageToBuffer
#define clEnqueueCopyBufferToImage f_clEnqueueCopyBufferToImage
#define clEnqueueMapBuffer f_clEnqueueMapBuffer
#define clEnqueueMapImage f_clEnqueueMapImage
#define clEnqueueUnmapMemObject f_clEnqueueUnmapMemObject

#define clWaitForEvents f_clWaitForEvents
#define clGetEventInfo f_clGetEventInfo
#define clCreateUserEvent f_clCreateUserEvent
#define clRetainEvent f_clRetainEvent
#define clReleaseEvent f_clReleaseEvent
#define clSetUserEventStatus f_clSetUserEventStatus
#define clGetEventProfilingInfo f_clGetEventProfilingInfo

#define clFlush f_clFlush
#define clFinish f_clFinish

#define clGetKernelSubGroupInfo f_clGetKernelSubGroupInfo
#define clGetKernelSubGroupInfoKHR f_clGetKernelSubGroupInfoKHR

MAP_PAIR_0_1_PROTO(err, rgy, RGY_ERR, cl, cl_int);

class RGYOpenCLQueue;

typedef std::unique_ptr<std::remove_pointer<cl_context>::type, decltype(clReleaseContext)> unique_context;
typedef std::unique_ptr<std::remove_pointer<cl_command_queue>::type, decltype(clReleaseCommandQueue)> unique_queue;

static const TCHAR *cl_errmes(cl_int err) {
    return get_err_mes(err_cl_to_rgy(err));
}

enum RGYCLMemcpyKind {
    RGYCLMemcpyD2D,
    RGYCLMemcpyD2H,
    RGYCLMemcpyH2D,
    RGYCLMemcpyH2H,
};

static inline RGYCLMemcpyKind getMemcpyKind(RGY_MEM_TYPE inputDevice, RGY_MEM_TYPE outputDevice) {
    if (inputDevice != RGY_MEM_TYPE_CPU) {
        return (outputDevice != RGY_MEM_TYPE_CPU) ? RGYCLMemcpyD2D : RGYCLMemcpyD2H;
    } else {
        return (outputDevice != RGY_MEM_TYPE_CPU) ? RGYCLMemcpyH2D : RGYCLMemcpyH2H;
    }
}

static const TCHAR *getMemcpyKindStr(RGYCLMemcpyKind kind) {
    switch (kind) {
    case RGYCLMemcpyD2D:
        return _T("copyDtoD");
    case RGYCLMemcpyD2H:
        return _T("copyDtoH");
    case RGYCLMemcpyH2D:
        return _T("copyHtoD");
    case RGYCLMemcpyH2H:
        return _T("copyHtoH");
    default:
        return _T("copyUnknown");
    }
}

static const TCHAR *getMemcpyKindStr(RGY_MEM_TYPE inputDevice, RGY_MEM_TYPE outputDevice) {
    return getMemcpyKindStr(getMemcpyKind(inputDevice, outputDevice));
}

struct cl_event_deleter {
    void operator()(cl_event *e) const {
        if (*e) {
            clReleaseEvent(*e);
        }
        delete e;
    }
};

class RGYOpenCLEvent {
public:
    RGYOpenCLEvent(const cl_event event) : event_(new cl_event, cl_event_deleter()) {
        *event_ = event;
    }

    RGYOpenCLEvent() : event_(new cl_event, cl_event_deleter()) {
        *event_ = nullptr;
    }

    void wait() const {
        clWaitForEvents(1, &(*event_));
    }
    void reset() {
        if (*event_ != nullptr) {
            event_ = std::shared_ptr<cl_event>(new cl_event, cl_event_deleter());
        }
        *event_ = nullptr;
    }
    cl_event *reset_ptr() {
        reset();
        return &(*event_);
    }
    cl_event &operator()() { return *event_; }
    const cl_event &operator()() const { return *event_; }
    const cl_event *ptr() const { return &(*event_); }
private:
    std::shared_ptr<cl_event> event_;
};

class RGYCLBufMap {
public:
    RGYCLBufMap(cl_mem mem) : m_mem(mem), m_queue(RGYDefaultQueue), m_hostPtr(nullptr), m_eventMap() {};
    ~RGYCLBufMap() {
        unmap();
    }
    RGY_ERR map(cl_map_flags map_flags, size_t size, cl_command_queue queue);
    RGY_ERR map(cl_map_flags map_flags, size_t size, cl_command_queue queue, const std::vector<RGYOpenCLEvent> &wait_events);
    RGY_ERR unmap();
    RGY_ERR unmap(cl_command_queue queue);
    RGY_ERR unmap(cl_command_queue queue, const std::vector<RGYOpenCLEvent> &wait_events);

    const RGYOpenCLEvent &event() const { return m_eventMap; }
    RGYOpenCLEvent &event() { return m_eventMap; }
    const void *ptr() const { return m_hostPtr; }
    void *ptr() { return m_hostPtr; }
protected:
    RGYCLBufMap(const RGYCLBufMap &) = delete;
    void operator =(const RGYCLBufMap &) = delete;
    cl_mem m_mem;
    cl_command_queue m_queue;
    void *m_hostPtr;
    RGYOpenCLEvent m_eventMap;
};

class RGYCLBuf {
public:
    RGYCLBuf(cl_mem mem, cl_mem_flags flags, size_t size) : m_mem(mem), m_flags(flags), m_size(size), m_mapped(mem) {
    };
    ~RGYCLBuf() {
        clear();
    }
    void clear() {
        m_mapped.unmap();
        if (m_mem) {
            clReleaseMemObject(m_mem);
            m_mem = nullptr;
        }
    }
    cl_mem &mem() { return m_mem; }
    const cl_mem &mem() const { return m_mem; }
    size_t size() const { return m_size; }
    cl_mem_flags flags() const { return m_flags; }

    RGY_ERR queueMapBuffer(cl_command_queue queue, cl_map_flags map_flags, const std::vector<RGYOpenCLEvent> &wait_events = {});
    const RGYOpenCLEvent &mapEvent() const { return m_mapped.event(); }
    const void *mappedPtr() const { return m_mapped.ptr(); }
    void *mappedPtr() { return m_mapped.ptr(); }
    RGY_ERR unmapBuffer();
    RGY_ERR unmapBuffer(cl_command_queue queue, const std::vector<RGYOpenCLEvent> &wait_events = {});
protected:
    RGYCLBuf(const RGYCLBuf &) = delete;
    void operator =(const RGYCLBuf &) = delete;

    cl_mem m_mem;
    cl_mem_flags m_flags;
    size_t m_size;
    RGYCLBufMap m_mapped;
};

class RGYCLFrameMap {
public:
    RGYCLFrameMap(RGYFrameInfo dev, RGYOpenCLQueue &queue) : m_dev(dev), m_queue(queue), m_host(), m_eventMap() {};
    ~RGYCLFrameMap() {
        unmap();
    }
    RGY_ERR map(cl_map_flags map_flags, RGYOpenCLQueue &queue);
    RGY_ERR map(cl_map_flags map_flags, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events);
    RGY_ERR unmap();
    RGY_ERR unmap(RGYOpenCLQueue &queue);
    RGY_ERR unmap(RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events);

    const RGYOpenCLEvent &event() const { return m_eventMap; }
    RGYOpenCLEvent &event() { return m_eventMap; }
    const RGYFrameInfo& host() const { return m_host; }
protected:
    RGYCLFrameMap(const RGYCLFrameMap &) = delete;
    void operator =(const RGYCLFrameMap &) = delete;
    RGYFrameInfo m_dev;
    RGYOpenCLQueue &m_queue;
    RGYFrameInfo m_host;
    RGYOpenCLEvent m_eventMap;
};

struct RGYCLFrame {
public:
    RGYFrameInfo frame;
    cl_mem_flags flags;
    std::unique_ptr<RGYCLFrameMap> m_mapped;
    RGYCLFrame()
        : frame(), flags(0), m_mapped() {
    };
    RGYCLFrame(const RGYFrameInfo &info_, cl_mem_flags flags_ = CL_MEM_READ_WRITE)
        : frame(info_), flags(flags_), m_mapped() {
    };
    RGY_ERR queueMapBuffer(RGYOpenCLQueue &queue, cl_map_flags map_flags, const std::vector<RGYOpenCLEvent> &wait_events = {});
    RGY_ERR unmapBuffer();
    RGY_ERR unmapBuffer(RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events = {});
    const RGYOpenCLEvent &mapEvent() const { return m_mapped->event(); }
    const RGYFrameInfo &mappedHost() const { return m_mapped->host(); }
protected:
    RGYCLFrame(const RGYCLFrame &) = delete;
    void operator =(const RGYCLFrame &) = delete;
public:
    cl_mem& mem(int i) {
        return (cl_mem&)frame.ptr[i];
    }
    void clear() {
        m_mapped.reset();
        for (int i = 0; i < _countof(frame.ptr); i++) {
            if (mem(i)) {
                clReleaseMemObject(mem(i));
                mem(i) = nullptr;
            }
            frame.pitch[i] = 0;
        }
    }
    ~RGYCLFrame() {
        m_mapped.reset();
        clear();
    }
};

struct RGYOpenCLDeviceInfo {
    cl_device_type type;
    int vendor_id;
    int max_compute_units;
    int max_clock_frequency;
    int max_samplers;
    uint64_t global_mem_size;
    size_t profiling_timer_resolution;
    std::string name;
    std::string vendor;
    std::string driver_version;
    std::string profile;
    std::string version;
    std::string extensions;
};

class RGYOpenCLDevice {
public:
    RGYOpenCLDevice(cl_device_id device);
    virtual ~RGYOpenCLDevice() {};
    RGYOpenCLDeviceInfo info();
    tstring infostr();
protected:
    cl_device_id m_device;
};

struct RGYOpenCLPlatformInfo {
    std::string profile;
    std::string version;
    std::string name;
    std::string vendor;
    std::string extension;

    std::string print();
};

class RGYOpenCLPlatform {
public:
    RGYOpenCLPlatform(cl_platform_id platform, shared_ptr<RGYLog> pLog);
    virtual ~RGYOpenCLPlatform() {};
    RGY_ERR createDeviceList(cl_device_type device_type);
    RGY_ERR createDeviceListD3D9(cl_device_type device_type, void *d3d9dev);
    RGY_ERR createDeviceListD3D11(cl_device_type device_type, void *d3d11dev);
    cl_platform_id get() const { return m_platform; };
    const void *d3d9dev() const { return m_d3d9dev; };
    const void *d3d11dev() const { return m_d3d11dev; };
    std::vector<cl_device_id>& devs() { return m_devices; };
    cl_device_id dev(int idx) { return m_devices[idx]; };
    const std::vector<cl_device_id>& devs() const { return m_devices; };
    void setDev(cl_device_id dev) { m_devices.clear(); m_devices.push_back(dev); };
    void setDev(cl_device_id dev, void *d3d9dev, void *d3d11dev) {
        m_devices.clear(); m_devices.push_back(dev);
        if (d3d9dev) m_d3d9dev = d3d9dev;
        if (d3d11dev) m_d3d11dev = d3d11dev;
    };
    void setDevs(std::vector<cl_device_id> &devs) { m_devices = devs; };
    bool isVendor(const char *vendor);
    RGYOpenCLPlatformInfo info();
protected:

    cl_platform_id m_platform;
    void *m_d3d9dev;
    void *m_d3d11dev;
    std::vector<cl_device_id> m_devices;
    shared_ptr<RGYLog> m_pLog;
};

template<typename T>
T divCeil(T i, T div) {
    return (i + div - 1) / div;
}

struct RGYWorkSize {
    size_t w[3];
    RGYWorkSize() {
        w[0] = w[1] = w[2] = 1;
    }
    RGYWorkSize(size_t x) {
        w[0] = x;
        w[1] = w[2] = 1;
    };
    RGYWorkSize(size_t x, size_t y) {
        w[0] = x;
        w[1] = y;
        w[2] = 1;
    }
    RGYWorkSize(size_t x, size_t y, size_t z) {
        w[0] = x;
        w[1] = y;
        w[2] = z;
    }
    size_t total() const {
        return w[0] * w[1] * w[2];
    }
    const size_t *operator()() const {
        return &w[0];
    }
    const size_t operator()(int i) const {
        return w[i];
    }
    RGYWorkSize groups(const RGYWorkSize &local) const {
        RGYWorkSize group = *this;
        for (int i = 0; i < 3; i++) {
            if (local.w[i] > 0) {
                group.w[i] = divCeil(w[i], local.w[i]);
            }
        }
        return group;
    }
    RGYWorkSize ceilGlobal(const RGYWorkSize& local) const {
        const RGYWorkSize group = groups(local);
        RGYWorkSize global = *this;
        for (int i = 0; i < 3; i++) {
            if (local.w[i] > 0) {
                global.w[i] = group.w[i] * local.w[i];
            }
        }
        return global;
    }
};

class RGYOpenCLKernelDynamicLocal {
protected:
    size_t size_;
public:
    RGYOpenCLKernelDynamicLocal(size_t size) : size_(size) {};
    ~RGYOpenCLKernelDynamicLocal() {};
    size_t size() const { return size_; }
};

class RGYOpenCLKernelLauncher {
public:
    RGYOpenCLKernelLauncher(cl_kernel kernel, std::string kernelName, RGYOpenCLQueue &queue, const RGYWorkSize &local, const RGYWorkSize &global, shared_ptr<RGYLog> pLog, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);
    virtual ~RGYOpenCLKernelLauncher() {};

    size_t subGroupSize() const;
    size_t subGroupCount() const;
    RGY_ERR launch(std::vector<void *> arg_ptrs = std::vector<void *>(), std::vector<size_t> arg_size = std::vector<size_t>(), std::vector<std::type_index> = std::vector<std::type_index>());

    template <typename... ArgTypes>
    RGY_ERR operator()(ArgTypes... args) {
        return launch(args...);
    }
    template <typename... ArgTypes>
    RGY_ERR launch(ArgTypes... args) {
        return this->launch(
            std::vector<void *>({ (void *)&args... }),
            std::vector<size_t>({ sizeof(args)... }),
            std::vector<std::type_index>({ typeid(args)... })
        );
    }
protected:
    cl_kernel m_kernel;
    std::string m_kernelName;
    RGYOpenCLQueue &m_queue;
    RGYWorkSize m_local;
    RGYWorkSize m_global;
    shared_ptr<RGYLog> m_pLog;
    std::vector<cl_event> m_wait_events;
    RGYOpenCLEvent *m_event;
};

class RGYOpenCLKernel {
public:
    RGYOpenCLKernel() : m_kernel(), m_kernelName(), m_pLog() {};
    RGYOpenCLKernel(cl_kernel kernel, std::string kernelName, shared_ptr<RGYLog> pLog);
    cl_kernel get() const { return m_kernel; }
    virtual ~RGYOpenCLKernel();
    RGYOpenCLKernelLauncher config(RGYOpenCLQueue &queue, const RGYWorkSize &local, const RGYWorkSize &global);
    RGYOpenCLKernelLauncher config(RGYOpenCLQueue &queue, const RGYWorkSize &local, const RGYWorkSize &global, RGYOpenCLEvent *event);
    RGYOpenCLKernelLauncher config(RGYOpenCLQueue &queue, const RGYWorkSize &local, const RGYWorkSize &global, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event = nullptr);
protected:
    cl_kernel m_kernel;
    std::string m_kernelName;
    shared_ptr<RGYLog> m_pLog;
};

class RGYOpenCLProgram {
public:
    RGYOpenCLProgram(cl_program program, shared_ptr<RGYLog> pLog);
    virtual ~RGYOpenCLProgram();

    RGYOpenCLKernel kernel(const char *kernelName);
    std::vector<uint8_t> getBinary();
protected:
    cl_program m_program;
    shared_ptr<RGYLog> m_pLog;
};

class RGYOpenCLQueue {
public:
    RGYOpenCLQueue();
    RGYOpenCLQueue(cl_command_queue queue, cl_device_id devid);
    RGYOpenCLQueue(RGYOpenCLQueue &&) = default;
    RGYOpenCLQueue &operator=(RGYOpenCLQueue &&rhs) {
        if (this != &rhs) {
            m_queue = std::move(rhs.m_queue);
            m_devid = rhs.m_devid;
        }
        return *this;
    }
    virtual ~RGYOpenCLQueue();

    cl_command_queue operator()() { return m_queue.get(); }
    const cl_command_queue operator()() const { return m_queue.get(); }
    const cl_command_queue get() const {
        return m_queue.get();
    }
    cl_device_id devid() const {
        return m_devid;
    }
    RGY_ERR flush() const;
    RGY_ERR finish() const;
    void clear();
protected:
    RGYOpenCLQueue(const RGYOpenCLQueue &) = delete;
    void operator =(const RGYOpenCLQueue &) = delete;
    unique_queue m_queue;
    cl_device_id m_devid;
};

enum class RGYFrameCopyMode {
    FRAME,
    FIELD_TOP,
    FIELD_BOTTOM
};

class RGYOpenCLContext {
public:
    RGYOpenCLContext(shared_ptr<RGYOpenCLPlatform> platform, shared_ptr<RGYLog> pLog);
    virtual ~RGYOpenCLContext();

    RGY_ERR createContext();
    cl_context context() const { return m_context.get(); };
    const RGYOpenCLQueue& queue(int idx=0) const { return m_queue[idx]; };
    RGYOpenCLQueue& queue(int idx=0) { return m_queue[idx]; };
    RGYOpenCLPlatform *platform() const { return m_platform.get(); };

    unique_ptr<RGYOpenCLProgram> build(const std::string& source, const char *options);
    unique_ptr<RGYOpenCLProgram> buildFile(const tstring &filename, const char *options);
    unique_ptr<RGYOpenCLProgram> buildResource(const TCHAR *name, const TCHAR *type, const char *options);

    RGYOpenCLQueue createQueue(cl_device_id devid);
    unique_ptr<RGYCLBuf> createBuffer(size_t size, cl_mem_flags flags = CL_MEM_READ_WRITE, void *host_ptr = nullptr);
    unique_ptr<RGYCLBuf> copyDataToBuffer(const void *host_ptr, size_t size, cl_mem_flags flags = CL_MEM_READ_WRITE, cl_command_queue queue = 0);
    RGY_ERR createImageFromPlane(cl_mem& image, cl_mem buffer, int bit_depth, int channel_order, bool normalized, int pitch, int width, int height, cl_mem_flags flags);
    unique_ptr<RGYCLFrame> createImageFromFrameBuffer(const RGYFrameInfo &frame, bool normalized, cl_mem_flags flags);
    unique_ptr<RGYCLFrame> createFrameBuffer(int width, int height, RGY_CSP csp, cl_mem_flags flags = CL_MEM_READ_WRITE);
    unique_ptr<RGYCLFrame> createFrameBuffer(const RGYFrameInfo &frame, cl_mem_flags flags = CL_MEM_READ_WRITE);
    RGY_ERR copyFrame(RGYFrameInfo *dst, const RGYFrameInfo *src);
    RGY_ERR copyFrame(RGYFrameInfo *dst, const RGYFrameInfo *src, const sInputCrop *srcCrop);
    RGY_ERR copyFrame(RGYFrameInfo *dst, const RGYFrameInfo *src, const sInputCrop *srcCrop, RGYOpenCLQueue &queue);
    RGY_ERR copyFrame(RGYFrameInfo *dst, const RGYFrameInfo *src, const sInputCrop *srcCrop, RGYOpenCLQueue &queue, RGYOpenCLEvent *event);
    RGY_ERR copyFrame(RGYFrameInfo *dst, const RGYFrameInfo *src, const sInputCrop *srcCrop, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event = nullptr, RGYFrameCopyMode copyMode = RGYFrameCopyMode::FRAME);
    RGY_ERR copyPlane(RGYFrameInfo *dst, const RGYFrameInfo *src);
    RGY_ERR copyPlane(RGYFrameInfo *dst, const RGYFrameInfo *src, const sInputCrop *srcCrop);
    RGY_ERR copyPlane(RGYFrameInfo *dst, const RGYFrameInfo *src, const sInputCrop *srcCrop, RGYOpenCLQueue &queue);
    RGY_ERR copyPlane(RGYFrameInfo *dst, const RGYFrameInfo *src, const sInputCrop *srcCrop, RGYOpenCLQueue &queue, RGYOpenCLEvent *event);
    RGY_ERR copyPlane(RGYFrameInfo *dst, const RGYFrameInfo *src, const sInputCrop *srcCrop, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event = nullptr, RGYFrameCopyMode copyMode = RGYFrameCopyMode::FRAME);
    RGY_ERR setPlane(int value, RGYFrameInfo *dst);
    RGY_ERR setPlane(int value, RGYFrameInfo *dst, const sInputCrop *srcCrop);
    RGY_ERR setPlane(int value, RGYFrameInfo *dst, const sInputCrop *srcCrop, RGYOpenCLQueue &queue);
    RGY_ERR setPlane(int value, RGYFrameInfo *dst, const sInputCrop *srcCrop, RGYOpenCLQueue &queue, RGYOpenCLEvent *event);
    RGY_ERR setPlane(int value, RGYFrameInfo *dst, const sInputCrop *srcCrop, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event = nullptr);
    RGY_ERR setFrame(int value, RGYFrameInfo *dst);
    RGY_ERR setFrame(int value, RGYFrameInfo *dst, const sInputCrop *srcCrop);
    RGY_ERR setFrame(int value, RGYFrameInfo *dst, const sInputCrop *srcCrop, RGYOpenCLQueue &queue);
    RGY_ERR setFrame(int value, RGYFrameInfo *dst, const sInputCrop *srcCrop, RGYOpenCLQueue &queue, RGYOpenCLEvent *event);
    RGY_ERR setFrame(int value, RGYFrameInfo *dst, const sInputCrop *srcCrop, RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event = nullptr);
protected:
    unique_ptr<RGYOpenCLProgram> build(const char *data, const size_t size, const char *options);

    shared_ptr<RGYOpenCLPlatform> m_platform;
    unique_context m_context;
    std::vector<RGYOpenCLQueue> m_queue;
    shared_ptr<RGYLog> m_pLog;
    unique_ptr<RGYOpenCLProgram> m_copyI2B;
    unique_ptr<RGYOpenCLProgram> m_copyB2I;
    unique_ptr<RGYOpenCLProgram> m_copyB2B;
    unique_ptr<RGYOpenCLProgram> m_copyI2I;
    unique_ptr<RGYOpenCLProgram> m_setB;
    unique_ptr<RGYOpenCLProgram> m_setI;
};

class RGYOpenCL {
public:
    static HMODULE openCLHandle;
    static bool openCLCrush;

    RGYOpenCL();
    RGYOpenCL(shared_ptr<RGYLog> pLog);
    virtual ~RGYOpenCL();

    std::vector<shared_ptr<RGYOpenCLPlatform>> getPlatforms(const char *vendor = nullptr);
protected:
    shared_ptr<RGYLog> m_pLog;
};

int initOpenCLGlobal();

#endif //ENABLE_OPENCL

#endif //__RGY_OPENCL_H__
