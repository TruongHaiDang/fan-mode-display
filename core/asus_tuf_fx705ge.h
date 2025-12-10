#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <string>
#include <vector>

// Helper to read fan speed and average temperature on the ASUS TUF FX705GE.
// The implementation searches hwmon entries dynamically, so it works even if
// the hwmon index changes between boots.
class AsusTufFx705ge
{
public:
    std::optional<int> readFanSpeedRpm() const;
    std::optional<double> readAverageTemperatureCelsius() const;
    std::vector<int> readAllFanSpeedsRpm() const;
    std::vector<double> readAllTemperaturesCelsius() const;
    std::string readFanBoostModeLabel() const;

private:
    static std::vector<std::filesystem::path> hwmonDirectories();
    static std::vector<std::filesystem::path> hwmonDirectoriesMatching(const std::vector<std::string> &names);
    static std::vector<int> readFanInputs(const std::filesystem::path &hwmonDir);
    static std::vector<long> readTemperatureInputs(const std::filesystem::path &hwmonDir);
    static std::vector<long> readThermalZoneTemperatures();
    static std::optional<long> readLongFromFile(const std::filesystem::path &filePath);
    static std::optional<std::string> readStringFromFile(const std::filesystem::path &filePath);
    static bool startsWith(const std::string &value, const std::string &prefix);
    static bool endsWith(const std::string &value, const std::string &suffix);
};
