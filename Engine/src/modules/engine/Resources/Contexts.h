//
// Created by cmorg on 8/1/2026.
//

#pragma once
#include "src/defines.h"

struct AssetContext {
    //Number of references to this asset
    unsigned long referenceCount = 0;
    //Index of the asset in the asset map
    unsigned int index = INVALID_ID;
    //Whether this asset will auto destroy itself when there are no remaining assets.
    bool bAutoRelease = false;
};
