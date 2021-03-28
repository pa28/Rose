/**
 * @file CommonSignals.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-28
 */

#pragma once

#include <cstdint>
#include "Signals.h"

namespace rose {
    /// Protocol for notifying objects that the application is about to start a new frame.
    using GraphicsModelFrameProtocol = Protocol<uint32_t>;

    class CommonSignals {
    protected:
        CommonSignals() = default;

    public:
        ~CommonSignals() = default;

        CommonSignals(const CommonSignals&) = delete;

        CommonSignals(CommonSignals&&) = delete;

        CommonSignals& operator=(const CommonSignals&) = delete;

        CommonSignals& operator=(CommonSignals&&) = delete;

        static CommonSignals& getCommonSignals() {
            static CommonSignals instance{};
            return instance;
        }

        GraphicsModelFrameProtocol::signal_type frameSignal{};

    };
}

