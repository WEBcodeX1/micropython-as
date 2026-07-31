#pragma once

#include <vector>

static constexpr int TEST_NUM_FILES = 200;
static constexpr unsigned int TEST_MIN_SIZE = 10u * 1024u;
static constexpr unsigned int TEST_MAX_SIZE = 1024u * 1024u;

static constexpr unsigned int testFileSize(int i)
{
    return (TEST_NUM_FILES <= 1)
        ? TEST_MIN_SIZE
        : static_cast<unsigned int>(
              TEST_MIN_SIZE
              + (static_cast<unsigned long long>(TEST_MAX_SIZE - TEST_MIN_SIZE)
                 * static_cast<unsigned int>(i))
                    / static_cast<unsigned int>(TEST_NUM_FILES - 1));
}

static inline unsigned char testFileByte(int fileIdx, unsigned int byteOffset)
{
    return static_cast<unsigned char>((fileIdx * 7 + byteOffset) & 0xFF);
}

static inline void fillTestFileContent(std::vector<unsigned char>& data, int fileIdx)
{
    for (unsigned int j = 0; j < data.size(); j++) {
        data[j] = testFileByte(fileIdx, j);
    }
}

static const unsigned char kNotFoundFileData[] = "<html><body>404 Not Found</body></html>";
static constexpr const char* kNotFoundContentPath = "/404.html";
static constexpr const char* kNotFoundContentType = "text/html";
