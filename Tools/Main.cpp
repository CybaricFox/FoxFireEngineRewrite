//
// Created by cmorg on 10/6/2026.
//

#include <cstring>

#include "FontCompiler.h"

int main(int argc, char* args[]) {
    if (strcmp(args[1], "Compile Font") == 0) {
        FontCompiler compiler{args[2], args[3]};
    }

    return 0;
}
