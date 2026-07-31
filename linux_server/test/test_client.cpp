// test_client.cpp
//
// Stability / correctness test client for the Linux HTTP server build.
// Connects to a running server_test instance and exercises the following
// HTTP/1.1 request patterns:
//
//  1. Single GET per connection – repeated for all 200 test files
//  2. Sequential GETs on one keep-alive connection
//  3. Pipelined GETs (N requests sent before reading any response)
//  4. Partial send  – request split across two writes with a short gap
//  5. Connection close mid-response – server must not crash
//  6. Stress – 500 rapid sequential requests across persistent connections
//
// Usage:  ./test_client [host] [port] [--test <name>]
//         ./test_client [host] [port] --client <name> <behavior> <count>
//                                     [pipelineDepth|seed] [--client ...]
//         Default host 127.0.0.1, default port 8080.
//
// Exit code 0 = all tests passed, non-zero = at least one failure.

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// Test file parameters – must match linux_server/test/filesystem/filedata-test.h
// ---------------------------------------------------------------------------

static constexpr int  TEST_NUM_FILES = 200;
static constexpr unsigned int TEST_MIN_SIZE = 10u * 1024u;
static constexpr unsigned int TEST_MAX_SIZE = 1024u * 1024u;

// Compute expected file size for file i (0-based), same formula as server.
static unsigned int expectedFileSize(int i)
{
    if (TEST_NUM_FILES <= 1) return TEST_MIN_SIZE;
    return static_cast<unsigned int>(
        TEST_MIN_SIZE +
        (unsigned long long)(TEST_MAX_SIZE - TEST_MIN_SIZE) * (unsigned int)i
        / (unsigned int)(TEST_NUM_FILES - 1)
    );
}

// Expected byte value at offset j in file i (0-based).
static unsigned char expectedByte(int fileIdx, unsigned int byteOffset)
{
    return static_cast<unsigned char>((fileIdx * 7 + byteOffset) & 0xFF);
}

// URL for file i (0-based).
static std::string fileURL(int i)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "/testfile%03d.html", i + 1);
    return buf;
}

// ---------------------------------------------------------------------------
// Low-level socket helpers
// ---------------------------------------------------------------------------

static const char* g_host = "127.0.0.1";
static int         g_port = 8080;
static std::mutex  g_logMutex;

static void logRequestDetails(const std::string& testType,
                              const std::string& fileDesc,
                              const std::string& request)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    printf("  [Request] test=%s\n", testType.c_str());
    printf("  [Request] file=%s\n", fileDesc.c_str());
    printf("  [Request] raw=\n%s", request.c_str());
}

static void logResponseDetails(const std::string& testType,
                               const std::string& fileDesc,
                               const std::string& header,
                               int contentLength)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    printf("  [Response] test=%s\n", testType.c_str());
    printf("  [Response] file=%s\n", fileDesc.c_str());
    if (!header.empty()) {
        printf("  [Response] headers=\n%s", header.c_str());
    } else {
        printf("  [Response] headers=<none>\n");
    }
    printf("  [Response] content-length=%d\n", contentLength);
}

static std::string joinFileList(const std::vector<std::string>& files)
{
    std::ostringstream oss;
    for (size_t i = 0; i < files.size(); i++) {
        if (i != 0) oss << ", ";
        oss << files[i];
    }
    return oss.str();
}

