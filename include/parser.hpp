#pragma once

#include <llhttp.h>
#include <string>

enum class ParserType {
	BOTH = 0,
	REQUEST = 1,
	RESPONSE = 2
};

class RequestParser {
  public:
	RequestParser(llhttp_type_t t) noexcept;

	int execute(std::string data) noexcept;

  private:
	llhttp_t parser{};
	llhttp_settings_t settings{};
};