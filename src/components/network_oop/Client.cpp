#include "Client.hpp"

using namespace std;

Client::Client(ClientFD_t ClientFD) :
    HTTPParser(4096),
    _ClientFD(ClientFD)
{
}

Client::~Client()
{
}

bool Client::receiveData()
{
    bool DataInKernelBuffer = true;

    while (DataInKernelBuffer == true) {

        //ssize_t RcvBytes = read(_ClientFD, _ReceiveBuffer, SOCKET_RECEIVE_BUFFER_SIZE);
        ssize_t RcvBytes = read(_ClientFD, _ReceiveBuffer, 1024);

        if (RcvBytes > 0) {
            appendBuffer(_ReceiveBuffer, RcvBytes);
        }
        else if (RcvBytes == 0) {
            DataInKernelBuffer = false;
        }
        else if (RcvBytes < 0) {
            const int RecvErrno = errno;
            DataInKernelBuffer = false;
            if (RecvErrno == EAGAIN || RecvErrno == EWOULDBLOCK || RecvErrno == EINTR) {
                return false;
            }
        }
    }
    return true;
}
