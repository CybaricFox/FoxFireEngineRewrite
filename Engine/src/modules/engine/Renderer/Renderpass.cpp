//
// Created by cmorg on 9/11/2026.
//

#include "Renderpass.h"

void Renderpass::shutdown() {
    if (targets) {
        FF_Memory::ff_free_recursive<RenderTarget>(targets, ARRAY, renderTargetCount);
        targets = nullptr;
    }
}
