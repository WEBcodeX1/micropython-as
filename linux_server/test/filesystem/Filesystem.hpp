#pragma once

// Test-build replacement for src/components/filesystem/Filesystem.hpp.
// Provides 200 files sized from 10 KB to 1 MB with deterministic byte content —
// no embedded C arrays needed.  Content is generated once on first access.
//
// File URL scheme:  /testfileNNN.bin  (NNN = 001 … 200)
// Content byte at offset j in file i (0-based):  (uint8_t)((i * 7 + j) & 0xFF)
// Additionally serves /404.html for 404 responses.

#include <string>
#include <vector>

using namespace std;

struct ServerFile {
    string ContentPath;
    string ContentType;
    const unsigned char* ContentPointer;
    unsigned int ContentLength;
};

class Filesystem {
public:
    static ServerFile getFileMetadata(string URLPath);
};
