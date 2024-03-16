#include "src/common/msg_id_util.h"
#include "src/common/log.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>


namespace mrpc
{

static int g_msg_id_len = 20;
static int g_random_fd = -1;
static thread_local std::string t_msg_id_no;
static thread_local std::string t_max_msg_id_no;

std::string MsgIdUtil::GenMsgId()
{
    if (t_msg_id_no.empty() || t_msg_id_no == t_max_msg_id_no) {
        if (g_random_fd == -1) {
            g_random_fd = ::open("/dev/urandom", O_RDONLY);
        }
        std::string res(g_msg_id_len, 0);
        if (::read(g_random_fd, &res[0], g_msg_id_len) != g_msg_id_len) {
            LOG_ERROR << "read form /dev/urandom error";
            ::close(g_msg_id_len);
            g_msg_id_len = -1;
            return "";
        }

        for (int i = 0; i < g_msg_id_len; i++) {
            uint8_t x = (uint8_t) res[i] % 10;
            res[i] = x + '0';
            t_max_msg_id_no += '9';
        }
        t_msg_id_no = res;
    } else {
        int i = t_msg_id_no.length() - 1;
        while (t_msg_id_no[i] == '9' && i > 0) {
            i -= 1;
        }
        if (i > 0) {
            t_msg_id_no[i] += 1;
            for (size_t j = i + 1; j < t_msg_id_no.length(); j++) {
                t_msg_id_no[j] = '0';
            }
        }
    }

    return t_msg_id_no;
}


}// namespace mrpc