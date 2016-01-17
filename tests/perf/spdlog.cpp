#include <spdlog/spdlog.h>

namespace
{
	class null_sink: public spdlog::sinks::sink
	{
	public:
		void log(const spdlog::details::log_msg &) override {}
		void flush() override {}
	};
}

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	int howmany = 10000000;
    auto logger = spdlog::create<null_sink>("null_sink");
	//"file_logger", "logs/spd-bench-st.txt", false);

    logger->set_pattern("[%Y-%b-%d %T.%e]: %v");
    for(int i  = 0 ; i < howmany; ++i)
        logger->info() << "spdlog message #" << i << ": This is some text for your pleasure";
	return 0;
}
