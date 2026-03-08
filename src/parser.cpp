#include "parser.hpp"
#include "llhttp.h"

RequestParser::RequestParser(llhttp_type_t t) noexcept {
	parser.data = this;

	llhttp_settings_init(&settings);

	llhttp_init(&parser, t, &settings);
}

int RequestParser::execute(std::string data) noexcept {
	return llhttp_execute(&parser, data.c_str(), data.size());
}