// Open a blocking TCP connection to g_host:g_port.
// Returns socket fd or -1 on error.
static int openConnection()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    // disable Nagle for faster partial-send tests
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(static_cast<uint16_t>(g_port));
    inet_pton(AF_INET, g_host, &addr.sin_addr);

    if (connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

// Send all bytes; returns false on error.
static bool sendAll(int fd, const void* buf, size_t len)
{
    const auto* p = static_cast<const char*>(buf);
    while (len > 0) {
        ssize_t n = send(fd, p, len, MSG_NOSIGNAL);
        if (n <= 0) return false;
        p   += n;
        len -= static_cast<size_t>(n);
    }
    return true;
}

// Receive exactly len bytes into buf; timeout_ms per poll() call.
// Returns false on error or timeout.
static bool recvExact(int fd, void* buf, size_t len, int timeout_ms = 10000)
{
    auto* p = static_cast<unsigned char*>(buf);
    while (len > 0) {
        struct pollfd pfd{fd, POLLIN, 0};
        int r = poll(&pfd, 1, timeout_ms);
        if (r <= 0) return false;                   // timeout or error

        ssize_t n = recv(fd, p, len, 0);
        if (n <= 0) return false;
        p   += n;
        len -= static_cast<size_t>(n);
    }
    return true;
}

// ---------------------------------------------------------------------------
// HTTP response reader
// ---------------------------------------------------------------------------

// Receive the HTTP response header (everything up to and including \r\n\r\n).
// Stores the excess bytes (start of body) in bodyPrefix.
// Returns the header string (including the blank line) or "" on error.
static std::string recvHeader(int fd, std::vector<unsigned char>& bodyPrefix,
                               int timeout_ms = 10000)
{
    std::string acc;
    acc.reserve(512);
    unsigned char c;

    while (true) {
        struct pollfd pfd{fd, POLLIN, 0};
        if (poll(&pfd, 1, timeout_ms) <= 0) return "";

        ssize_t n = recv(fd, &c, 1, 0);
        if (n <= 0) return "";

        acc += static_cast<char>(c);

        if (acc.size() >= 4 &&
            acc[acc.size()-4] == '\r' && acc[acc.size()-3] == '\n' &&
            acc[acc.size()-2] == '\r' && acc[acc.size()-1] == '\n')
        {
            break;  // full header received
        }
    }
    (void)bodyPrefix;
    return acc;
}

// Extract the value of a header field (case-insensitive field name).
static std::string headerValue(const std::string& header, const std::string& field)
{
    std::string lower = header;
    std::string lfield = field;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    std::transform(lfield.begin(), lfield.end(), lfield.begin(), ::tolower);

    auto pos = lower.find(lfield + ":");
    if (pos == std::string::npos) return "";
    pos += lfield.size() + 1;
    // skip leading spaces
    while (pos < lower.size() && lower[pos] == ' ') pos++;
    auto end = lower.find("\r\n", pos);
    if (end == std::string::npos) return "";
    return header.substr(pos, end - pos);
}

static std::string trimCopy(std::string value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.erase(value.begin());
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
    return value;
}

static bool verifyContentTypeHeader(const std::string& header,
                                    const std::string& expectedType)
{
    std::string contentType = trimCopy(headerValue(header, "Content-Type"));
    if (contentType.empty()) {
        fprintf(stderr, "  missing Content-Type header\n");
        return false;
    }

    const auto separator = contentType.find(';');
    if (separator != std::string::npos) {
        contentType = trimCopy(contentType.substr(0, separator));
    }

    if (contentType != expectedType) {
        fprintf(stderr, "  content-type mismatch: got '%s' expected '%s'\n",
                contentType.c_str(), expectedType.c_str());
        return false;
    }
    return true;
}

// Parse the HTTP status code from the status line.
static int parseStatusCode(const std::string& header)
{
    // "HTTP/1.1 200 OK\r\n..."
    auto sp = header.find(' ');
    if (sp == std::string::npos) return -1;
    return atoi(header.c_str() + sp + 1);
}

// Read a complete HTTP response (header + body).
// contentLen set to parsed Content-Length (-1 if absent).
struct HttpResponse {
    int status        = -1;
    int contentLength = -1;
    std::string header;
    std::vector<unsigned char> body;
    bool ok           = false;
};

static HttpResponse readResponse(int fd, int timeout_ms = 10000)
{
    HttpResponse resp;
    std::vector<unsigned char> dummy;

    std::string header = recvHeader(fd, dummy, timeout_ms);
    if (header.empty()) return resp;

    resp.header = header;
    resp.status = parseStatusCode(header);

    std::string clStr = headerValue(header, "Content-Length");
    if (!clStr.empty()) {
        resp.contentLength = atoi(clStr.c_str());
    }

    if (resp.contentLength > 0) {
        resp.body.resize(static_cast<size_t>(resp.contentLength));
        if (!recvExact(fd, resp.body.data(),
                       static_cast<size_t>(resp.contentLength), timeout_ms)) {
            return resp;
        }
    }
    resp.ok = true;
    return resp;
}

// Build a minimal HTTP/1.1 GET request string.
static std::string makeGet(const std::string& url, const std::string& host)
{
    return "GET " + url + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: keep-alive\r\n\r\n";
}

// ---------------------------------------------------------------------------
// Content verification
// ---------------------------------------------------------------------------

// Verify the body bytes against the deterministic pattern for file fileIdx.
// For large files only the first and last 512 bytes are checked (performance).
static bool verifyBody(const std::vector<unsigned char>& body, int fileIdx,
                       unsigned int expectedLen)
{
    if (body.size() != expectedLen) {
        fprintf(stderr, "  body size mismatch: got %zu expected %u\n",
                body.size(), expectedLen);
        return false;
    }

    // Full verification for files up to 128 KB; spot-check for larger ones.
    unsigned int checkEnd = (expectedLen <= 128u * 1024u)
                          ? expectedLen
                          : std::min(512u, expectedLen);

    for (unsigned int j = 0; j < checkEnd; j++) {
        if (body[j] != expectedByte(fileIdx, j)) {
            fprintf(stderr, "  content mismatch at offset %u: got 0x%02x expected 0x%02x\n",
                    j, body[j], expectedByte(fileIdx, j));
            return false;
        }
    }

    if (expectedLen > 128u * 1024u) {
        // also check last 512 bytes
        unsigned int start = expectedLen - 512u;
        for (unsigned int j = start; j < expectedLen; j++) {
            if (body[j] != expectedByte(fileIdx, j)) {
                fprintf(stderr, "  content mismatch at offset %u\n", j);
                return false;
            }
        }
    }
    return true;
}

static bool verifyHtmlFileResponse(const HttpResponse& resp, int fileIdx,
                                   unsigned int expectedLen)
{
    return resp.ok &&
           resp.status == 200 &&
           verifyContentTypeHeader(resp.header, "text/html") &&
           verifyBody(resp.body, fileIdx, expectedLen);
}

static bool verifyHtml404Response(const HttpResponse& resp)
{
    return resp.ok &&
           resp.status == 404 &&
           verifyContentTypeHeader(resp.header, "text/html");
}

// ---------------------------------------------------------------------------
// Test result tracking
// ---------------------------------------------------------------------------

static int g_pass = 0;
static int g_fail = 0;

#define PASS(msg)  do { printf("  PASS: %s\n", msg); g_pass++; } while(0)
#define FAIL(msg)  do { fprintf(stderr, "  FAIL: %s\n", msg); g_fail++; } while(0)

// ---------------------------------------------------------------------------
// Test 1 – Single GET per connection
// One new connection per file; all 200 test files.
// ---------------------------------------------------------------------------
static void testSingleGetPerConnection()
{
    const std::string testType = "single";
    printf("[Test 1] Single GET per connection (%d files)\n", TEST_NUM_FILES);

    int pass = 0, fail = 0;
    for (int i = 0; i < TEST_NUM_FILES; i++) {
        int fd = openConnection();
        if (fd < 0) { fail++; continue; }

        std::string file = fileURL(i);
        std::string req = makeGet(file, g_host);
        logRequestDetails(testType, file, req);
        if (!sendAll(fd, req.data(), req.size())) {
            close(fd); fail++; continue;
        }

        HttpResponse resp = readResponse(fd);
        logResponseDetails(testType, file, resp.header, resp.contentLength);
        close(fd);

        unsigned int sz = expectedFileSize(i);
        if (!verifyHtmlFileResponse(resp, i, sz)) {
            fail++;
        } else {
            pass++;
        }
    }
    printf("  %d/%d files OK\n", pass, TEST_NUM_FILES);
    if (fail == 0) PASS("all single-GET requests served correctly");
    else { char m[64]; snprintf(m,sizeof(m),"%d requests failed",fail); FAIL(m); }
}

// ---------------------------------------------------------------------------
// Test 2 – Sequential GETs on one persistent connection
// 50 different files; one connection, each response read before next request.
// ---------------------------------------------------------------------------
static void testSequentialGetPersistent()
{
    const std::string testType = "sequential";
    printf("[Test 2] Sequential GETs on persistent connection (50 files)\n");

    int fd = openConnection();
    if (fd < 0) { FAIL("could not connect"); return; }

    int pass = 0, fail = 0;
    for (int k = 0; k < 50; k++) {
        int i = k % TEST_NUM_FILES;
        std::string file = fileURL(i);
        std::string req = makeGet(file, g_host);
        logRequestDetails(testType, file, req);
        if (!sendAll(fd, req.data(), req.size())) { fail++; break; }

        HttpResponse resp = readResponse(fd);
        logResponseDetails(testType, file, resp.header, resp.contentLength);
        unsigned int sz = expectedFileSize(i);
        if (!verifyHtmlFileResponse(resp, i, sz))
            fail++;
        else
            pass++;
    }
    close(fd);
    printf("  %d/50 responses OK\n", pass);
    if (fail == 0) PASS("all sequential keep-alive GETs served correctly");
    else { char m[64]; snprintf(m,sizeof(m),"%d requests failed",fail); FAIL(m); }
}

// ---------------------------------------------------------------------------
// Test 3 – Pipelined GETs
// Send K requests in one burst, then read K responses.
// The server processes requests one per loop iteration, so pipeline depth is
// bounded to keep the test duration reasonable.
// ---------------------------------------------------------------------------
static void testPipelinedGet()
{
    const std::string testType = "pipelined";
    printf("[Test 3] Pipelined GETs\n");

    // Pipeline depth per burst
    static constexpr int DEPTH = 8;
    // Number of bursts
    static constexpr int BURSTS = 5;
    int total_pass = 0, total_fail = 0;

    for (int b = 0; b < BURSTS; b++) {
        int fd = openConnection();
        if (fd < 0) { total_fail += DEPTH; continue; }

        // Build and send all DEPTH requests in one write
        std::string bulk;
        std::vector<std::string> files;
        for (int k = 0; k < DEPTH; k++) {
            int i = (b * DEPTH + k) % TEST_NUM_FILES;
            std::string file = fileURL(i);
            files.push_back(file);
            bulk += makeGet(file, g_host);
        }
        logRequestDetails(testType, joinFileList(files), bulk);
        if (!sendAll(fd, bulk.data(), bulk.size())) {
            close(fd); total_fail += DEPTH; continue;
        }

        // Read all DEPTH responses
        for (int k = 0; k < DEPTH; k++) {
            int i = (b * DEPTH + k) % TEST_NUM_FILES;
            std::string file = fileURL(i);
            HttpResponse resp = readResponse(fd);
            logResponseDetails(testType, file, resp.header, resp.contentLength);
            unsigned int sz = expectedFileSize(i);
            if (!verifyHtmlFileResponse(resp, i, sz))
                total_fail++;
            else
                total_pass++;
        }
        close(fd);
    }
    printf("  %d/%d pipelined responses OK\n", total_pass, DEPTH * BURSTS);
    if (total_fail == 0)
        PASS("all pipelined GETs served correctly");
    else {
        char m[64]; snprintf(m,sizeof(m),"%d pipelined responses failed",total_fail); FAIL(m);
    }
}

// ---------------------------------------------------------------------------
// Test 4 – Partial sends
// Request is split into two writes with a 50 ms gap between them.
// The parser must buffer the partial data and complete on the second write.
// ---------------------------------------------------------------------------
static void testPartialSend()
{
    const std::string testType = "partial";
    printf("[Test 4] Partial GET request sends (20 files)\n");

    int pass = 0, fail = 0;
    for (int k = 0; k < 20; k++) {
        int i = k % TEST_NUM_FILES;
        int fd = openConnection();
        if (fd < 0) { fail++; continue; }

        std::string file = fileURL(i);
        std::string req = makeGet(file, g_host);
        logRequestDetails(testType, file, req);

        // Split at ≈ 60 % of the request length (guaranteed inside the header)
        size_t split = req.size() * 6 / 10;
        if (split == 0) split = 1;
        if (split >= req.size()) split = req.size() - 1;

        bool ok = sendAll(fd, req.data(), split);
        usleep(50 * 1000);   // 50 ms gap
        ok = ok && sendAll(fd, req.data() + split, req.size() - split);

        if (!ok) { close(fd); fail++; continue; }

        HttpResponse resp = readResponse(fd);
        logResponseDetails(testType, file, resp.header, resp.contentLength);
        close(fd);

        unsigned int sz = expectedFileSize(i);
        if (!verifyHtmlFileResponse(resp, i, sz))
            fail++;
        else
            pass++;
    }
    printf("  %d/20 partial-send requests OK\n", pass);
    if (fail == 0) PASS("all partial-send requests served correctly");
    else { char m[64]; snprintf(m,sizeof(m),"%d partial-send requests failed",fail); FAIL(m); }
}

// ---------------------------------------------------------------------------
// Test 5 – Connection close mid-response
// Client reads only a small slice of the response body then closes the socket.
// The server must handle EPIPE / ECONNRESET without crashing.
// ---------------------------------------------------------------------------
static void testConnectionCloseMidResponse()
{
    const std::string testType = "midclose";
    printf("[Test 5] Connection close mid-response (10 large files)\n");

    int ok_count = 0;
    for (int k = 0; k < 10; k++) {
        // Use large files (last quarter of the list) so we can cut mid-body
        int i = TEST_NUM_FILES - 10 + k;
        int fd = openConnection();
        if (fd < 0) continue;

        std::string file = fileURL(i);
        std::string req = makeGet(file, g_host);
        logRequestDetails(testType, file, req);
        if (!sendAll(fd, req.data(), req.size())) { close(fd); continue; }

        // Read the header
        std::vector<unsigned char> dummy;
        std::string header = recvHeader(fd, dummy);
        int contentLength = atoi(headerValue(header, "Content-Length").c_str());
        logResponseDetails(testType, file, header, contentLength);
        if (header.empty()) { close(fd); continue; }

        // Read only first 1 KB of body, then close abruptly
        unsigned char partial[1024];
        ssize_t n = recv(fd, partial, sizeof(partial), 0);
        (void)n;
        close(fd);  // triggers ECONNRESET / EPIPE on the server side
        ok_count++;
    }

    // Sleep briefly so the server has time to process the closed connections
    usleep(200 * 1000);

    // Issue one normal request to confirm the server is still alive
    int fd = openConnection();
    if (fd < 0) {
        FAIL("server not responding after mid-response connection closes");
        return;
    }
    std::string file = fileURL(0);
    std::string req = makeGet(file, g_host);
    logRequestDetails(testType, file, req);
    sendAll(fd, req.data(), req.size());
    HttpResponse resp = readResponse(fd);
    logResponseDetails(testType, file, resp.header, resp.contentLength);
    close(fd);

    if (resp.ok && resp.status == 200 &&
        verifyContentTypeHeader(resp.header, "text/html"))
        PASS("server alive after mid-response connection closes");
    else
        FAIL("server unresponsive after mid-response connection closes");
}

// ---------------------------------------------------------------------------
// Test 6 – Stress: 500 sequential requests across persistent connections
// Files are cycled round-robin; only Content-Length is verified (no body copy).
// ---------------------------------------------------------------------------
static void testStressSequential()
{
    const std::string testType = "stress";
    static constexpr int TOTAL = 500;
    static constexpr int BATCH =  50; // requests per connection

    printf("[Test 6] Stress: %d sequential requests (%d per connection)\n",
           TOTAL, BATCH);

    int pass = 0, fail = 0;

    for (int base = 0; base < TOTAL; base += BATCH) {
        int fd = openConnection();
        if (fd < 0) { fail += BATCH; continue; }

        for (int k = 0; k < BATCH && (base + k) < TOTAL; k++) {
            int i   = (base + k) % TEST_NUM_FILES;
            unsigned int expectedLen = expectedFileSize(i);

            std::string file = fileURL(i);
            std::string req = makeGet(file, g_host);
            logRequestDetails(testType, file, req);
            if (!sendAll(fd, req.data(), req.size())) { fail++; break; }

            // Read header
            std::vector<unsigned char> dummy;
            std::string header = recvHeader(fd, dummy);
            int cl = atoi(headerValue(header, "Content-Length").c_str());
            logResponseDetails(testType, file, header, cl);
            if (header.empty()) { fail++; break; }

            if (!verifyContentTypeHeader(header, "text/html") ||
                cl < 0 || (unsigned int)cl != expectedLen) { fail++; break; }

            // Consume body without copying to vector (saves memory on stress run)
            std::vector<unsigned char> body(static_cast<size_t>(cl));
            if (!recvExact(fd, body.data(), static_cast<size_t>(cl))) {
                fail++; break;
            }
            pass++;
        }
        close(fd);
    }
    printf("  %d/%d stress requests OK\n", pass, TOTAL);
    if (fail == 0)
        PASS("all stress requests served correctly");
    else {
        char m[64]; snprintf(m,sizeof(m),"%d stress requests failed",fail); FAIL(m);
    }
}

// ---------------------------------------------------------------------------
// Test 7 – 404 response for unknown URL
// ---------------------------------------------------------------------------
static void test404Response()
{
    const std::string testType = "404";
    printf("[Test 7] 404 response for unknown URL\n");

    int fd = openConnection();
    if (fd < 0) { FAIL("could not connect"); return; }

    std::string file = "/no/such/file.txt";
    std::string req = makeGet(file, g_host);
    logRequestDetails(testType, file, req);
    if (!sendAll(fd, req.data(), req.size())) { close(fd); FAIL("send failed"); return; }

    HttpResponse resp = readResponse(fd);
    logResponseDetails(testType, file, resp.header, resp.contentLength);
    close(fd);

    if (verifyHtml404Response(resp))
        PASS("404 returned for unknown URL");
    else {
        fprintf(stderr, "  status=%d ok=%d\n", resp.status, (int)resp.ok);
        FAIL("expected 404");
    }
}

// ---------------------------------------------------------------------------
// Test 8 – Small files (first 10) vs large files (last 10)
// Validates correctness at the extremes of the size range.
// ---------------------------------------------------------------------------
static void testSizeExtremes()
{
    const std::string testType = "extremes";
    printf("[Test 8] Small files (first 10) and large files (last 10)\n");

    int pass = 0, fail = 0;
    std::vector<int> indices;
    for (int i = 0; i < 10; i++)                    indices.push_back(i);
    for (int i = TEST_NUM_FILES - 10; i < TEST_NUM_FILES; i++) indices.push_back(i);

    for (int i : indices) {
        int fd = openConnection();
        if (fd < 0) { fail++; continue; }

        std::string file = fileURL(i);
        std::string req = makeGet(file, g_host);
        logRequestDetails(testType, file, req);
        if (!sendAll(fd, req.data(), req.size())) { close(fd); fail++; continue; }

        HttpResponse resp = readResponse(fd, 30000);  // large files need more time
        logResponseDetails(testType, file, resp.header, resp.contentLength);
        close(fd);

        unsigned int sz = expectedFileSize(i);
        if (!verifyHtmlFileResponse(resp, i, sz))
            fail++;
        else
            pass++;
    }
    printf("  %d/20 extreme-size requests OK\n", pass);
    if (fail == 0) PASS("small and large file extremes served correctly");
    else { char m[64]; snprintf(m,sizeof(m),"%d size-extreme requests failed",fail); FAIL(m); }
}

// ---------------------------------------------------------------------------
// Multi-client framework
//
// ClientBehavior controls what each concurrent client thread does.
// ClientProfile bundles a behavior with its parameters so that any number
// of profiles can be handed to runMultiClient(), which launches one
// std::thread per profile and runs them all simultaneously.
// ---------------------------------------------------------------------------

enum class ClientBehavior {
    SINGLE_GETS,         // one GET per connection, repeated requestCount times
    BIG_FILES_ONLY,      // request only the N largest test files (by index)
    RANDOM_FILES,        // request files chosen with a simple LCG RNG
    PARTIAL_SENDS,       // split every request across two writes with a 50 ms gap
    SEQUENTIAL_PERSIST,  // sequential GETs batched on persistent connections
    PIPELINED,           // pipeline bursts of pipelineDepth requests
};

static const char* behaviorName(ClientBehavior b)
{
    switch (b) {
    case ClientBehavior::SINGLE_GETS:         return "SINGLE_GETS";
    case ClientBehavior::BIG_FILES_ONLY:     return "BIG_FILES_ONLY";
    case ClientBehavior::RANDOM_FILES:       return "RANDOM_FILES";
    case ClientBehavior::PARTIAL_SENDS:      return "PARTIAL_SENDS";
    case ClientBehavior::SEQUENTIAL_PERSIST: return "SEQUENTIAL_PERSIST";
    case ClientBehavior::PIPELINED:          return "PIPELINED";
    }
    return "UNKNOWN";
}

struct ClientProfile {
    std::string    name;
    ClientBehavior behavior;
    int            requestCount;   // total requests to issue
    int            pipelineDepth;  // burst size for PIPELINED (default 8)
    unsigned int   seed;           // RNG seed for RANDOM_FILES

    ClientProfile(std::string n, ClientBehavior b, int rc,
                  int pd = 8, unsigned int s = 42u)
        : name(std::move(n)), behavior(b), requestCount(rc),
          pipelineDepth(pd), seed(s) {}
};

struct ClientResult {
    std::string name;
    int  pass      = 0;
    int  fail      = 0;
    bool completed = false;
};

// Run a single ClientProfile and return its result.
// This function is called in its own thread; it uses only thread-local state
// (local variables + per-call socket fds).  g_host / g_port are read-only.
static ClientResult runClientProfile(const ClientProfile& profile)
{
    ClientResult result;
    result.name = profile.name;
    const std::string testType = "client:" + profile.name;

    // Simple multiplicative LCG for RANDOM_FILES.
    unsigned int rng = profile.seed;
    auto nextRand = [&]() -> int {
        rng = rng * 1664525u + 1013904223u;
        return static_cast<int>((rng >> 1) % static_cast<unsigned int>(TEST_NUM_FILES));
    };

    switch (profile.behavior) {

    // ---- SINGLE_GETS ----------------------------------------------------
    // Opens one connection per request, cycling through the existing files.
    case ClientBehavior::SINGLE_GETS: {
        for (int k = 0; k < profile.requestCount; k++) {
            int i = k % TEST_NUM_FILES;
            int fd = openConnection();
            if (fd < 0) { result.fail++; continue; }

            std::string file = fileURL(i);
            std::string req = makeGet(file, g_host);
            logRequestDetails(testType, file, req);
            if (!sendAll(fd, req.data(), req.size())) {
                close(fd); result.fail++; continue;
            }
            HttpResponse resp = readResponse(fd);
            logResponseDetails(testType, file, resp.header, resp.contentLength);
            close(fd);

            unsigned int sz = expectedFileSize(i);
            if (!verifyHtmlFileResponse(resp, i, sz))
                result.fail++;
            else
                result.pass++;
        }
        break;
    }

    // ---- BIG_FILES_ONLY ------------------------------------------------
    // Requests the last min(requestCount, TEST_NUM_FILES) files by index,
    // cycling round-robin when requestCount > that pool.
    case ClientBehavior::BIG_FILES_ONLY: {
        int pool = std::min(profile.requestCount, TEST_NUM_FILES);
        for (int k = 0; k < profile.requestCount; k++) {
            // cycle through the largest `pool` files (highest indices)
            int i = TEST_NUM_FILES - 1 - (k % pool);
            int fd = openConnection();
            if (fd < 0) { result.fail++; continue; }

            std::string file = fileURL(i);
            std::string req = makeGet(file, g_host);
            logRequestDetails(testType, file, req);
            if (!sendAll(fd, req.data(), req.size())) {
                close(fd); result.fail++; continue;
            }
            HttpResponse resp = readResponse(fd, 30000);
            logResponseDetails(testType, file, resp.header, resp.contentLength);
            close(fd);

            unsigned int sz = expectedFileSize(i);
            if (!verifyHtmlFileResponse(resp, i, sz))
                result.fail++;
            else
                result.pass++;
        }
        break;
    }

    // ---- RANDOM_FILES --------------------------------------------------
    // Picks file indices at random (seeded per-profile for reproducibility).
    case ClientBehavior::RANDOM_FILES: {
        for (int k = 0; k < profile.requestCount; k++) {
            int i  = nextRand();
            int fd = openConnection();
            if (fd < 0) { result.fail++; continue; }

            std::string file = fileURL(i);
            std::string req = makeGet(file, g_host);
            logRequestDetails(testType, file, req);
            if (!sendAll(fd, req.data(), req.size())) {
                close(fd); result.fail++; continue;
            }
            HttpResponse resp = readResponse(fd, 30000);
            logResponseDetails(testType, file, resp.header, resp.contentLength);
            close(fd);

            unsigned int sz = expectedFileSize(i);
            if (!verifyHtmlFileResponse(resp, i, sz))
                result.fail++;
            else
                result.pass++;
        }
        break;
    }

    // ---- PARTIAL_SENDS -------------------------------------------------
    // Each request is split into two writes with a 50 ms pause between them.
    case ClientBehavior::PARTIAL_SENDS: {
        for (int k = 0; k < profile.requestCount; k++) {
            int i  = k % TEST_NUM_FILES;
            int fd = openConnection();
            if (fd < 0) { result.fail++; continue; }

            std::string file = fileURL(i);
            std::string req = makeGet(file, g_host);
            logRequestDetails(testType, file, req);
            size_t split = req.size() * 6 / 10;
            if (split == 0)          split = 1;
            if (split >= req.size()) split = req.size() - 1;

            bool ok = sendAll(fd, req.data(), split);
            usleep(50 * 1000);   // 50 ms gap
            ok = ok && sendAll(fd, req.data() + split, req.size() - split);

            if (!ok) { close(fd); result.fail++; continue; }

            HttpResponse resp = readResponse(fd);
            logResponseDetails(testType, file, resp.header, resp.contentLength);
            close(fd);

            unsigned int sz = expectedFileSize(i);
            if (!verifyHtmlFileResponse(resp, i, sz))
                result.fail++;
            else
                result.pass++;
        }
        break;
    }

    // ---- SEQUENTIAL_PERSIST --------------------------------------------
    // Batches requests on a single keep-alive connection; opens a new one
    // every 50 requests (mirrors testSequentialGetPersistent).
    case ClientBehavior::SEQUENTIAL_PERSIST: {
        static constexpr int BATCH = 50;
        int remaining = profile.requestCount;
        int fileIdx   = 0;
        while (remaining > 0) {
            int fd = openConnection();
            if (fd < 0) { result.fail += remaining; break; }

            int batch = std::min(BATCH, remaining);
            for (int k = 0; k < batch; k++) {
                int i = fileIdx++ % TEST_NUM_FILES;
                std::string file = fileURL(i);
                std::string req = makeGet(file, g_host);
                logRequestDetails(testType, file, req);
                if (!sendAll(fd, req.data(), req.size())) {
                    result.fail++; break;
                }
                HttpResponse resp = readResponse(fd);
                logResponseDetails(testType, file, resp.header, resp.contentLength);
                unsigned int sz   = expectedFileSize(i);
                if (!verifyHtmlFileResponse(resp, i, sz))
                    result.fail++;
                else
                    result.pass++;
            }
            close(fd);
            remaining -= batch;
        }
        break;
    }

    // ---- PIPELINED -----------------------------------------------------
    // Sends pipelineDepth requests in one burst before reading any response.
    case ClientBehavior::PIPELINED: {
        int depth = profile.pipelineDepth;
        int sent  = 0;
        while (sent < profile.requestCount) {
            int fd = openConnection();
            if (fd < 0) {
                int skip = std::min(depth, profile.requestCount - sent);
                result.fail += skip;
                sent        += skip;
                continue;
            }
            int burst = std::min(depth, profile.requestCount - sent);
            std::string bulk;
            std::vector<std::string> files;
            for (int k = 0; k < burst; k++) {
                std::string file = fileURL((sent + k) % TEST_NUM_FILES);
                files.push_back(file);
                bulk += makeGet(file, g_host);
            }
            logRequestDetails(testType, joinFileList(files), bulk);

            if (!sendAll(fd, bulk.data(), bulk.size())) {
                close(fd); result.fail += burst; sent += burst; continue;
            }
            for (int k = 0; k < burst; k++) {
                int i = (sent + k) % TEST_NUM_FILES;
                std::string file = fileURL(i);
                HttpResponse resp = readResponse(fd);
                logResponseDetails(testType, file, resp.header, resp.contentLength);
                unsigned int sz   = expectedFileSize(i);
                if (!verifyHtmlFileResponse(resp, i, sz))
                    result.fail++;
                else
                    result.pass++;
            }
            close(fd);
            sent += burst;
        }
        break;
    }

    } // switch

    result.completed = true;
    return result;
}

// Launch one std::thread per profile, run all simultaneously, collect results.
static void runMultiClient(const std::vector<ClientProfile>& profiles)
{
    printf("[Multi-client] %zu clients running simultaneously\n", profiles.size());
    for (const auto& p : profiles) {
        printf("  %-24s  behavior=%-20s  requests=%d\n",
               p.name.c_str(), behaviorName(p.behavior), p.requestCount);
    }
    printf("\n");

    std::vector<ClientResult>  results(profiles.size());
    std::vector<std::thread>   threads;
    threads.reserve(profiles.size());

    for (size_t i = 0; i < profiles.size(); i++) {
        threads.emplace_back([&profiles, &results, i]() {
            results[i] = runClientProfile(profiles[i]);
        });
    }
    for (auto& t : threads) t.join();

    int total_pass = 0, total_fail = 0;
    for (const auto& r : results) {
        printf("  Client %-24s  pass=%-4d  fail=%-4d  %s\n",
               r.name.c_str(), r.pass, r.fail,
               r.completed ? "completed" : "INCOMPLETE");
        total_pass += r.pass;
        total_fail += r.fail;
    }
    printf("  Aggregate: %d passed, %d failed\n", total_pass, total_fail);

    if (total_fail == 0)
        PASS("all multi-client requests served correctly");
    else {
        char m[80];
        snprintf(m, sizeof(m), "%d multi-client request(s) failed", total_fail);
        FAIL(m);
    }
}

// ---------------------------------------------------------------------------
// Test 9 – Multi-client: simultaneous connections with distinct behaviors
//
// Three core profiles mirror the examples from the feature request:
//   client1  → only the 100 largest files
//   client2  → 200 files selected at random
//   client3  → 20 requests with partial sends
// Two additional profiles exercise sequential-persistent and pipelined paths.
// ---------------------------------------------------------------------------
static void testMultiClientParallel()
{
    printf("[Test 9] Multi-client simultaneous connections\n\n");

    std::vector<ClientProfile> profiles = {
        // client1: 100 big files only (largest by index)
        ClientProfile("client1-big-files",  ClientBehavior::BIG_FILES_ONLY,      100),
        // client2: 200 random files (seeded for reproducibility)
        ClientProfile("client2-random",     ClientBehavior::RANDOM_FILES,         200, 8, 98765u),
        // client3: partial-send only
        ClientProfile("client3-partial",    ClientBehavior::PARTIAL_SENDS,         20),
        // client4: sequential on persistent keep-alive connections
        ClientProfile("client4-sequential", ClientBehavior::SEQUENTIAL_PERSIST,    50),
        // client5: pipelined bursts (depth 8, 40 total requests)
        ClientProfile("client5-pipelined",  ClientBehavior::PIPELINED,             40, 8),
    };

    runMultiClient(profiles);
}

static std::string normalizeArg(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) -> unsigned char {
                       return (c == '-') ? '_' : static_cast<unsigned char>(std::tolower(c));
                   });
    return value;
}

