#include "src/common/config.h"
#include "src/common/log.h"
#include "src/net/tcp/net_addr.h"

int main()
{
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    mrpc::Logger::InitGlobalLogger();

    mrpc::IPNetAddr addr("127.0.0.1", 12345);
    mrpc::IPNetAddr addr2("127.0.0.1:12345");
    LOG_DEBUG << "create addr: " << addr.toString();
    LOG_DEBUG << "create addr: " << addr2.toString();
    return 0;
}