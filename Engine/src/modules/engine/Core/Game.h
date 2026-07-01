#pragma once
#include <memory>

#include "Platform.h"
#include "foxfire_export.h"


class FOXFIRE_API Game {
private:
    Platform* platform = new Platform("FoxFire Engine Sandbox", 100, 100, 1280, 720);

    void startup();

public:
    Game();
    ~Game();
};
