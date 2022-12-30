#include "applications/application.h"
#include "common/log_stream.h"
#include <fstream>

using namespace moboware::common;
using namespace moboware::applications;
using namespace boost;

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

  json::stream_parser parser;
  system::error_code ec;

  while (not configStream.eof()) {
    std::string c;
    std::getline(configStream, c);

    if (0 == parser.write_some(c, ec)) {
      LOG_DEBUG("Failed to parse config file");
      return false;
    }
  }

  if (not parser.done()) {
    LOG_ERROR("Parser is not done yet!");
    return false;
  }

  const boost::json::value& rootDocument{ parser.release() };

  // read logging settings
  if (rootDocument.as_object().contains("Logging")) {
    const auto loggingNode = rootDocument.at("Logging");
    if (loggingNode.as_object().contains("LogDirectory")) {
      const auto logDirectory{ loggingNode.at("LogDirectory").as_string().c_str() };
      std::filesystem::path logFile{ logDirectory };
      logFile += applicationName;
      logFile += ".log";
      LogStream::GetInstance().SetLogFile(logFile);
    }

    const auto logLevel{ loggingNode.at("LogLevel").as_string().c_str() };
    LogStream::GetInstance().SetLevel(LogStream::GetInstance().GetLevel(logLevel));
  }

  // read channel settings
  if (not rootDocument.as_object().contains(CHANNELS_VALUE)) {
    LOG_DEBUG("Channel item not found");
    return false;
  }

  const auto& channelsValues = rootDocument.at(CHANNELS_VALUE);
  if (not channelsValues.is_array()) {
    LOG_DEBUG("Channels value is not an array");
    return false;
  }

  const auto& channelsValuesArray{ channelsValues.as_array() };
  if (channelsValuesArray.size() != m_Channels.size()) {
    LOG_DEBUG("Channel config and channels not same size " << channelsValuesArray.size() << "!=" << m_Channels.size());
    return false;
  }

  int i{};
  for (const auto& channelValue : channelsValuesArray) {
    const auto& channel = m_Channels[i++];
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
