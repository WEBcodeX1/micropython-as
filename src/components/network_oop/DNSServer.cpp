#include "DNSServer.hpp"
#include "Network.hpp"
#include "Helper.hpp"

#include "esp_log.h"

#include <cstring>

using namespace std;


DNSServer::DNSServer(uint16_t ListenPort)
    : SocketListenPort(ListenPort)
{
}

DNSServer::~DNSServer()
{
}

void DNSServer::start()
{
    // setup socket (filedescriptor)
    ServerSocketFD = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&ServerSocketAddr, 0, sizeof(ServerSocketAddr));
    memset(&ClientSocketAddr, 0, sizeof(ClientSocketAddr));

    ServerSocketAddr.sin_family = AF_INET;
    ServerSocketAddr.sin_addr.s_addr = INADDR_ANY;
    ServerSocketAddr.sin_port = htons(SocketListenPort);

    // bind socket
    bind(ServerSocketFD, (const struct sockaddr*)&ServerSocketAddr, sizeof(ServerSocketAddr));

    ClientSocketLen = sizeof(ClientSocketAddr);

    //- start server loop
    ServerLoop();
}

void DNSServer::ServerLoop()
{
    string IPAddress = Network::getIPAddr();

    vector<string> IPAddressSplit;
    StringHelper::split(IPAddress, ".", IPAddressSplit);

    while(1)
    {
        const int ReceivedBytes = recvfrom(ServerSocketFD, &RecvBuffer[0], 128, MSG_WAITALL, (struct sockaddr*)&ClientSocketAddr, &ClientSocketLen);

        if (ReceivedBytes >= 24 && ReceivedBytes < 100)
        {
            //ESP_LOG_BUFFER_HEX_LEVEL("DNSServer", &RecvBuffer[0], ReceivedBytes, ESP_LOG_INFO);
            //ESP_LOG_BUFFER_CHAR_LEVEL("DNSServer", &RecvBuffer[12], ReceivedBytes-4, ESP_LOG_INFO);

            const bool HasPongGameQueryName =
                ReceivedBytes >= DNSQuestionTypeOffset + 4 &&
                RecvBuffer[12] == PongGameQueryName[0] &&
                RecvBuffer[13] == PongGameQueryName[1] &&
                RecvBuffer[14] == PongGameQueryName[2] &&
                RecvBuffer[15] == PongGameQueryName[3] &&
                RecvBuffer[16] == PongGameQueryName[4] &&
                RecvBuffer[17] == PongGameQueryName[5] &&
                RecvBuffer[18] == PongGameQueryName[6] &&
                RecvBuffer[19] == PongGameQueryName[7] &&
                RecvBuffer[20] == PongGameQueryName[8] &&
                RecvBuffer[21] == PongGameQueryName[9] &&
                RecvBuffer[22] == PongGameQueryName[10];
            const bool IsAInQuery =
                RecvBuffer[DNSQuestionTypeOffset] == 0x00 &&
                RecvBuffer[DNSQuestionTypeOffset+1] == 0x01 &&
                RecvBuffer[DNSQuestionTypeOffset+2] == 0x00 &&
                RecvBuffer[DNSQuestionTypeOffset+3] == 0x01;
            const bool IsStandardSingleQuestionQuery =
                RecvBuffer[4] == 0x00 && RecvBuffer[5] == 0x01 &&
                RecvBuffer[6] == 0x00 && RecvBuffer[7] == 0x00 &&
                RecvBuffer[8] == 0x00 && RecvBuffer[9] == 0x00;
            const bool IsEdns0OptQuery =
                ReceivedBytes == PongGameEdns0QueryLength &&
                RecvBuffer[PongGameQueryLength] == Edns0OptRecord[0] &&
                RecvBuffer[PongGameQueryLength+1] == Edns0OptRecord[1] &&
                RecvBuffer[PongGameQueryLength+2] == Edns0OptRecord[2] &&
                RecvBuffer[PongGameQueryLength+3] == Edns0OptRecord[3] &&
                RecvBuffer[PongGameQueryLength+4] == Edns0OptRecord[4] &&
                RecvBuffer[PongGameQueryLength+5] == Edns0OptRecord[5] &&
                RecvBuffer[PongGameQueryLength+6] == Edns0OptRecord[6] &&
                RecvBuffer[PongGameQueryLength+7] == Edns0OptRecord[7] &&
                RecvBuffer[PongGameQueryLength+8] == Edns0OptRecord[8] &&
                RecvBuffer[PongGameQueryLength+9] == Edns0OptRecord[9] &&
                RecvBuffer[PongGameQueryLength+10] == Edns0OptRecord[10];
            const bool IsPlainPongGameQuery =
                HasPongGameQueryName &&
                ReceivedBytes == PongGameQueryLength &&
                IsStandardSingleQuestionQuery &&
                RecvBuffer[10] == 0x00 && RecvBuffer[11] == 0x00 &&
                IsAInQuery;
            const bool IsEdns0PongGameQuery =
                HasPongGameQueryName &&
                IsStandardSingleQuestionQuery &&
                RecvBuffer[10] == 0x00 && RecvBuffer[11] == 0x01 &&
                IsAInQuery &&
                IsEdns0OptQuery;
            const bool IsSupportedPongGameQuery = IsPlainPongGameQuery || IsEdns0PongGameQuery;

            ESP_LOGI("DNSServer", "Plain:%d EDNS0:%d PongName:%d", IsPlainPongGameQuery, IsEdns0PongGameQuery, HasPongGameQueryName);

            //- answer QCLASS=IN, QTYPE=A queries for pong.game only
            if (IsSupportedPongGameQuery) {
                const int AnswerOffset = PongGameQueryLength;
                const int ResponseLength = ReceivedBytes + 16;

                //- construct answer packets
                RecvBuffer[2] = 0x81;  //- response type byte1
                RecvBuffer[3] = 0x80;  //- response type byte2

                RecvBuffer[7] = 0x01;  //- 1 answer RRs byte2

                RecvBuffer[8] = 0x00;  //- 0 authority RRs byte1
                RecvBuffer[9] = 0x00;  //- 0 authority RRs byte2

                RecvBuffer[10] = 0x00; //- additional RRs byte1
                RecvBuffer[11] = IsEdns0PongGameQuery ? 0x01 : 0x00; //- additional RRs byte2

                RecvBuffer[AnswerOffset] = 0xc0;    //- compressed answer byte1
                RecvBuffer[AnswerOffset+1] = 0x0c;  //- compressed answer byte2

                RecvBuffer[AnswerOffset+2] = 0x00;  //- QCLASS=IN, QTYPE=A byte1
                RecvBuffer[AnswerOffset+3] = 0x01;  //- QCLASS=IN, QTYPE=A byte2
                RecvBuffer[AnswerOffset+4] = 0x00;  //- QCLASS=IN, QTYPE=A byte3
                RecvBuffer[AnswerOffset+5] = 0x01;  //- QCLASS=IN, QTYPE=A byte4

                RecvBuffer[AnswerOffset+6] = 0x00;  //- ttl byte1 (3600 sec)
                RecvBuffer[AnswerOffset+7] = 0x00;  //- ttl byte2 (3600 sec)
                RecvBuffer[AnswerOffset+8] = 0x0e;  //- ttl byte3 (3600 sec)
                RecvBuffer[AnswerOffset+9] = 0x10;  //- ttl byte4 (3600 sec)

                RecvBuffer[AnswerOffset+10] = 0x00; //- RD length (IPv4) byte1
                RecvBuffer[AnswerOffset+11] = 0x04; //- RD length (IPv4) byte2

                RecvBuffer[AnswerOffset+12] = stoi(IPAddressSplit[0]); //- IPv4 byte1
                RecvBuffer[AnswerOffset+13] = stoi(IPAddressSplit[1]); //- IPv4 byte2
                RecvBuffer[AnswerOffset+14] = stoi(IPAddressSplit[2]); //- IPv4 byte3
                RecvBuffer[AnswerOffset+15] = stoi(IPAddressSplit[3]); //- IPv4 byte4

                if (IsEdns0PongGameQuery) {
                    memcpy(&RecvBuffer[AnswerOffset+16], Edns0OptRecord, Edns0OptRecordLength);
                }

                ESP_LOG_BUFFER_CHAR_LEVEL("DNSServer", &RecvBuffer[0], ResponseLength, ESP_LOG_INFO);

                sendto(ServerSocketFD, &RecvBuffer[0], ResponseLength, 0, (const struct sockaddr*) &ClientSocketAddr, ClientSocketLen);
            }
            else if (!HasPongGameQueryName) {
                RecvBuffer[2] = 0x81;  //- response type byte1
                RecvBuffer[3] = 0x83;  //- response type byte2, NXDOMAIN

                RecvBuffer[6] = 0x00;  //- 0 answer RRs byte1
                RecvBuffer[7] = 0x00;  //- 0 answer RRs byte2

                RecvBuffer[8] = 0x00;  //- 0 authority RRs byte1
                RecvBuffer[9] = 0x00;  //- 0 authority RRs byte2

                sendto(ServerSocketFD, &RecvBuffer[0], ReceivedBytes, 0, (const struct sockaddr*) &ClientSocketAddr, ClientSocketLen);
            }
        }
        else {
            vTaskDelay(10);
        }
    }
}
