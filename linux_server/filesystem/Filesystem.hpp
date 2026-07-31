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

#include "../../../src/components/filesystem/filedata.h"
#define ServerFiles BaseServerFiles
#include "../../../src/components/filesystem/filemetadata.h"
#undef ServerFiles

#include "../generated_fs/filedata-test.h"
#include "../generated_fs/filemetadata-test.h"

class Filesystem
{

private:

public:

    static ServerFile getFileMetadata(string URLPath)
    {
        for(const ServerFile &FileMetadata: ServerFiles) {
            if (FileMetadata.ContentPath == URLPath) {
                return FileMetadata;
            }
        }
        return { "", "", nullptr, 0 };
    }

};
