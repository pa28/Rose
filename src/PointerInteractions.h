/**
 * @file ButtonSemantics.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-20
 */

#pragma once

namespace rose {

    /**
    * @class PointerInteractions
    * @brief
    */
    class PointerInteractions {
    protected:

    public:
        PointerInteractions() = default;
        virtual ~PointerInteractions() = default;

        PointerInteractions(const PointerInteractions&) = delete;
        PointerInteractions(PointerInteractions &&) = delete;
        PointerInteractions& operator=(const PointerInteractions&) = delete;
        PointerInteractions& operator=(PointerInteractions&&) = delete;


    };
}

