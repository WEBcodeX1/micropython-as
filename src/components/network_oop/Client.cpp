#include "Client.hpp"

#include "esp_log.h"

using namespace std;


Client::Client(ClientFD_t ClientFD) :
    HTTPParser(1024),
    _ClientFD(ClientFD)
{
}

Client::~Client()
{
}

bool Client::receiveData(char* BufferRef)
{
    bool DataInKernelBuffer = true;

    while (DataInKernelBuffer == true) {

        ssize_t RcvBytes = read(_ClientFD, &BufferRef[0], 1024);

        if (RcvBytes > 0) {
            ESP_LOGI("HTTPServer", "Client received bytes:%d", RcvBytes);
            //ESP_LOG_BUFFER_HEX_LEVEL("HTTPServer", &BufferRef[0], RcvBytes, ESP_LOG_INFO);
            appendBuffer(&BufferRef[0], RcvBytes);
        }
        else if (RcvBytes == 0) {
            DataInKernelBuffer = false;
        }
        else if (RcvBytes < 0) {
            const int RecvErrno = errno;
            DataInKernelBuffer = false;
            //ESP_LOGI("HTTPServer", "Errno:%d", RecvErrno);
            if (RecvErrno == EAGAIN || RecvErrno == EWOULDBLOCK || RecvErrno == EINTR) {
                return false;
            }
        }
    }
    return true;
}

ClientFD_t Client::getClientFD()
{
        return _ClientFD;
}
