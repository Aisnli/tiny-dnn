/*
    Copyright (c) 2016, Taiga Nomi, Edgar Riba
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#include "tiny_dnn/layers/layer.h"
#include "tiny_dnn/core/params/conv_params.h"
#include "tiny_dnn/core/params/deconv_params.h"
#include "tiny_dnn/core/params/maxpool_params.h"
#include "tiny_dnn/core/params/fully_params.h"

#ifdef CNN_USE_NNPACK
#include "nnpack.h"
#endif

namespace tiny_dnn {
namespace core {

// TODO(edgar): remove this
class context;

enum class backend_t { internal, nnpack, libdnn, avx, opencl };

inline std::ostream& operator << (std::ostream& os, backend_t type) {
    switch (type) {
        case backend_t::internal: os << "Internal"; break;
        case backend_t::nnpack:   os << "NNPACK";   break;
        case backend_t::libdnn:   os << "LibDNN";   break;
        case backend_t::avx:      os << "AVX";      break;
        case backend_t::opencl:   os << "OpenCL";   break;
        default:
            throw nn_error("Not supported ostream enum.");
            break;
    }
    return os;
}

inline backend_t default_engine() {
#ifdef CNN_USE_AVX
#if defined(__AVX__) || defined(__AVX2__)
    return backend_t::avx;
#endif
#endif // CNN_USE_AVX
    return backend_t::internal;
}

#ifdef CNN_USE_NNPACK
// Singleton to keep a global state whether NNPACK is initialized.
// Before using the API an initialization is required. For this reason
// we need to get an instance of the object in order to avoid a throw error.
//
// Usage:
//     NNPackInitializer::getInstance().initialize();
//
class NNPackInitializer {
 public:
    // We create a static instance of the object in case
    // that it wasn't created before and we return it.
    static NNPackInitializer& getInstance() {
        static NNPackInitializer instance;
        return instance;
    }

    // Tries to initialize NNPACK.
    // Calls an internal method to initialize in case that it's not,
    // otherwise it returns a void.
    // Throws an error if we do not succed with initialization.
    void initialize() {
        if (initialized_) return; // alredy initialized, do nothig.

        // calls internal method to initialize
        nnp_status init_status = nnp_initialize();
        if (init_status != nnp_status_success) {
            throw nn_error("Cannot initialize NNPACK.");
        }

        // succeded with initialization. We set the global
        // state to avoid exception errors in addition to
        // reuse code.
        initialized_ = true;
    }

 private:
    /** Flag to store whether NNPACK is initialized */
    bool initialized_ = false;
};

//TODO: create an interface to let users choose the algorithm
inline nnp_convolution_algorithm nnp_algorithm() {
    return nnp_convolution_algorithm_auto;
}

//TODO: create an interface to let users choose the transform strategy
inline nnp_convolution_transform_strategy nnp_kts() {
    //some algorithm accept tuple based only
    return nnp_convolution_transform_strategy_tuple_based;
}
#endif

class backend {
 public:
    // context holds solution-dependent parameters
    // context should be able to hold any types of structures (like boost::any)
    explicit backend(context* ctx_ = nullptr) {
        CNN_UNREFERENCED_PARAMETER(ctx_);
    }

    // core math functions

    virtual void conv2d(const std::vector<tensor_t*>& in_data,
                        std::vector<tensor_t*>&       out_data) = 0;

    virtual void conv2d_q(const std::vector<tensor_t*>& in_data,
                          std::vector<tensor_t*>&       out_data) = 0;

    virtual void conv2d_eq(const std::vector<tensor_t*>& in_data,
                           std::vector<tensor_t*>&       out_data) = 0;

    virtual void conv2d(const std::vector<tensor_t*>& in_data,
                        const std::vector<tensor_t*>& out_data,
                        std::vector<tensor_t*>&       out_grad,
                        std::vector<tensor_t*>&       in_grad) = 0;

    virtual void conv2d_q(const std::vector<tensor_t*>& in_data,
                          const std::vector<tensor_t*>& out_data,
                          std::vector<tensor_t*>&       out_grad,
                          std::vector<tensor_t*>&       in_grad) = 0;

    virtual void deconv2d(const std::vector<tensor_t*>& in_data,
                          std::vector<tensor_t*>&       out_data) = 0;

    virtual void deconv2d_q(const std::vector<tensor_t*>& in_data,
                            std::vector<tensor_t*>&       out_data) = 0;

    virtual void deconv2d_eq(const std::vector<tensor_t*>& in_data,
                             std::vector<tensor_t*>&       out_data) = 0;

    virtual void deconv2d(const std::vector<tensor_t*>& in_data,
                          const std::vector<tensor_t*>& out_data,
                          std::vector<tensor_t*>&       out_grad,
                          std::vector<tensor_t*>&       in_grad) = 0;

    virtual void deconv2d_q(const std::vector<tensor_t*>& in_data,
                            const std::vector<tensor_t*>& out_data,
                            std::vector<tensor_t*>&       out_grad,
                            std::vector<tensor_t*>&       in_grad) = 0;

    virtual void maxpool(const std::vector<tensor_t*>& in_data,
                         std::vector<tensor_t*>&       out_data) = 0;

    virtual void maxpool(const std::vector<tensor_t*>& in_data,
                         const std::vector<tensor_t*>& out_data,
                         std::vector<tensor_t*>&       out_grad,
                         std::vector<tensor_t*>&       in_grad) = 0;

    virtual void fully(const std::vector<tensor_t*>& in_data,
                       std::vector<tensor_t*>&       out_data) = 0;

    virtual void fully_q(const std::vector<tensor_t*>& in_data,
                         std::vector<tensor_t*>&       out_data) = 0;

    virtual void fully_eq(const std::vector<tensor_t*>& in_data,
                          std::vector<tensor_t*>&       out_data) = 0;

    virtual void fully(const std::vector<tensor_t*>& in_data,
                       const std::vector<tensor_t*>& out_data,
                       std::vector<tensor_t*>&       out_grad,
                       std::vector<tensor_t*>&       in_grad) = 0;

    virtual void fully_q(const std::vector<tensor_t*>& in_data,
                         const std::vector<tensor_t*>& out_data,
                         std::vector<tensor_t*>&       out_grad,
                         std::vector<tensor_t*>&       in_grad) = 0;

    context* get_context() const { return ctx_; }

    void set_layer(layerptr_t layer) { layer_ = layer; }

    virtual backend_t type() const = 0;

 protected:
    context* ctx_;
    layerptr_t layer_;
};

}  // namespace core
}  // namespace tiny_dnn
