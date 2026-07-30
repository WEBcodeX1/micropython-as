#include "ASRequestGlobal.hpp"
#include "ClientHandler.hpp"
#include "Filesystem.hpp"

#ifdef LINUX_BUILD
#include "/usr/local/include/linux/httpgenerator.hpp"
#else
#include "/usr/local/include/esp32s3/httpgenerator.hpp"
#endif

#include "constants.h"

#include "esp_log.h"

using namespace std;


ClientHandler::ClientHandler()
{
    //- preserve maximum clients
    Clients.reserve(WIFI_MAX_STA_CONN);
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::addClient(const ClientFD_t ClientFD)
{
    //- set client connection non blocking
    Socket::makeNonblocking(ClientFD);

    ESP_LOGI("HTTPServer", "Add client ClientFD:%d", ClientFD);

    ClientRef_t ClientObj = new Client(ClientFD);

    Clients.emplace(
        ClientFD, ClientObj
    );
}

uint8_t ClientHandler::processClients()
{
    //- receive data from all client filedescriptors
    vector<ClientFD_t> EraseFDs;

    //- sum existing messages
    uint8_t sumMessages = 0;

    for (auto const &ClientItem : Clients)
    {
        auto const ReadFD = ClientItem.first;

        //ESP_LOGI("HTTPServer", "Processing ReadFD:%d", ReadFD);

        const ClientRef_t ClientObj = Clients[ReadFD];

        if (ClientObj->receiveData(&receiveBuffer[0]) == true) {
            //ESP_LOGI("HTTPServer", "Close conn received");
            EraseFDs.push_back(ReadFD);
        }

        RequestsMapPtr_t Requests = ClientObj->getRequestsPtr();

        if (Requests->size() > 0) {

            //- add message to messages sum
            sumMessages += 1;

            //ESP_LOGI("HTTPServer", "Requests size:%d", Requests->size());

            const RequestProperties_t Request = Requests->at(0);

            //- check for MicroPython request
            if (ASRequestStatus == AS_REQ_WAIT_IN) {
                ClientObj->ASRequestInit(Request);
            }

            if (ClientObj->MsgGetSendStatus() == SEND_STATE_IDLE) {

                //- reset httpgenerator object
                ClientObj->MsgReset();

                // on AS SEND_RESULT command, send result message
                if (ASRequestStatus == AS_REQ_PROCESSED) {

                    ClientObj->MsgAddDateHeader();
                    ClientObj->MsgAddHeader("Content-Type", "application/json");
                    ClientObj->MsgAddHeader("Connection", "keep-alive");
                    ClientObj->MsgAddHeader("Server", "falcon-as");

                    ClientObj->MsgSetBodyRef(
                        reinterpret_cast<const unsigned char*>(ASRequestExchangeBuffer),
                        ASRequestContentLength
                    );

                    ClientObj->MsgGenerate();
                    ClientObj->MsgSetSendStatus(SEND_STATE_SENDING);

                    //- reset AS status to wait incoming
                    ASRequestStatus = AS_REQ_WAIT_IN;
                }

                //- on non-AS request && GET request (static file)
                else if (Request.HTTPMethod == 1) {

                    //ESP_LOGI("HTTPServer", "Request GET URL:%s", Request.URL.c_str());

                    const ServerFile FileMetadata = Filesystem::getFileMetadata(
                        Request.URL
                    );

                    if (FileMetadata.ContentPointer != nullptr) {
                        ClientObj->MsgAddDateHeader();
                        ClientObj->MsgAddHeader("Content-Type", FileMetadata.ContentType);
                        ClientObj->MsgAddHeader("Connection", "keep-alive");
                        ClientObj->MsgAddHeader("Server", "falcon-as");
                        ClientObj->MsgSetBodyRef(FileMetadata.ContentPointer, FileMetadata.ContentLength);
                        ClientObj->MsgGenerate();
                        ClientObj->MsgSetSendStatus(SEND_STATE_SENDING);
                    }
                    else {
                        const ServerFile File404 = Filesystem::getFileMetadata("/404.html");

                        ClientObj->MsgAddDateHeader();
                        ClientObj->MsgSetStatus(404, "Not Found");
                        ClientObj->MsgAddHeader("Content-Type", "text/html");
                        ClientObj->MsgAddHeader("Connection", "keep-alive");
                        ClientObj->MsgAddHeader("Server", "falcon-as");
                        ClientObj->MsgSetBodyRef(File404.ContentPointer, File404.ContentLength);
                        ClientObj->MsgGenerate();
                        ClientObj->MsgSetSendStatus(SEND_STATE_SENDING);
                    }
                }
            }
            else if (ClientObj->MsgGetSendStatus() == SEND_STATE_SENDING) {

                const SendMetadata_t SendMetadata = ClientObj->MsgGetSendMetadata();
                const ssize_t BytesWritten = write(ClientObj->getClientFD(), SendMetadata.BufferRef, SendMetadata.BufferSize);

                //ESP_LOGI("HTTPServer", "Bytes written:%d Buffer size:%d BufferAddress:0%x", BytesWritten, SendMetadata.BufferSize, SendMetadata.BufferRef);
                //ESP_LOG_BUFFER_HEX_LEVEL("HTTPServer", SendMetadata.BufferRef, SendMetadata.BufferSize, ESP_LOG_INFO);

                //- on error, remove and close connection
                if (BytesWritten < 0) {
                    const int WriteErrno = errno;
                    if (WriteErrno != EAGAIN && WriteErrno != EWOULDBLOCK && WriteErrno != EINTR) {
                        EraseFDs.push_back(ReadFD);
                    }
                }
                else if (ClientObj->MsgUpdateSendMetadata(BytesWritten) == true) {
                    //ESP_LOGI("HTTPServer", "Request finished, erasing");
                    ClientObj->MsgReset();
                    Requests->erase(Requests->begin());
                }
            }
        }
    }

    for (auto &EraseFD : EraseFDs)
    {
        //ESP_LOGI("HTTPServer", "Erase fd from clients::%d", EraseFD);
        Clients.erase(EraseFD);
        close(EraseFD);
    }

    return sumMessages;
}
