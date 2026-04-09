#include "CWWebServer.h"

bool ClockwiseWebServer::applySettingsObject(
  JsonObjectConst data,
  const char *&error,
  ClockwiseParams::ApplySettingsReport &report)
{
  return ClockwiseParams::getInstance()->applySettingsJson(data, error, &report);
}

void ClockwiseWebServer::handleApiSettings()
{
  if (!server.hasArg("plain"))
  {
    sendErrorJson("missing request body");
    return;
  }

  StaticJsonDocument<1536> doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err)
  {
    sendErrorJson("invalid json");
    return;
  }

  JsonObjectConst data = doc["settings"].as<JsonObjectConst>();
  if (data.isNull())
  {
    sendErrorJson("missing settings object");
    return;
  }

  ClockwiseParams::ApplySettingsReport applyReport;
  const char *error = nullptr;
  if (!applySettingsObject(data, error, applyReport))
  {
    sendErrorJson(error);
    return;
  }

  StaticJsonDocument<192> response;
  response["ok"] = true;
  response["changed"] = applyReport.anyChanged;
  response["restartRequired"] = applyReport.restartRequired;
  response["applyMode"] = applyReport.restartRequired
    ? "restart"
    : (applyReport.hotApplied ? "hot" : "none");
  sendJsonResponse(response);
}
