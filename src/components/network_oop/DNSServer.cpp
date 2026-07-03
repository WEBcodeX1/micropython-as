#include "DNSServer.hpp"

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
    while(1)
    {
        const int ReceivedBytes = recvfrom(ServerSocketFD, &RecvBuffer[0], 128, MSG_WAITALL, (struct sockaddr*)&ClientSocketAddr, &ClientSocketLen);

        if (ReceivedBytes > 0) {

            ESP_LOG_BUFFER_HEX_LEVEL("DNSServer", &RecvBuffer[0], ReceivedBytes, ESP_LOG_INFO);
            ESP_LOG_BUFFER_CHAR_LEVEL("DNSServer", &RecvBuffer[12], ReceivedBytes-4, ESP_LOG_INFO);

            const unsigned char QTypeByte1 = RecvBuffer[ReceivedBytes-4];
            const unsigned char QTypeByte2 = RecvBuffer[ReceivedBytes-3];
            const unsigned char QTypeByte3 = RecvBuffer[ReceivedBytes-2];
            const unsigned char QTypeByte4 = RecvBuffer[ReceivedBytes-1];

            ESP_LOGI("DNSServer", "QueryType b1:%d b2:%d b3:%d b4:%d", QTypeByte1, QTypeByte2, QTypeByte3, QTypeByte4);

            //- answer QCLASS=IN, QTYPE=A queries only
            if (QTypeByte1 == 0x00 && QTypeByte2 == 0x01 && QTypeByte3 == 0x00 && QTypeByte4 == 0x01) {

                // respond to all A record queries with HTTP servers IP address

                //- construct answer packets
                RecvBuffer[2] = 0x81; //- response type byte1
                RecvBuffer[3] = 0x80; //- response type byte2

                RecvBuffer[7] = 0x01; //- 1 answer RRs byte2

                RecvBuffer[8] = 0x00; //- 0 authority RRs byte1
                RecvBuffer[9] = 0x00; //- 0 authority RRs byte2

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

                RecvBuffer[ReceivedBytes+12] = 0xc0; //- IPv4 byte1 (192)
                RecvBuffer[ReceivedBytes+13] = 0xa8; //- IPv4 byte2 (168)
                RecvBuffer[ReceivedBytes+14] = 0x0a; //- IPv4 byte3 (10)
                RecvBuffer[ReceivedBytes+15] = 0xfe; //- IPv4 byte4 (254)

                ESP_LOG_BUFFER_HEX_LEVEL("DNSServer", &RecvBuffer[0], ReceivedBytes+16, ESP_LOG_INFO);

                sendto(ServerSocketFD, &RecvBuffer[0], ReceivedBytes+16, 0, (const struct sockaddr*) &ClientSocketAddr, ClientSocketLen);
            }
        }
        else {
            vTaskDelay(10);
        }
    }
}
