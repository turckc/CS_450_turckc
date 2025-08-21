#pragma once
#include <chrono>
#include <atomic>

using namespace std;

chrono::steady_clock::time_point getTime();
float getElapsedSeconds(chrono::steady_clock::time_point start, chrono::steady_clock::time_point end);