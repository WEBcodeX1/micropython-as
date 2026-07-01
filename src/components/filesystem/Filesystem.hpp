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

#include "filedata.h"
#include "filemetadata.h"


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
