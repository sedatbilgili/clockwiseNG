#include "CWDateTime.h"

namespace {
const char *resolveBuiltInPosix(const char *timeZone)
{
  if (timeZone == nullptr)
  {
    return nullptr;
  }

  if (strcmp(timeZone, "Asia/Istanbul") == 0)
  {
    return "<+03>-3";
  }

  if (strcmp(timeZone, "Etc/UTC") == 0 || strcmp(timeZone, "UTC") == 0)
  {
    return "UTC0";
  }

  return nullptr;
}
}

void CWDateTime::begin(const char *timeZone, bool use24format, const char *ntpServer = NTP_SERVER, const char *posixTZ = "")
{
  Serial.printf("[Time] NTP Server: %s, Timezone: %s\n", ntpServer, timeZone);
  ezt::setServer(String(ntpServer));

  const char *resolvedPosix = nullptr;
  if (strlen(posixTZ) > 1)
  {
    resolvedPosix = posixTZ;
  }
  else
  {
    resolvedPosix = resolveBuiltInPosix(timeZone);
  }

  if (resolvedPosix != nullptr) {
    // An empty value still contains a null character so not empty is a value greater than 1.
    // Set to defined Posix TZ
    Serial.printf("[Time] Using POSIX TZ: %s\n", resolvedPosix);
    myTZ.setPosix(resolvedPosix);
  } else {
    // Use automatic eztime remote lookup
    Serial.printf("[Time] Using remote timezone lookup for %s\n", timeZone);
    myTZ.setLocation(timeZone);
  }

  this->use24hFormat = use24format;
  ezt::updateNTP();
  waitForSync(10);
}

String CWDateTime::getFormattedTime()
{
  return myTZ.dateTime();
}

String CWDateTime::getFormattedTime(const char *format)
{
  return myTZ.dateTime(format);
}

char *CWDateTime::getHour(const char *format)
{
  static char buffer[3] = {'\0'};
  strncpy(buffer, myTZ.dateTime((use24hFormat ? "H" : "h")).c_str(), sizeof(buffer));
  return buffer;
}

char *CWDateTime::getMinute(const char *format)
{
  static char buffer[3] = {'\0'};
  strncpy(buffer, myTZ.dateTime("i").c_str(), sizeof(buffer));
  return buffer;
}

int CWDateTime::getHour()
{
  return myTZ.dateTime((use24hFormat ? "H" : "h")).toInt();
}

int CWDateTime::getMinute()
{
  return myTZ.dateTime("i").toInt();
}

int CWDateTime::getSecond()
{
  return myTZ.dateTime("s").toInt();
}

int CWDateTime::getDay() 
{
  return myTZ.dateTime("d").toInt();
}
int CWDateTime::getMonth()
{
  return myTZ.dateTime("m").toInt();
}
int CWDateTime::getWeekday() 
{
  return myTZ.dateTime("w").toInt()-1;
}

long CWDateTime::getMilliseconds() 
{
  return myTZ.ms(TIME_NOW);
}

bool CWDateTime::isAM() 
{
  return myTZ.isAM();
}

bool CWDateTime::is24hFormat() 
{
  return this->use24hFormat;
}
