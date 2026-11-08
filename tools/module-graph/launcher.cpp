#include <cstdlib>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#ifndef MOONLIGHT_MODULE_GRAPH_PATH
#error "MOONLIGHT_MODULE_GRAPH_PATH must be defined by CMake"
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    return _spawnv(_P_WAIT, MOONLIGHT_MODULE_GRAPH_PATH, argv);
#else
    execv(MOONLIGHT_MODULE_GRAPH_PATH, argv);
    return EXIT_FAILURE;
#endif
}
