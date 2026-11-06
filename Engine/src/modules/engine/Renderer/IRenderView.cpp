//
// Created by cmorg on 9/12/2026.
//

#include "IRenderView.h"

void IRenderView::shutdown() {
    FF_Memory::ff_free_recursive<Renderpass>(renderpasses, ARRAY, renderpassCount);
    renderpasses = nullptr;
}

bool IRenderView::initialize(const unsigned long newSize) {
    size = newSize;

    return true;
};