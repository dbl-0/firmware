#pragma once

#if ARCH_PORTDUINO

#include "TelemetrySensor.h"
#include <string>

class VirtualSensor : public TelemetrySensor
{
  public:
    VirtualSensor();
    bool initDevice(TwoWire *bus, ScanI2C::FoundDevice *dev) override;
    bool getMetrics(meshtastic_Telemetry *measurement) override;

  private:
    std::string filePath;
    int maxStalenessSeconds = 1800; // 30 min default
};

#endif