static bool parseBehavior(const std::string& raw, ClientBehavior& behavior)
{
    const std::string value = normalizeArg(raw);
    if (value == "single" || value == "single_get" || value == "single_gets") {
        behavior = ClientBehavior::SINGLE_GETS;
        return true;
    }
    if (value == "big" || value == "big_files" || value == "big_files_only") {
        behavior = ClientBehavior::BIG_FILES_ONLY;
        return true;
    }
    if (value == "random" || value == "random_files") {
        behavior = ClientBehavior::RANDOM_FILES;
        return true;
    }
    if (value == "partial" || value == "partial_send" || value == "partial_sends") {
        behavior = ClientBehavior::PARTIAL_SENDS;
        return true;
    }
    if (value == "sequential" || value == "sequential_persist" || value == "persistent") {
        behavior = ClientBehavior::SEQUENTIAL_PERSIST;
        return true;
    }
    if (value == "pipeline" || value == "pipelined") {
        behavior = ClientBehavior::PIPELINED;
        return true;
    }
    return false;
}

static void printUsage(const char* argv0)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s [host] [port]\n"
            "  %s [host] [port] --test <all|single|sequential|pipelined|partial|midclose|stress|404|extremes|multi>\n"
            "  %s [host] [port] --client <name> <single|big|random|partial|sequential|pipelined> <count> [value]\n"
            "\n"
            "Notes:\n"
            "  --test 404 runs the standalone non-existent-file test only.\n"
            "  --client may be repeated to launch one or more simultaneous client profiles.\n"
            "  Optional [value] is RNG seed for random, pipeline depth for pipelined.\n",
            argv0, argv0, argv0);
}

