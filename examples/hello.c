#define ZF_LOG_LEVEL ZF_LOG_INFO
#define ZF_LOG_TAG "MAIN"
#include <signal.h>
#include <unistd.h>
#include <zf_log.h>

int main(int argc, char *argv[])
{
	zf_log_set_tag_prefix("hello");

	ZF_LOGV("Argument of this VERBOSE message will not be evaluated: %i",
			kill(getpid(), SIGKILL));
	ZF_LOGI("You will see that INFO message");

	ZF_LOGI("You will see the number of arguments: %i", argc);
	ZF_LOGD("You will NOT see the first argument: %s", *argv);

	zf_log_set_output_level(ZF_LOG_WARN);
	ZF_LOGW("You will see this WARNING message");
	ZF_LOGI("You will NOT see this INFO message");

	const char data[] =
			"Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
			"Aliquam pharetra orci id velit porttitor tempus.";
	ZF_LOGW_MEM(data, sizeof(data), "Lorem ipsum at %p:", data);

	return 0;
}
