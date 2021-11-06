#pragma once

#include <ostream>
#include <sstream>

namespace visionaray {
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
} // visionaray


#define LOG(LEVEL) ::visionaray::logging::Stream(LEVEL).stream()
