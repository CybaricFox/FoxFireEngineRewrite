//
// Created by cmorg on 11/4/2026.
//

#pragma once
#include "EngineTextureTypes.h"
#include "src/modules/engine/Memory/DynamicArray.h"

enum RenderTargetAttachmentType {
    RENDER_TARGET_ATTACHMENT_TYPE_COLOR = 0x1,
    RENDER_TARGET_ATTACHMENT_TYPE_DEPTH = 0x2,
    RENDER_TARGET_ATTACHMENT_TYPE_STENCIL = 0x4,
};

enum RenderTargetAttachmentSource {
    RENDER_TARGET_ATTACHMENT_SOURCE_DEFAULT = 0x1,
    RENDER_TARGET_ATTACHMENT_SOURCE_VIEW = 0x2
};

enum RenderTargetAttachmentLoadOperation {
    RENDER_TARGET_ATTACHMENT_LOAD_OP_DONT_CARE = 0x0,
    RENDER_TARGET_ATTACHMENT_LOAD_OP_LOAD = 0x1
};

enum RenderTargetAttachmentStoreOperation {
    RENDER_TARGET_ATTACHMENT_STORE_OP_DONT_CARE = 0x0,
    RENDER_TARGET_ATTACHMENT_STORE_OP_STORE = 0x1
};

struct RenderTargetAttachmentConfig {
    RenderTargetAttachmentType type{};
    RenderTargetAttachmentSource source{};
    RenderTargetAttachmentLoadOperation loadOperation{};
    RenderTargetAttachmentStoreOperation storeOperation{};
    bool bPresentAfter = false;
};

struct RenderTargetConfig {
    unsigned char attachmentCount = 0;
    RenderTargetAttachmentConfig* attachments = nullptr;
};

struct RenderTargetAttachment {
    RenderTargetAttachmentType type{};
    RenderTargetAttachmentSource source{};
    RenderTargetAttachmentLoadOperation loadOperation{};
    RenderTargetAttachmentStoreOperation storeOperation{};
    bool bPresentAfter = false;
    Texture* texture = nullptr;
};

struct RenderTarget {
    unsigned char attachmentCount = 0;
    RenderTargetAttachment* attachments = nullptr;
    void* framebuffer = nullptr;
};