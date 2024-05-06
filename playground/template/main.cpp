#include <chrono>
#include <iostream>

template <typename TDataHandler>   //
class DataReader : private TDataHandler {
public:
  DataReader() = default;
  ~DataReader() = default;

  void ReadData()
  {
    // read from socket etc...
    // Call callback
    const std::string_view data{"likrjfgl;sdkjg;ldsjg;ldkj gsdf"};
    TDataHandler::OnRead(data);
  }

private:
};

class DataHandler {   // callback class
public:
  void OnRead(const std::string_view &data)
  {
    std::cout << "Data:" << data << std::endl;
  }
};

int main(int, char **)
{

  DataReader<DataHandler> dataReader;
  dataReader.ReadData();
  return 0;
}