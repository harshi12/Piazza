#include <atomic>

std::atomic<bool> isLocked;

bool lock() { return !isLocked.exchange(true); }

void unlock() { isLocked = false; }