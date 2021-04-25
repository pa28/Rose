/**
 * @file Manager.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-24
 */

#pragma once

#include "Visual.h"
#include "Layout.h"

namespace rose {

    class Row : public Manager {
    protected:

    public:
        Row() : Manager() {
            setLayoutManager(std::make_unique<LinearLayout>(Orientation::Horizontal));
        }

        static constexpr std::string_view id = "Row";
        std::string_view nodeId() const noexcept override {
            return id;
        }
    };

    class Column : public Manager {
    protected:

    public:
        Column() : Manager() {
            setLayoutManager(std::make_unique<LinearLayout>(Orientation::Vertical));
        }

        static constexpr std::string_view id = "Column";
        std::string_view nodeId() const noexcept override {
            return id;
        }
    };
}
