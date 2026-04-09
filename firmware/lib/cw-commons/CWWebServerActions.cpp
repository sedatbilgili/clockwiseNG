#include "CWWebServer.h"

#include <Update.h>
#include <cstdlib>
#include <cstring>
#include "../../src/OtaRuntime.h"

namespace
{
bool copyHttpOtaUrl(char *dest, size_t destSize, const char *url, const char *&error)
{
  if (url == nullptr || url[0] == '\0')
  {
    error = "missing url";
    return false;
  }

  if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0)
  {
    error = "url must start with http:// or https://";
    return false;
  }

  const size_t urlLength = strlen(url);
  if (urlLength >= destSize)
  {
    error = "url too long";
    return false;
  }

  strncpy(dest, url, destSize - 1);
  dest[destSize - 1] = '\0';
  error = nullptr;
  return true;
}
}

void ClockwiseWebServer::queueHttpOta()
{
  if (!server.hasArg("url"))
  {
    sendErrorJson("missing url");
    return;
  }

  const String urlArg = server.arg("url");
  const char *error = nullptr;
  if (!copyHttpOtaUrl(pendingHttpOtaUrl, sizeof(pendingHttpOtaUrl), urlArg.c_str(), error))
  {
    sendErrorJson(error);
    return;
  }

  pendingHttpOtaQueued = true;
#if CW_DEBUG_WEB_FREEZE
  Serial.printf("[WEB] queued HTTP OTA URL: %s\n", pendingHttpOtaUrl);
#endif
  server.send(202, "application/json", "{\"ok\":true}");
}

void ClockwiseWebServer::handleOtaUploadData()
{
  HTTPUpload &upload = server.upload();

  switch (upload.status)
  {
    case UPLOAD_FILE_START:
    {
      firmwareUploadInProgress = true;
      firmwareUploadSucceeded = false;
      firmwareUploadProgress = 0;
      firmwareUploadBytesReceived = 0;
      firmwareUploadExpectedBytes = 0;
      firmwareUploadError[0] = '\0';
      OtaRuntime::beginSession(OtaRuntime::Source::BrowserUpload);

      if (server.hasArg("size"))
      {
        firmwareUploadExpectedBytes = static_cast<size_t>(strtoul(server.arg("size").c_str(), nullptr, 10));
      }
      else if (server.hasHeader("Content-Length"))
      {
        firmwareUploadExpectedBytes = static_cast<size_t>(strtoul(server.header("Content-Length").c_str(), nullptr, 10));
      }

      if (!OtaRuntime::beginFirmwareStream(firmwareUploadExpectedBytes))
      {
        snprintf(firmwareUploadError, sizeof(firmwareUploadError), "Update.begin failed (%u)", Update.getError());
        OtaRuntime::failSession();
      }
      break;
    }

    case UPLOAD_FILE_WRITE:
    {
      if (firmwareUploadError[0] != '\0')
      {
        break;
      }

      if (!OtaRuntime::writeFirmwareStreamChunk(upload.buf, upload.currentSize))
      {
        snprintf(firmwareUploadError, sizeof(firmwareUploadError), "Update.write failed (%u)", Update.getError());
        OtaRuntime::abortFirmwareStream();
        OtaRuntime::failSession();
        break;
      }

      firmwareUploadBytesReceived += upload.currentSize;
      const unsigned int totalBytes = firmwareUploadExpectedBytes > 0
        ? static_cast<unsigned int>(firmwareUploadExpectedBytes)
        : 0U;
      firmwareUploadProgress = totalBytes > 0
        ? min<uint8_t>(99, static_cast<uint8_t>((firmwareUploadBytesReceived * 100ULL) / firmwareUploadExpectedBytes))
        : min<uint8_t>(99, firmwareUploadProgress + 1);
      break;
    }

    case UPLOAD_FILE_END:
    {
      firmwareUploadInProgress = false;
      if (firmwareUploadError[0] != '\0')
      {
        break;
      }

      if (!OtaRuntime::finalizeFirmwareStream())
      {
        snprintf(firmwareUploadError, sizeof(firmwareUploadError), "Update.end failed (%u)", Update.getError());
        OtaRuntime::failSession();
        break;
      }

      firmwareUploadSucceeded = true;
      firmwareUploadProgress = 100;
      firmwareUploadBytesReceived = 0;
      firmwareUploadExpectedBytes = 0;
      OtaRuntime::completeSession(false);
      break;
    }

    case UPLOAD_FILE_ABORTED:
    {
      firmwareUploadInProgress = false;
      firmwareUploadSucceeded = false;
      firmwareUploadBytesReceived = 0;
      firmwareUploadExpectedBytes = 0;
      snprintf(firmwareUploadError, sizeof(firmwareUploadError), "upload aborted");
      OtaRuntime::abortFirmwareStream();
      OtaRuntime::failSession();
      break;
    }

    default:
      break;
  }
}

void ClockwiseWebServer::handleOtaUploadRequest()
{
  if (firmwareUploadInProgress)
  {
    sendErrorJson("upload still in progress", 409);
    return;
  }

  if (firmwareUploadError[0] != '\0')
  {
    sendErrorJson(firmwareUploadError, 500);
    firmwareUploadError[0] = '\0';
    firmwareUploadSucceeded = false;
    firmwareUploadProgress = 0;
    firmwareUploadBytesReceived = 0;
    firmwareUploadExpectedBytes = 0;
    return;
  }

  if (!firmwareUploadSucceeded)
  {
    sendErrorJson("no firmware uploaded");
    return;
  }

  StaticJsonDocument<128> response;
  response["ok"] = true;
  response["uploaded"] = true;
  response["restarting"] = true;
  sendJsonResponse(response);

  firmwareUploadSucceeded = false;
  firmwareUploadProgress = 0;
  firmwareUploadBytesReceived = 0;
  firmwareUploadExpectedBytes = 0;
  OtaRuntime::queueRestart();
}

void ClockwiseWebServer::handleApiActions()
{
  if (!server.hasArg("plain"))
  {
    sendErrorJson("missing request body");
    return;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err)
  {
    sendErrorJson("invalid json");
    return;
  }

  const char *action = doc["action"] | "";
  if (strcmp(action, "restart") == 0)
  {
    restartDevice();
    return;
  }

  if (strcmp(action, "httpOta") == 0)
  {
    const char *error = nullptr;
    bool queued = false;

    const char *jsonUrl = doc["url"] | nullptr;
    if (jsonUrl != nullptr && jsonUrl[0] != '\0')
    {
      queued = copyHttpOtaUrl(pendingHttpOtaUrl, sizeof(pendingHttpOtaUrl), jsonUrl, error);
    }

    if (!queued && server.hasArg("url"))
    {
      const String queryUrl = server.arg("url");
      queued = copyHttpOtaUrl(pendingHttpOtaUrl, sizeof(pendingHttpOtaUrl), queryUrl.c_str(), error);
    }

    if (!queued)
    {
      sendErrorJson(error != nullptr ? error : "missing url");
      return;
    }

    pendingHttpOtaQueued = true;
    StaticJsonDocument<96> response;
    response["ok"] = true;
    response["queued"] = true;
    sendJsonResponse(response, 202);
    return;
  }

  if (strcmp(action, "readPin") == 0)
  {
    if (!doc.containsKey("pin"))
    {
      sendErrorJson("missing pin");
      return;
    }

    StaticJsonDocument<128> response;
    response["ok"] = true;
    response["pin"] = doc["pin"].as<int>();
    response["value"] = analogRead(doc["pin"].as<int>());
    sendJsonResponse(response);
    return;
  }

  sendErrorJson("unknown action");
}
