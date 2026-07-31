#include "Filesystem.hpp"

#include <mutex>
#include <vector>

using namespace std;

namespace {

static vector<vector<unsigned char>> gTestFiles;
static once_flag                     gInitFlag;

static void initTestFiles()
{
    gTestFiles.resize(TestServerFiles.size());

    for (size_t i = 0; i < TestServerFiles.size(); i++) {
        auto& data = gTestFiles[i];
        data.resize(TestServerFiles[i].ContentLength);
        fillTestFileContent(data, static_cast<int>(i));
    }
}

} // namespace

ServerFile Filesystem::getFileMetadata(string URLPath)
{
    if (URLPath == kNotFoundContentPath) {
        return {
            kNotFoundContentPath,
            kNotFoundContentType,
            kNotFoundFileData,
            static_cast<unsigned int>(sizeof(kNotFoundFileData) - 1)
        };
    }

    call_once(gInitFlag, initTestFiles);

    for (size_t i = 0; i < TestServerFiles.size(); i++) {
        const auto& spec = TestServerFiles[i];
        if (spec.ContentPath == URLPath) {
            return {
                spec.ContentPath,
                spec.ContentType,
                gTestFiles[i].data(),
                spec.ContentLength
            };
        }
    }

    return { "", "", nullptr, 0 };
}
