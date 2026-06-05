#include "coring_server/parser/base_parser.hpp"
#include "llhttp.h"

llhttp_errno_t BaseParser::execute(std::string data) noexcept {
	return llhttp_execute(&parser, data.c_str(), data.size());
}

void BaseParser::resume_after_upgrade() noexcept {
	llhttp_resume_after_upgrade(&parser);
}

void BaseParser::resume() noexcept {
	llhttp_resume(&parser);
}

void BaseParser::pause() noexcept {
	llhttp_pause(&parser);
}

bool BaseParser::should_keep_alive() noexcept {
	return llhttp_should_keep_alive(&parser) == true;
}

unsigned char BaseParser::get_type() noexcept {
	return llhttp_get_type(&parser);
}

unsigned char BaseParser::get_http_minor() noexcept {
	return llhttp_get_http_minor(&parser);
}

unsigned char BaseParser::get_http_major() noexcept {
	return llhttp_get_http_major(&parser);
}

std::string BaseParser::get_error_name(llhttp_errno err) const noexcept {
	return llhttp_errno_name(err);
}

void BaseParser::set_error_reason(std::string reason) noexcept {
	return llhttp_set_error_reason(&parser, reason.c_str());
}

std::string BaseParser::get_error_reason() noexcept {
	return llhttp_get_error_reason(&parser);
}

std::string BaseParser::get_error_pos() noexcept {
	return llhttp_get_error_pos(&parser);
};

std::string BaseParser::status_name(llhttp_status_t status) const noexcept {
	return llhttp_status_name(status);
}
