#include <moonlight/orcha/orcha.h>

#include <cassert>

int main() {
    const auto info = moonlight::orcha::module_info();
    assert(info.api_version == 1U);
    assert(info.name != nullptr);
    return 0;
}
