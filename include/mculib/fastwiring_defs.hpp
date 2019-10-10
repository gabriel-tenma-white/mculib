#pragma once

// to be declared and set by user
extern int cpu_mhz;

namespace mculib {
	static constexpr int INPUT = 0;
	static constexpr int OUTPUT = 1;
	static constexpr int INPUT_PULLUP = 2;
	static constexpr int INPUT_PULLDOWN = 3;

	static constexpr int HIGH = 1;
	static constexpr int LOW = 0;

}
