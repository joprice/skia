/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkOnce.h"
#include "src/base/SkHalf.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOpts.h"

#if defined(SK_ARM_HAS_NEON)
    #define SK_OPTS_NS neon
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SKX
    #define SK_OPTS_NS skx
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    #define SK_OPTS_NS avx2
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
    #define SK_OPTS_NS avx
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE42
    #define SK_OPTS_NS sse42
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    #define SK_OPTS_NS sse41
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    #define SK_OPTS_NS ssse3
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE3
    #define SK_OPTS_NS sse3
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #define SK_OPTS_NS sse2
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    #define SK_OPTS_NS sse
#else
    #define SK_OPTS_NS portable
#endif

#include "src/opts/SkBitmapProcState_opts.h"
#include "src/opts/SkBlitMask_opts.h"
#include "src/opts/SkBlitRow_opts.h"
#include "src/opts/SkRasterPipeline_opts.h"
#include "src/opts/SkSwizzler_opts.h"
#include "src/opts/SkUtils_opts.h"

namespace SkOpts {
    // Define default function pointer values here...
    // If our global compile options are set high enough, these defaults might even be
    // CPU-specialized, e.g. a typical x86-64 machine might start with SSE2 defaults.
    // They'll still get a chance to be replaced with even better ones, e.g. using SSE4.1.
#define DEFINE_DEFAULT(name) decltype(name) name = SK_OPTS_NS::name
    DEFINE_DEFAULT(blit_mask_d32_a8);

    DEFINE_DEFAULT(blit_row_color32);
    DEFINE_DEFAULT(blit_row_s32a_opaque);

    DEFINE_DEFAULT(RGBA_to_BGRA);
    DEFINE_DEFAULT(RGBA_to_rgbA);
    DEFINE_DEFAULT(RGBA_to_bgrA);
    DEFINE_DEFAULT(RGB_to_RGB1);
    DEFINE_DEFAULT(RGB_to_BGR1);
    DEFINE_DEFAULT(gray_to_RGB1);
    DEFINE_DEFAULT(grayA_to_RGBA);
    DEFINE_DEFAULT(grayA_to_rgbA);
    DEFINE_DEFAULT(inverted_CMYK_to_RGB1);
    DEFINE_DEFAULT(inverted_CMYK_to_BGR1);

    DEFINE_DEFAULT(memset16);
    DEFINE_DEFAULT(memset32);
    DEFINE_DEFAULT(memset64);

    DEFINE_DEFAULT(rect_memset16);
    DEFINE_DEFAULT(rect_memset32);
    DEFINE_DEFAULT(rect_memset64);

    DEFINE_DEFAULT(S32_alpha_D32_filter_DX);

#undef DEFINE_DEFAULT

    size_t raster_pipeline_lowp_stride  = SK_OPTS_NS::raster_pipeline_lowp_stride();
    size_t raster_pipeline_highp_stride = SK_OPTS_NS::raster_pipeline_highp_stride();

#define M(st) (StageFn)SK_OPTS_NS::st,
    StageFn ops_highp[] = { SK_RASTER_PIPELINE_OPS_ALL(M) };
    StageFn just_return_highp = (StageFn)SK_OPTS_NS::just_return;
    void (*start_pipeline_highp)(size_t, size_t, size_t, size_t, SkRasterPipelineStage*) =
            SK_OPTS_NS::start_pipeline;
#undef M

#define M(st) (StageFn)SK_OPTS_NS::lowp::st,
    StageFn ops_lowp[] = { SK_RASTER_PIPELINE_OPS_LOWP(M) };
    StageFn just_return_lowp = (StageFn)SK_OPTS_NS::lowp::just_return;
    void (*start_pipeline_lowp)(size_t, size_t, size_t, size_t, SkRasterPipelineStage*) =
            SK_OPTS_NS::lowp::start_pipeline;
#undef M

    // Each Init_foo() is defined in src/opts/SkOpts_foo.cpp.
    void Init_ssse3();
    void Init_avx();
    void Init_hsw();
    void Init_erms();

    static void init() {
    #if defined(SK_ENABLE_OPTIMIZE_SIZE)
        // All Init_foo functions are omitted when optimizing for size
    #elif defined(SK_CPU_X86)
        #if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_SSSE3
            if (SkCpu::Supports(SkCpu::SSSE3)) { Init_ssse3(); }
        #endif

        #if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_AVX
            if (SkCpu::Supports(SkCpu::AVX)) { Init_avx();   }
            if (SkCpu::Supports(SkCpu::HSW)) { Init_hsw();   }
        #endif

        if (SkCpu::Supports(SkCpu::ERMS)) { Init_erms(); }
    #endif
    }

    void Init() {
        static SkOnce once;
        once(init);
    }
}  // namespace SkOpts
