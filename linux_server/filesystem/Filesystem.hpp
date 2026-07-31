#pragma once

#include <array>
#include <string>

using namespace std;

struct ServerFile {
    string ContentPath;
    string ContentType;
    const unsigned char* ContentPointer;
    unsigned int ContentLength;
};

#include "../../src/components/filesystem/filedata.h"
#define ServerFiles BaseServerFiles
#include "../../src/components/filesystem/filemetadata.h"
#undef ServerFiles

#if __has_include("filedata-test.h") && __has_include("filemetadata-test.h")
#include "filedata-test.h"
#include "filemetadata-test.h"
#define LINUX_SERVER_HAS_TEST_SERVER_FILES 1
#else
#define LINUX_SERVER_HAS_TEST_SERVER_FILES 0
#endif

class Filesystem
{

private:

public:

    static ServerFile getFileMetadata(string URLPath)
    {
#if LINUX_SERVER_HAS_TEST_SERVER_FILES
        for(const ServerFile &FileMetadata: ServerFiles) {
#else
        for(const ServerFile &FileMetadata: BaseServerFiles) {
#endif
            if (FileMetadata.ContentPath == URLPath) {
                return FileMetadata;
            }
        }
        return { "", "", nullptr, 0 };
    }

};
