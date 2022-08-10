/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ImageUtils.h"

#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/image/SkImage_Base.h"

namespace {

// We require that the user returns a Graphite-backed image that preserves the dimensions, channels
// and alpha type of the original image. Additionally, it must satisfy the mipmap requirements.
// The bit depth of the individual channels can change (e.g., 4444 -> 8888 is allowed).
bool valid_client_provided_image(const SkImage* clientProvided,
                                 const SkImage* original,
                                 skgpu::graphite::Mipmapped mipmapped) {
    if (!clientProvided ||
        !as_IB(clientProvided)->isGraphiteBacked() ||
        original->dimensions() != clientProvided->dimensions() ||
        original->alphaType() != clientProvided->alphaType()) {
        return false;
    }

    uint32_t origChannels = SkColorTypeChannelFlags(original->colorType());
    uint32_t clientChannels = SkColorTypeChannelFlags(clientProvided->colorType());
    if (origChannels != clientChannels) {
        return false;
    }

    return mipmapped == skgpu::graphite::Mipmapped::kNo || clientProvided->hasMipmaps();
}

} // anonymous namespace

namespace skgpu::graphite {

std::pair<sk_sp<SkImage>, SkSamplingOptions> GetGraphiteBacked(Recorder* recorder,
                                                               const SkImage* imageIn,
                                                               SkSamplingOptions sampling) {
    skgpu::graphite::Mipmapped mipmapped = (sampling.mipmap != SkMipmapMode::kNone)
                                               ? skgpu::graphite::Mipmapped::kYes
                                               : skgpu::graphite::Mipmapped::kNo;

    sk_sp<SkImage> result;
    if (as_IB(imageIn)->isGraphiteBacked()) {
        result = sk_ref_sp(imageIn);

        // If the preexisting Graphite-backed image doesn't have the required mipmaps we will drop
        // down the sampling
        if (mipmapped == skgpu::graphite::Mipmapped::kYes && !result->hasMipmaps()) {
            mipmapped = skgpu::graphite::Mipmapped::kNo;
            sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
        }
    } else {
        auto clientImageProvider = recorder->clientImageProvider();
        result = clientImageProvider->findOrCreate(recorder, imageIn, mipmapped);

        if (!valid_client_provided_image(result.get(), imageIn, mipmapped)) {
            // The client did not fulfill the ImageProvider contract so drop the image.
            result = nullptr;
        }
    }

    return { result, sampling };
}

} // namespace skgpu::graphite