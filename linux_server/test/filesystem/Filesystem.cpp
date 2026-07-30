#include "Filesystem.hpp"

#include <cstdio>
#include <cstring>
#include <mutex>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

// ---------------------------------------------------------------------------
// Test file parameters
// ---------------------------------------------------------------------------

// Number of test files served at /testfileNNN.bin (NNN = 001 … NUM_TEST_FILES)
static constexpr int TEST_NUM_FILES = 200;
// Smallest file size (file index 0)
static constexpr unsigned int TEST_MIN_SIZE = 10u * 1024u;     // 10 KB
// Largest file size (file index TEST_NUM_FILES - 1)
static constexpr unsigned int TEST_MAX_SIZE = 1024u * 1024u;   // 1 MB

// ---------------------------------------------------------------------------
// Deterministic size and content helpers
// ---------------------------------------------------------------------------

// Linear distribution of file sizes from TEST_MIN_SIZE to TEST_MAX_SIZE.
// i is 0-based.
unsigned int testFileSize(int i)
{
    if (TEST_NUM_FILES <= 1) return TEST_MIN_SIZE;
    return static_cast<unsigned int>(
        TEST_MIN_SIZE +
        (unsigned long long)(TEST_MAX_SIZE - TEST_MIN_SIZE) * (unsigned int)i
        / (unsigned int)(TEST_NUM_FILES - 1)
    );
}

// Fill buf[0..size-1] with the deterministic pattern for file i (0-based).
// Byte at offset j = (uint8_t)((i * 7 + j) & 0xFF)
static void fillContent(unsigned char* buf, unsigned int size, int i)
{
    for (unsigned int j = 0; j < size; j++) {
        buf[j] = static_cast<unsigned char>((i * 7 + j) & 0xFF);
    }
}

// ---------------------------------------------------------------------------
// File store
// ---------------------------------------------------------------------------

struct TestFileEntry {
    string path;
    string contentType;
    vector<unsigned char> data;
};

static vector<TestFileEntry> gTestFiles;
static once_flag             gInitFlag;

static void initTestFiles()
{
    gTestFiles.reserve(static_cast<size_t>(TEST_NUM_FILES) + 1);

    for (int i = 0; i < TEST_NUM_FILES; i++) {
        unsigned int sz = testFileSize(i);
        TestFileEntry entry;

        char url[32];
        snprintf(url, sizeof(url), "/testfile%03d.bin", i + 1);
        entry.path        = url;
        entry.contentType = "application/octet-stream";
        entry.data.resize(sz);
        fillContent(entry.data.data(), sz, i);
        gTestFiles.push_back(std::move(entry));
    }

    // 404 fallback page
    static const char kNotFound[] = "<html><body>404 Not Found</body></html>";
    TestFileEntry e404;
    e404.path        = "/404.html";
    e404.contentType = "text/html";
    e404.data.assign(kNotFound, kNotFound + strlen(kNotFound));
    gTestFiles.push_back(std::move(e404));
}

// ---------------------------------------------------------------------------
// Filesystem::getFileMetadata
// ---------------------------------------------------------------------------

ServerFile Filesystem::getFileMetadata(string URLPath)
{
    call_once(gInitFlag, initTestFiles);

    for (const auto& entry : gTestFiles) {
        if (entry.path == URLPath) {
            return {
                entry.path,
                entry.contentType,
                entry.data.data(),
                static_cast<unsigned int>(entry.data.size())
            };
        }
    }
    return { "", "", nullptr, 0 };
}
