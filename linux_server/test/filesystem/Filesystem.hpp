#pragma once

// Test-build replacement for src/components/filesystem/Filesystem.hpp.
// Keeps the production split between file data declarations and file metadata
// declarations, while generating the large HTML test payloads deterministically.

#include <array>
#include <string>
#include <vector>

using namespace std;

struct ServerFile {
    string ContentPath;
    string ContentType;
    const unsigned char* ContentPointer;
    unsigned int ContentLength;
};

struct TestFileSpec {
    const char* ContentPath;
    const char* ContentType;
    unsigned int ContentLength;
};

#include "filedata-test.h"
#include "filemetadata-test.h"

class Filesystem {
public:
    static ServerFile getFileMetadata(string URLPath);
};
