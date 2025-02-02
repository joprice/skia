/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "modules/skottie/include/SlotManager.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/text/TextAdapter.h"
#include "modules/skresources/include/SkResources.h"

class skottie::SlotManager::ImageAssetProxy final : public skresources::ImageAsset {
public:
    explicit ImageAssetProxy(sk_sp<skresources::ImageAsset> asset)
    : fImageAsset(std::move(asset)) {}

    // always returns true to force the FootageLayer to always redraw in case asset is swapped
    bool isMultiFrame() override { return true; }

    FrameData getFrameData(float t) override {
        if (fImageAsset) {
            return fImageAsset->getFrameData(t);
        }
        return {nullptr , SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest),
            SkMatrix::I(), SizeFit::kCenter};
    }

    void setImageAsset (sk_sp<skresources::ImageAsset> asset) {
        fImageAsset = std::move(asset);
    }

    sk_sp<const skresources::ImageAsset> getImageAsset() const {
        return fImageAsset;
    }
private:
    sk_sp<skresources::ImageAsset> fImageAsset;
};

skottie::SlotManager::SlotManager(sk_sp<skottie::internal::SceneGraphRevalidator> revalidator) :
fRevalidator(revalidator) {}

skottie::SlotManager::~SlotManager() = default;

void skottie::SlotManager::setColorSlot(SlotID slotID, SkColor c) {
    const auto valueGroup = fColorMap.find(slotID);
    if (valueGroup) {
        for (auto& cPair : *valueGroup) {
            *(cPair.value) = c;
            cPair.node->invalidate();
        }
        fRevalidator->revalidate();
    }
}

void skottie::SlotManager::setImageSlot(SlotID slotID, sk_sp<skresources::ImageAsset> i) {
    const auto imageGroup = fImageMap.find(slotID);
    if (imageGroup) {
        for (auto& imageAsset : *imageGroup) {
            imageAsset->setImageAsset(i);
        }
        fRevalidator->revalidate();
    }
}

void skottie::SlotManager::setScalarSlot(SlotID slotID, ScalarValue s) {
    const auto valueGroup = fScalarMap.find(slotID);
    if (valueGroup) {
        for (auto& sPair : *valueGroup) {
            *(sPair.value) = s;
            if (sPair.node) {
                sPair.node->invalidate();
            } else if (sPair.adapter) {
                sPair.adapter->onSync();
            }
        }
        fRevalidator->revalidate();
    }
}

void skottie::SlotManager::setTextSlot(SlotID slotID, TextPropertyValue& t) {
    const auto adapterGroup = fTextMap.find(slotID);
    if (adapterGroup) {
        for (auto& textAdapter : *adapterGroup) {
            textAdapter->setText(t);
        }
        fRevalidator->revalidate();
    }
}

SkColor skottie::SlotManager::getColorSlot(SlotID slotID) const {
    const auto valueGroup = fColorMap.find(slotID);
    return valueGroup && !valueGroup->empty() ? *(valueGroup->at(0).value) : SK_ColorBLACK;
}

sk_sp<const skresources::ImageAsset> skottie::SlotManager::getImageSlot(SlotID slotID) const {
    const auto imageGroup = fImageMap.find(slotID);
    return imageGroup && !imageGroup->empty() ? imageGroup->at(0)->getImageAsset() : nullptr;
}

skottie::ScalarValue skottie::SlotManager::getScalarSlot(SlotID slotID) const {
    const auto valueGroup = fScalarMap.find(slotID);
    return valueGroup && !valueGroup->empty() ? *(valueGroup->at(0).value) : -1;
}

skottie::TextPropertyValue skottie::SlotManager::getTextSlot(SlotID slotID) const {
    const auto adapterGroup = fTextMap.find(slotID);
    return adapterGroup && !adapterGroup->empty() ?
           adapterGroup->at(0)->getText() :
           TextPropertyValue();
}

void skottie::SlotManager::trackColorValue(SlotID slotID, SkColor* colorValue,
                                           sk_sp<sksg::Node> node) {
    fColorMap[slotID].push_back({colorValue, std::move(node), nullptr});
}

sk_sp<skresources::ImageAsset> skottie::SlotManager::trackImageValue(SlotID slotID,
                                                                     sk_sp<skresources::ImageAsset>
                                                                        imageAsset) {
    auto proxy = sk_make_sp<ImageAssetProxy>(std::move(imageAsset));
    fImageMap[slotID].push_back(proxy);
    return std::move(proxy);
}

void skottie::SlotManager::trackScalarValue(SlotID slotID, ScalarValue* scalarValue,
                                            sk_sp<internal::AnimatablePropertyContainer> adapter) {
    fScalarMap[slotID].push_back({scalarValue, nullptr, adapter});
}

void skottie::SlotManager::trackTextValue(SlotID slotID, sk_sp<internal::TextAdapter> adapter) {
    fTextMap[slotID].push_back(std::move(adapter));
}

skottie::SlotManager::SlotInfo skottie::SlotManager::getSlotInfo() const {
    SlotInfo sInfo;
    for (const auto& c : fColorMap) {
        sInfo.fColorSlotIDs.push_back(c.first);
    }
    for (const auto& s : fScalarMap) {
        sInfo.fScalarSlotIDs.push_back(s.first);
    }
    for (const auto& i : fImageMap) {
        sInfo.fImageSlotIDs.push_back(i.first);
    }
    for (const auto& t : fTextMap) {
        sInfo.fTextSlotIDs.push_back(t.first);
    }
    return sInfo;
}
