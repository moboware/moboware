#pragma once
#include "channels/web_socket_channel.h"
#include "common/application_base.h"

namespace moboware::applications {
class Application : public common::ApplicationBase
{
public:
  explicit Application(const std::shared_ptr<common::Service>& service, //
                       const std::vector<std::shared_ptr<common::ChannelBase>>& channels);
  int Run(const int argc, const char* argv[]) final;

private:
  bool ReadCommandline(const int argc, const char* argv[]) final;
  bool LoadConfig(const std::string& configFile, const std::filesystem::path& applicationName) final;
  void Stop() final;

  std::vector<std::shared_ptr<common::ChannelBase>> m_Channels{};
};

}