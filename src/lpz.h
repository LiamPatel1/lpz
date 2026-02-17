#pragma once
#include <string>

namespace lpz {
	enum class ErrorCode {
		SystemError,
		InputError,
	};

	struct Error {
		ErrorCode c;
		std::string m;
	};
}