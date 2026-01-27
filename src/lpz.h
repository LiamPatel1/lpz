#pragma once
#include <string>
#include <expected>

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