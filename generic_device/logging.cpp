#include <iostream>

#include "logging.hpp"

//-------------------------------------------------------------------------------------------------
// Terminal colors
//

#define TERMINAL_RED     "\033[0;31m"
#define TERMINAL_GREEN   "\033[0;32m"
#define TERMINAL_YELLOW  "\033[1;33m"
#define TERMINAL_RESET   "\033[0m"
#define TERMINAL_DEFAULT TERMINAL_RESET
#define TERMINAL_BOLD    "\033[1;1m"


namespace generic {
    namespace logging
    {
        Stream::Stream(Level level)
            : level_(level)
        {
        }

        Stream::~Stream()
        {
            if (level_ == Level::Info)
                std::cout << TERMINAL_GREEN << stream_.str() << TERMINAL_RESET << '\n';
            else if (level_ == Level::Warning)
                std::cout << TERMINAL_YELLOW << stream_.str() << TERMINAL_RESET << '\n';
            else if (level_ == Level::Error)
                std::cout << TERMINAL_RED << stream_.str() << TERMINAL_RESET << '\n';
        }
    } // logging
} // generic


