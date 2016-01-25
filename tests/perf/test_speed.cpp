#include <cstdint>
#include <cinttypes>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <thread>
#include <condition_variable>
#include "test_switch.h"

namespace
{
	class thread_latch
	{
	public:
		thread_latch(): _go(false), _halt(false) {}
		void wait() const;
		void release();
		bool halted() const;
		void halt();
	private:
		mutable std::mutex _m;
		mutable std::condition_variable _cv;
		bool _go;
		std::atomic<bool> _halt;
	};

	void thread_latch::wait() const
	{
		std::unique_lock<std::mutex> lock(_m);
		while (!_go)
		{
			_cv.wait(lock);
		}
	}

	void thread_latch::release()
	{
		const std::lock_guard<std::mutex> lock(_m);
		_go = true;
		_cv.notify_all();
	}

	bool thread_latch::halted() const
	{
		return _halt;
	}

	void thread_latch::halt()
	{
		_halt = true;
	}

	class thread_group
	{
	public:
		thread_group(const thread_latch &latch, const unsigned n);
		void start(const std::function<void(void)> f);
		uint64_t join();
	private:
		void run();
		const thread_latch &_latch;
		const unsigned _n;
		std::atomic<uint64_t> _count;
		std::function<void(void)> _f;
		std::vector<std::thread> _ths;
	};

	thread_group::thread_group(const thread_latch &latch, const unsigned n):
		_latch(latch), _n(n), _count(0)
	{
	}

	void thread_group::start(const std::function<void(void)> f)
	{
		_f = std::move(f);
		for (auto i = _n; 0 < i--;)
		{
			_ths.push_back(std::thread(&thread_group::run, this));
		}
	}

	uint64_t thread_group::join()
	{
		for (auto &th : _ths)
		{
			th.join();
		}
		_ths.clear();
		return _count;
	}

	void thread_group::run()
	{
		uint64_t count = 0;
		while (!_latch.halted())
		{
			_f();
			++count;
		}
		_count += count;
	}

	class bench
	{
	public:
		bench() {}
		void setup(const std::function<void(void)> f);
		unsigned run(const unsigned n, const unsigned seconds);
	private:
		std::function<void(void)> _f;
	};

	void bench::setup(const std::function<void(void)> f)
	{
		_f = std::move(f);
	}

	unsigned bench::run(const unsigned n, const unsigned seconds)
	{
		thread_latch latch;
		thread_group tg(latch, n);
		tg.start(_f);
		latch.release();
		std::this_thread::sleep_for(std::chrono::seconds(seconds));
		latch.halt();
		return tg.join();
	}
}

int main(int argc, char *argv[])
{
	XLOG_INIT();
	unsigned n = 1;
	if (1 < argc)
	{
		n = std::stoi(argv[1]);
		if (n <= 0 || 99 < n)
		{
			fprintf(stderr, "Bad thread count (%u).\n", n);
			return -1;
		}
	}
	unsigned seconds = 1;
	if (2 < argc)
	{
		seconds = std::stoi(argv[2]);
		if (seconds <= 0 || 60*60 < seconds)
		{
			fprintf(stderr, "Bad duration (%u).\n", seconds);
			return -1;
		}
	}
	bench b;
	b.setup([](){
		XLOG_STATEMENT();
	});
	const uint64_t k = b.run(n, seconds);
	fprintf(stdout, "%" PRIu64 "\n", static_cast<uint64_t>(k));
	return 0;
}
