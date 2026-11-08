#include "moonlight/example/example.h"

#include <cassert>
#include <string_view>

int main() {
    assert(std::string_view{moonlight::example::greeting()} == "Hello from Moonlight");
    return 0;
}
