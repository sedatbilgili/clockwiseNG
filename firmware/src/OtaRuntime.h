#pragma once

#include <cstddef>
#include <cstdint>

namespace OtaRuntime
{
enum class Source : unsigned char
{
  Arduino = 0,
  HttpUrl,
  BrowserUpload
};

void setupArduinoOta();
void handleArduinoOta();
bool isInProgress();
void handleInProgressLoop();
bool startHttpOta(const char *url);
bool beginFirmwareStream(size_t expectedSize);
bool writeFirmwareStreamChunk(uint8_t *data, size_t size);
bool finalizeFirmwareStream();
void abortFirmwareStream();
void beginSession(Source source);
void updateProgress(unsigned int progress, unsigned int total);
void failSession();
void completeSession(bool requestRestart);
void queueRestart();
bool consumeRestartRequest();
}
