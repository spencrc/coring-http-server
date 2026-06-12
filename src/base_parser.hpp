#pragma once

#include <functional>
#include <llhttp.h>
#include <string>

enum class ParserType {
	BOTH = 0,
	REQUEST = 1,
	RESPONSE = 2
};

#define CALLBACK_LIST(XX)                 \
	XX(on_message_begin)                  \
	XX(on_message_complete)               \
	XX(on_url_complete)                   \
	XX(on_method_complete)                \
	XX(on_protocol_complete)              \
	XX(on_version_complete)               \
	XX(on_status_complete)                \
	XX(on_header_field_complete)          \
	XX(on_header_value_complete)          \
	XX(on_chunk_header)                   \
	XX(on_chunk_extension_name_complete)  \
	XX(on_chunk_extension_value_complete) \
	XX(on_chunk_complete)                 \
	XX(on_reset)                          \
	XX(on_headers_complete)

#define DATA_CALLBACK_LIST(XX)   \
	XX(on_protocol)              \
	XX(on_url)                   \
	XX(on_status)                \
	XX(on_method)                \
	XX(on_version)               \
	XX(on_header_field)          \
	XX(on_header_value)          \
	XX(on_chunk_extension_name)  \
	XX(on_chunk_extension_value) \
	XX(on_body)

#define WRAPPER_CALLBACK_HANDLE(name) \
	std::function<int()> name = []() { return 0; };

#define WRAPPER_DATA_CALLBACK_HANDLE(name) \
	std::function<int(std::string)> name = [](std::string) { return 0; };

#define CALLBACK_BRIDGE(name)                            \
	static int cb_##name(llhttp_t *p) {                  \
		auto *self = static_cast<BaseParser *>(p->data); \
		return self->name();                             \
	}

#define DATA_CALLBACK_BRIDGE(name)                                     \
	static int cb_##name(llhttp_t *p, const char *at, size_t length) { \
		auto *self = static_cast<BaseParser *>(p->data);               \
		return self->name(std::string(at, length));                    \
	}

#define SET_ANY_BRIDGE(name) settings.name = cb_##name;

class BaseParser {
  public:
	BaseParser(llhttp_type_t t) noexcept {
		llhttp_settings_init(&settings);

		CALLBACK_LIST(SET_ANY_BRIDGE)
		DATA_CALLBACK_LIST(SET_ANY_BRIDGE)

		llhttp_init(&parser, t, &settings);

		parser.data = this;
	}

	llhttp_errno_t execute(std::string_view data) noexcept;
	void resume_after_upgrade() noexcept;
	void resume() noexcept;
	void pause() noexcept;
	bool should_keep_alive() noexcept;
	unsigned char get_type() noexcept;
	unsigned char get_http_minor() noexcept;
	unsigned char get_http_major() noexcept;
	std::string get_error_name(llhttp_errno err) const noexcept;
	void set_error_reason(std::string reason) noexcept;
	std::string get_error_reason() noexcept;
	std::string get_error_pos() noexcept;
	std::string status_name(llhttp_status_t status) const noexcept;

  protected:
	llhttp_t parser{};
	llhttp_settings_t settings{};

	CALLBACK_LIST(WRAPPER_CALLBACK_HANDLE)
	DATA_CALLBACK_LIST(WRAPPER_DATA_CALLBACK_HANDLE)

  private:
	DATA_CALLBACK_LIST(DATA_CALLBACK_BRIDGE)
	CALLBACK_LIST(CALLBACK_BRIDGE)
};

#undef CALLBACK_LIST
#undef DATA_CALLBACK_LIST
#undef CALLBACK_BRIDGE
#undef DATA_CALLBACK_BRIDGE
#undef WRAPPER_DATA_CALLBACK_HANDLE
#undef WRAPPER_CALLBACK_HANDLE
#undef SET_ANY_BRIDGE