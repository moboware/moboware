#include "applications/application.h"
#include "common/log.h"
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

bool Application::LoadConfig(const std::string& configFile)
{
  std::ifstream configStream(configFile.c_str());
  if (!configStream.is_open()) {
    LOG("Failed to open file:" << configFile);
    return false;
  }

  Json::CharReaderBuilder builder{};
  Json::Value rootDocument{};
  auto collectComments{ false };
  Json::String* errors{};
  if (!Json::parseFromStream(builder, configStream, &rootDocument, errors)) {
    LOG("Failed to parse config file");
    return false;
  }

  if (!rootDocument.isMember(CHANNELS_VALUE)) {
    LOG("Channel item not found");
    return false;
  }

  const Json::Value channelsValue = rootDocument[CHANNELS_VALUE];
  if (!channelsValue.isArray()) {
    LOG("Channels value is not an array");
    return false;
  }

  if (channelsValue.size() != m_Channels.size()) {
    LOG("Channel config and channels not same size");
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
  if (!LoadConfig(configFile)) {
    LOG("Failed to load config");
    return EXIT_FAILURE;
  }

  for (const auto channel : m_Channels) {
    if (!channel->Start()) {
      LOG("Failed to start channel");

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
