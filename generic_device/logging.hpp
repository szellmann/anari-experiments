#pragma once

#include <ostream>
#include <sstream>

namespace generic {
    namespace logging
    {
        enum class Level
        {
            Error,
            Warning,
            Info,
        };

        class Stream
        {   

        public:
            Stream(Level level);
           ~Stream();

            inline std::ostream& stream()
            {
                return stream_;
            }

        private:
            std::ostringstream stream_;
            Level level_;

        };
    } // logging
} // generic


#define LOG(LEVEL) ::generic::logging::Stream(LEVEL).stream()
