#include "DNSServer.hpp"
#include "Network.hpp"
#include "Helper.hpp"

#include "esp_log.h"

using namespace std;


DNSServer::DNSServer()
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

            const unsigned char QTypeByte1 = RecvBuffer[ReceivedBytes-4];
            const unsigned char QTypeByte2 = RecvBuffer[ReceivedBytes-3];
            const unsigned char QTypeByte3 = RecvBuffer[ReceivedBytes-2];
            const unsigned char QTypeByte4 = RecvBuffer[ReceivedBytes-1];
            const bool IsPongGameQuery =
                ReceivedBytes == DNSQuestionOffset + PongGameQueryNameLength + 4 &&
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

            ESP_LOGI("DNSServer", "QueryType b1:%d b2:%d b3:%d b4:%d", QTypeByte1, QTypeByte2, QTypeByte3, QTypeByte4);

            //- answer QCLASS=IN, QTYPE=A queries for pong.game only
            if (IsPongGameQuery &&
                QTypeByte1 == 0x00 && QTypeByte2 == 0x01 &&
                QTypeByte3 == 0x00 && QTypeByte4 == 0x01) {

                //- construct answer packets
                RecvBuffer[2] = 0x81;  //- response type byte1
                RecvBuffer[3] = 0x80;  //- response type byte2

                RecvBuffer[7] = 0x01;  //- 1 answer RRs byte2

                RecvBuffer[8] = 0x00;  //- 0 authority RRs byte1
                RecvBuffer[9] = 0x00;  //- 0 authority RRs byte2

                RecvBuffer[10] = 0x00; //- 0 additional RRs byte1
                RecvBuffer[11] = 0x00; //- 0 additional RRs byte2

                RecvBuffer[ReceivedBytes] = 0xc0;    //- compressed answer byte1
                RecvBuffer[ReceivedBytes+1] = 0x0c;  //- compressed answer byte2

                RecvBuffer[ReceivedBytes+2] = 0x00;  //- QCLASS=IN, QTYPE=A byte1
                RecvBuffer[ReceivedBytes+3] = 0x01;  //- QCLASS=IN, QTYPE=A byte2
                RecvBuffer[ReceivedBytes+4] = 0x00;  //- QCLASS=IN, QTYPE=A byte3
                RecvBuffer[ReceivedBytes+5] = 0x01;  //- QCLASS=IN, QTYPE=A byte4

                RecvBuffer[ReceivedBytes+6] = 0x00;  //- ttl byte1 (3600 sec)
                RecvBuffer[ReceivedBytes+7] = 0x00;  //- ttl byte2 (3600 sec)
                RecvBuffer[ReceivedBytes+8] = 0x0e;  //- ttl byte3 (3600 sec)
                RecvBuffer[ReceivedBytes+9] = 0x10;  //- ttl byte4 (3600 sec)

                RecvBuffer[ReceivedBytes+10] = 0x00; //- RD length (IPv4) byte1
                RecvBuffer[ReceivedBytes+11] = 0x04; //- RD length (IPv4) byte2

                RecvBuffer[ReceivedBytes+12] = stoi(IPAddressSplit[0]); //- IPv4 byte1
                RecvBuffer[ReceivedBytes+13] = stoi(IPAddressSplit[1]); //- IPv4 byte2
                RecvBuffer[ReceivedBytes+14] = stoi(IPAddressSplit[2]); //- IPv4 byte3
                RecvBuffer[ReceivedBytes+15] = stoi(IPAddressSplit[3]); //- IPv4 byte4

                ESP_LOG_BUFFER_CHAR_LEVEL("DNSServer", &RecvBuffer[0], ReceivedBytes+16, ESP_LOG_INFO);

                sendto(ServerSocketFD, &RecvBuffer[0], ReceivedBytes+16, 0, (const struct sockaddr*) &ClientSocketAddr, ClientSocketLen);
            }
        }
        else {
            vTaskDelay(10);
        }
    }
}
