#include "asus_tuf_fx705ge.h"

namespace fs = std::filesystem;

std::optional<int> AsusTufFx705ge::readFanSpeedRpm() const
{
    const std::vector<std::string> preferredNames = {"asus", "asus-isa", "asus_wmi", "asusec"};
    auto candidateHwmons = hwmonDirectoriesMatching(preferredNames);
    if (candidateHwmons.empty()) {
        candidateHwmons = hwmonDirectories();
    }

    for (const auto &dir : candidateHwmons) {
        const auto speeds = readFanInputs(dir);
        if (!speeds.empty()) {
            return *std::max_element(speeds.begin(), speeds.end());
        }
    }

    return std::nullopt;
}

std::optional<double> AsusTufFx705ge::readAverageTemperatureCelsius() const
{
    const std::vector<std::string> preferredNames = {"coretemp", "asus", "asus-isa", "acpitz"};
    auto candidateHwmons = hwmonDirectoriesMatching(preferredNames);
    if (candidateHwmons.empty()) {
        candidateHwmons = hwmonDirectories();
    }

    for (const auto &dir : candidateHwmons) {
        const auto tempsMilli = readTemperatureInputs(dir);
        if (!tempsMilli.empty()) {
            const auto sum = std::accumulate(tempsMilli.begin(), tempsMilli.end(), 0L);
            const double averageMilli = static_cast<double>(sum) / static_cast<double>(tempsMilli.size());
            return averageMilli / 1000.0;
        }
    }

    const auto thermalTemps = readThermalZoneTemperatures();
    if (!thermalTemps.empty()) {
        const auto sum = std::accumulate(thermalTemps.begin(), thermalTemps.end(), 0L);
        const double averageMilli = static_cast<double>(sum) / static_cast<double>(thermalTemps.size());
        return averageMilli / 1000.0;
    }

    return std::nullopt;
}

std::vector<fs::path> AsusTufFx705ge::hwmonDirectories()
{
    std::vector<fs::path> result;
    const fs::path hwmonRoot("/sys/class/hwmon");
    if (!fs::exists(hwmonRoot) || !fs::is_directory(hwmonRoot)) {
        return result;
    }

    for (const auto &entry : fs::directory_iterator(hwmonRoot)) {
        if (entry.is_directory()) {
            result.push_back(entry.path());
        }
    }

    return result;
}

std::vector<fs::path> AsusTufFx705ge::hwmonDirectoriesMatching(const std::vector<std::string> &names)
{
    std::vector<fs::path> result;
    for (const auto &dir : hwmonDirectories()) {
        const auto nameFile = dir / "name";
        auto nameValue = readStringFromFile(nameFile);
        if (!nameValue) {
            continue;
        }

        std::string normalized = *nameValue;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        for (const auto &expected : names) {
            std::string expectedNormalized = expected;
            std::transform(expectedNormalized.begin(), expectedNormalized.end(), expectedNormalized.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (normalized == expectedNormalized) {
                result.push_back(dir);
                break;
            }
        }
    }

    return result;
}

std::vector<int> AsusTufFx705ge::readFanInputs(const fs::path &hwmonDir)
{
    std::vector<int> speeds;
    if (!fs::exists(hwmonDir) || !fs::is_directory(hwmonDir)) {
        return speeds;
    }

    for (const auto &entry : fs::directory_iterator(hwmonDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto filename = entry.path().filename().string();
        if (!startsWith(filename, "fan") || !endsWith(filename, "_input")) {
            continue;
        }

        auto value = readLongFromFile(entry.path());
        if (value && *value > 0) {
            speeds.push_back(static_cast<int>(*value));
        }
    }

    return speeds;
}

std::vector<long> AsusTufFx705ge::readTemperatureInputs(const fs::path &hwmonDir)
{
    std::vector<long> temps;
    if (!fs::exists(hwmonDir) || !fs::is_directory(hwmonDir)) {
        return temps;
    }

    for (const auto &entry : fs::directory_iterator(hwmonDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto filename = entry.path().filename().string();
        if (!startsWith(filename, "temp") || !endsWith(filename, "_input")) {
            continue;
        }

        auto value = readLongFromFile(entry.path());
        if (value && *value > 0) {
            temps.push_back(*value);
        }
    }

    return temps;
}

std::vector<long> AsusTufFx705ge::readThermalZoneTemperatures()
{
    std::vector<long> temps;
    const fs::path thermalRoot("/sys/class/thermal");
    if (!fs::exists(thermalRoot) || !fs::is_directory(thermalRoot)) {
        return temps;
    }

    for (const auto &entry : fs::directory_iterator(thermalRoot)) {
        if (!entry.is_directory() || !startsWith(entry.path().filename().string(), "thermal_zone")) {
            continue;
        }

        const auto tempFile = entry.path() / "temp";
        auto value = readLongFromFile(tempFile);
        if (value && *value > 0) {
            temps.push_back(*value);
        }
    }

    return temps;
}

std::optional<long> AsusTufFx705ge::readLongFromFile(const fs::path &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return std::nullopt;
    }

    long value = 0;
    file >> value;
    if (!file.good()) {
        return std::nullopt;
    }

    return value;
}

std::optional<std::string> AsusTufFx705ge::readStringFromFile(const fs::path &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return std::nullopt;
    }

    std::string line;
    if (!std::getline(file, line)) {
        return std::nullopt;
    }

    while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back()))) {
        line.pop_back();
    }

    while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front()))) {
        line.erase(line.begin());
    }

    return line;
}

bool AsusTufFx705ge::startsWith(const std::string &value, const std::string &prefix)
{
    return value.size() >= prefix.size()
        && std::equal(prefix.begin(), prefix.end(), value.begin());
}

bool AsusTufFx705ge::endsWith(const std::string &value, const std::string &suffix)
{
    return value.size() >= suffix.size()
        && std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
}
