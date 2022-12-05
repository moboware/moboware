#include "applications/application.h"
#include "common/log_stream.h"
#include <fstream>
#include <json/json.h>

using namespace moboware::common;
using namespace moboware::applications;

const std::string CHANNELS_VALUE{ "Channels" };

Application::Application(const std::shared_ptr<common::Service>& service, //
                         const std::vector<std::shared_ptr<common::ChannelBase>>& channels)
  : common::ApplicationBase(service)
  , m_Channels(channels)
{
}

bool Application::LoadConfig(const std::string& configFile, const std::filesystem::path& applicationName)
{
  std::ifstream configStream(configFile.c_str());
  if (not configStream.is_open()) {
    LOG_DEBUG("Failed to open file:" << configFile);
    return false;
  }

  Json::CharReaderBuilder builder{};
  Json::Value rootDocument{};
  auto collectComments{ false };
  Json::String* errors{};
  if (not Json::parseFromStream(builder, configStream, &rootDocument, errors)) {
    LOG_DEBUG("Failed to parse config file");
    return false;
  }
  // read logging settings
  if (rootDocument.isMember("Logging")) {
    const auto loggingNode = rootDocument["Logging"];
    if (loggingNode.isMember("LogDirectory")) {
      const auto logDirectory{ loggingNode["LogDirectory"].asString() };
      std::filesystem::path logFile{ logDirectory };
      logFile += applicationName;
      logFile += ".log";
      LogStream::GetInstance().SetLogFile(logFile);
    }

    const auto logLevel{ loggingNode["LogLevel"].asString() };
    LogStream::GetInstance().SetLevel(LogStream::GetInstance().GetLevel(logLevel));
  }
  // read channel settings
  if (not rootDocument.isMember(CHANNELS_VALUE)) {
    LOG_DEBUG("Channel item not found");
    return false;
  }

  const Json::Value channelsValue = rootDocument[CHANNELS_VALUE];
  if (not channelsValue.isArray()) {
    LOG_DEBUG("Channels value is not an array");
    return false;
  }

  if (channelsValue.size() != m_Channels.size()) {
    LOG_DEBUG("Channel config and channels not same size");
    return false;
  }

  for (Json::ArrayIndex i = 0; i < channelsValue.size(); i++) {
    const auto channelValue = channelsValue[i];
    const auto channel = m_Channels[i];
    if (!channel->LoadConfig(channelValue)) {
      return false;
    }
  }

  return true;
}

int Application::Run(const int argc, const char* argv[])
{
  // read commandline parameters
  if (!ReadCommandline(argc, argv)) {
    return EXIT_FAILURE;
  }

  // load application and channel config
  const std::string configFile = "./config.json";
  const std::filesystem::path applicationPath = argv[0];

  if (not LoadConfig(configFile, applicationPath.stem())) {
    LOG_DEBUG("Failed to load config");
    return EXIT_FAILURE;
  }

  for (const auto channel : m_Channels) {
    if (!channel->Start()) {
      LOG_DEBUG("Failed to start channel");

      return EXIT_FAILURE;
    }
  }

  GetService()->Run();
  return EXIT_SUCCESS;
}

bool Application::ReadCommandline(const int argc, const char* argv[])
{
  return true;
}

void Application::Stop()
{
  //????m_Channels->Stop();
  GetService()->Stop();
}
