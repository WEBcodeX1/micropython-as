#include "ASRequestGlobal.hpp"
#include "ASRequestHandler.hpp"
#include "ASRequestDef.hpp"

#include "esp_log.h"

using namespace std;


ASRequestHandler::ASRequestHandler()
{
}

ASRequestHandler::~ASRequestHandler()
{
}


void ASRequestHandler::ASRequestInit(RequestProperties_t Request)
{
    for(const ASRequestDefinition_t &ASRequestDef: ASRequestDefinitions) {
        if (ASRequestDef.URL == Request.URL && ASRequestDef.HTTPMethod == Request.HTTPMethod) {
            const auto PayloadLength = Request.Payload.length();
            ESP_LOGI("ASRequest", "Request URL:%s Method:%d ID:%d PayloadLength:%d", Request.URL.c_str(), Request.HTTPMethod, ASRequestDef.ID, PayloadLength);
            ASRequestID = ASRequestDef.ID;
            ASRequestContentLength = PayloadLength;
            Request.Payload.copy(ASRequestExchangeBuffer, PayloadLength);
            ASRequestStatus = AS_REQ_PROCESSING;
            break;
        }
    }
}
