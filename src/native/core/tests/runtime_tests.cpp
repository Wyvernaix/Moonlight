#include "moonlight/core/runtime.h"

#include <cassert>
#include <string_view>

int main() {
    assert(std::string_view{moonlight::core::runtime_name()} == "Moonlight Core");
    return 0;
}
