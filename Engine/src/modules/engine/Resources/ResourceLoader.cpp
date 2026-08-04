//
// Created by cmorg on 8/3/2026.
//

#include "ResourceLoader.h"


void ResourceLoader::unload(Resource &resource) {
    if (resource.data) {
        FF_Memory::ff_free(resource.data, resource.dataSize, memoryTag);
        resource.data = nullptr;
        resource.dataSize = 0;
        resource.loaderId = INVALID_ID;
        resource.path.clear();
    }
}
