//
// Created by cmorg on 11/15/2026.
//

#pragma once
#include "src/modules/engine/Resources/Skybox.h"
#include "src/modules/engine/UserInterface/UIText.h"

struct MeshPacketData {
    unsigned int meshCount = 0;
    unsigned int* meshes = nullptr;
};
static_assert(std::is_trivially_copyable_v<MeshPacketData>,"Render View Packet Datas must remain trivially copyable");

struct UIPacketData {
    MeshPacketData meshData{};
    unsigned int textCount = 0;
    UIText** texts = nullptr;
};
static_assert(std::is_trivially_copyable_v<UIPacketData>,"Render View Packet Datas must remain trivially copyable");

struct SkyboxPacketData {
    Skybox* skybox = nullptr;
};
static_assert(std::is_trivially_copyable_v<SkyboxPacketData>,"Render View Packet Datas must remain trivially copyable");

struct PickPacketData {
    DynamicArray<GeometryRenderData>* worldMeshData = nullptr;
    MeshPacketData uiMeshData{};
    unsigned int uiGeometryCount = 0;
    unsigned int textCount = 0;
    UIText** texts = nullptr;
};
static_assert(std::is_trivially_copyable_v<PickPacketData>,"Render View Packet Datas must remain trivially copyable");