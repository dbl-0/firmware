#include "configuration.h"

#if ARCH_PORTDUINO

#include "VirtualSensor.h"
#include "PortduinoGlue.h"
#include <chrono>
#include <sys/stat.h>
#include <yaml-cpp/yaml.h>

VirtualSensor::VirtualSensor() : TelemetrySensor(meshtastic_TelemetrySensorType_SENSOR_UNSET, "VirtualSensor") {}

bool VirtualSensor::initDevice(TwoWire *bus, ScanI2C::FoundDevice *dev)
{
    filePath = portduino_config.virtualSensor_path;
    if (filePath.empty()) {
        LOG_INFO("VirtualSensor: no path configured, disabling");
        return false;
    }
    LOG_INFO("VirtualSensor: initialized with path %s", filePath.c_str());
    initialized = true;
    status = 1;
    return true;
}

bool VirtualSensor::getMetrics(meshtastic_Telemetry *measurement)
{
    // Check file exists
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        LOG_WARN("VirtualSensor: file not found: %s", filePath.c_str());
        return false;
    }

    // Check staleness
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    int age = static_cast<int>(now - fileStat.st_mtime);
    if (age > maxStalenessSeconds) {
        LOG_WARN("VirtualSensor: file is %d seconds old (max %d), skipping", age, maxStalenessSeconds);
        return false;
    }

    // Parse YAML
    YAML::Node data;
    try {
        data = YAML::LoadFile(filePath);
    } catch (const YAML::Exception &e) {
        LOG_WARN("VirtualSensor: YAML parse error: %s", e.what());
        return false;
    }

    bool hasAny = false;

    if (data["temperature"]) {
        measurement->variant.environment_metrics.temperature = data["temperature"].as<float>();
        measurement->variant.environment_metrics.has_temperature = true;
        hasAny = true;
    }
    if (data["relative_humidity"]) {
        measurement->variant.environment_metrics.relative_humidity = data["relative_humidity"].as<float>();
        measurement->variant.environment_metrics.has_relative_humidity = true;
        hasAny = true;
    }
    if (data["barometric_pressure"]) {
        measurement->variant.environment_metrics.barometric_pressure = data["barometric_pressure"].as<float>();
        measurement->variant.environment_metrics.has_barometric_pressure = true;
        hasAny = true;
    }
    if (data["wind_speed"]) {
        measurement->variant.environment_metrics.wind_speed = data["wind_speed"].as<float>();
        measurement->variant.environment_metrics.has_wind_speed = true;
        hasAny = true;
    }
    if (data["wind_direction"]) {
        measurement->variant.environment_metrics.wind_direction = data["wind_direction"].as<int>();
        measurement->variant.environment_metrics.has_wind_direction = true;
        hasAny = true;
    }

    if (hasAny) {
        LOG_DEBUG("VirtualSensor: read metrics from %s (age %ds)", filePath.c_str(), age);
    }

    return hasAny;
}

#endif
