#include "src/common/config.h"
#include "src/common/log.h"
#include "src/net/tcp/tcp_buffer.h"

void test_buffer()
{
    using namespace mrpc;
    TcpBuffer buf(27);

    // int
    int32_t i1 = 132456;
    buf.writeInt<int32_t>(i1);

    int64_t i2 = 46465165;
    buf.writeInt<int64_t>(i2);

    int32_t req = buf.readInt<int32_t>();
    LOG_DEBUG << "req: " << req;

    int64_t req2 = buf.readInt<int64_t>();
    LOG_DEBUG << "req2: " << req2;

    // string
    for (int i = 0; i < 100; i++) {
        buf.wirteToBuffer("123456789123456789123456789");
    }
    std::string tmp = buf.peekAsString(buf.readAble());
    std::vector<char> tmp2;
    buf.peekAsVector(tmp2, buf.readAble());
    LOG_DEBUG << "readAble: " << buf.readAble() << ", readIndex: " << buf.readIndex();
    LOG_DEBUG << "writeAble: " << buf.writAble() << ", writeIndex: " << buf.writeIndex();
    LOG_DEBUG << ", tmplen: " << tmp.length();
    LOG_DEBUG << ", tmp2len: " << tmp2.size();

    std::string tmp4 = buf.readAllAsString();
    LOG_DEBUG << "readAble: " << buf.readAble() << ", readIndex: " << buf.readIndex();
    LOG_DEBUG << "writeAble: " << buf.writAble() << ", writeIndex: " << buf.writeIndex();
    LOG_DEBUG << ", tmp4len: " << tmp4.size();
}
void test_buffer1()
{
    using namespace mrpc;

    TcpBuffer buf("admin");
    int32_t aaa = 654321;
    int32_t bbb = 123456;
    buf.writeInt<int32_t>(aaa);
    buf.writeInt<int32_t>(bbb);

    LOG_DEBUG << buf.peekAsString(1, 3);

    LOG_DEBUG << buf.readAsString(5);
    LOG_DEBUG << buf.readInt<int32_t>();
    LOG_DEBUG << buf.readInt<int32_t>();
}

int main()
{
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    mrpc::Logger::InitGlobalLogger();

    // tcp buffer
    test_buffer1();

    return 0;
}