#include "VKTime.hpp"

chrono::steady_clock::time_point getTime() {
    return chrono::steady_clock::now();
}

float getElapsedSeconds(chrono::steady_clock::time_point start, 
                        chrono::steady_clock::time_point end) {
    return chrono::duration<float, std::chrono::seconds::period>(end - start).count();
}