static bool runNamedTest(const std::string& rawName)
{
    const std::string name = normalizeArg(rawName);

    if (name == "all") {
        testSingleGetPerConnection();
        testSequentialGetPersistent();
        testPipelinedGet();
        testPartialSend();
        testConnectionCloseMidResponse();
        testStressSequential();
        test404Response();
        testSizeExtremes();
        testMultiClientParallel();
        return true;
    }
    if (name == "single")                 { testSingleGetPerConnection();   return true; }
    if (name == "sequential")             { testSequentialGetPersistent();  return true; }
    if (name == "pipelined")              { testPipelinedGet();             return true; }
    if (name == "partial")                { testPartialSend();              return true; }
    if (name == "midclose")               { testConnectionCloseMidResponse(); return true; }
    if (name == "stress")                 { testStressSequential();         return true; }
    if (name == "404" || name == "not_found") { test404Response();         return true; }
    if (name == "extremes")               { testSizeExtremes();             return true; }
    if (name == "multi")                  { testMultiClientParallel();      return true; }
    return false;
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    int argi = 1;
    if (argi < argc && strncmp(argv[argi], "--", 2) != 0) g_host = argv[argi++];
    if (argi < argc && strncmp(argv[argi], "--", 2) != 0) g_port = atoi(argv[argi++]);

    std::string testName = "all";
    std::vector<ClientProfile> cliProfiles;

    while (argi < argc) {
        std::string arg = argv[argi++];
        if (arg == "--test") {
            if (argi >= argc) {
                printUsage(argv[0]);
                return 2;
            }
            testName = argv[argi++];
            continue;
        }

        if (arg == "--client") {
            if (argi + 2 >= argc) {
                printUsage(argv[0]);
                return 2;
            }

            std::string name = argv[argi++];
            std::string behaviorRaw = argv[argi++];
            ClientBehavior behavior;
            if (!parseBehavior(behaviorRaw, behavior)) {
                fprintf(stderr, "Unknown client behavior: %s\n", behaviorRaw.c_str());
                printUsage(argv[0]);
                return 2;
            }

            int requestCount = atoi(argv[argi++]);
            int pipelineDepth = 8;
            unsigned int seed = 42u;

            if (behavior == ClientBehavior::PIPELINED &&
                argi < argc && strncmp(argv[argi], "--", 2) != 0) {
                pipelineDepth = atoi(argv[argi++]);
            }
            if (behavior == ClientBehavior::RANDOM_FILES &&
                argi < argc && strncmp(argv[argi], "--", 2) != 0) {
                seed = static_cast<unsigned int>(strtoul(argv[argi++], nullptr, 10));
            }

            cliProfiles.emplace_back(name, behavior, requestCount, pipelineDepth, seed);
            continue;
        }

        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        }

        fprintf(stderr, "Unknown argument: %s\n", arg.c_str());
        printUsage(argv[0]);
        return 2;
    }

    if (!cliProfiles.empty() && normalizeArg(testName) != "all") {
        fprintf(stderr, "--test and --client cannot be combined\n");
        printUsage(argv[0]);
        return 2;
    }

    printf("=== HTTP server stability tests ===\n");
    printf("Target: %s:%d\n\n", g_host, g_port);

    if (!cliProfiles.empty()) {
        runMultiClient(cliProfiles);
    } else if (!runNamedTest(testName)) {
        fprintf(stderr, "Unknown test name: %s\n", testName.c_str());
        printUsage(argv[0]);
        return 2;
    }

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
