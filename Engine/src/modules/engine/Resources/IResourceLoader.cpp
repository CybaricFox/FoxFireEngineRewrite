//
// Created by cmorg on 8/3/2026.
//

#include "IResourceLoader.h"


void IResourceLoader::unload(Resource &resource) {
    if (resource.data) {
        FF_Memory::ff_free_raw(resource.data, resource.dataSize, memoryTag);
        resource.data = nullptr;
        resource.dataSize = 0;
        resource.loaderId = INVALID_ID_U32;
        resource.path.clear();
    }
}
