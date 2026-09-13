//
// Created by cmorg on 9/12/2026.
//

#include "IRenderView.h"

void IRenderView::shutdown() {
    renderpasses.shutdown();

    shaderSystemRef = nullptr;
}

bool IRenderView::initialize(ShaderSystem *shaderRef, const unsigned long newSize) {
    shaderSystemRef = shaderRef;
    size = newSize;

    return true;
};