#pragma once

#include "base_parser.hpp"
#include "llhttp.h"
#include <coroutine>

namespace coring_server {
    struct promise;
    class ClientReqParser : public BaseParser {
    public:
    	ClientReqParser(std::coroutine_handle<promise> handle) : BaseParser(HTTP_REQUEST), handle(handle) {
    		on_message_complete = [this]() {
    			this->handle.resume();
    			return 0;
    		};
    	}

    private:
    	std::coroutine_handle<promise> handle;
    };
} // namespace coring_server
