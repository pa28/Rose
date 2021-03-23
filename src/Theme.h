/**
 * @file Theme.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-23
 */

#pragma once

#include "Color.h"

namespace rose {

    /**
     * @class Theme
     * @brief A set of values that provide defaults forming a cohesive visual and operational theme.
     */
    class Theme {
    protected:
        Theme() = default;
    public:
        ~Theme() = default;

        static Theme& getTheme() {
            static Theme instance{};
            return instance;
        }

        /**
         * Colors
         */
        color::HSVA
                BaseColor{200.f, .00, .25, 1.0},
                TopColor{BaseColor.modValue(0.2)},
                BotColor{BaseColor.modValue(-0.15)},
                LeftColor{BaseColor.modValue(0.1)},
                RightColor{BaseColor.modValue(-0.15)},
                InvertColor{BaseColor.modValue(-0.075)},
                TextColour{BaseColor.contrasting()},
                RedHSVA{ 0.f, 1.f, 0.55f, 1.f},
                GreenHSVA{79.f,1.f,.4f,1.f},
                YellowHSVA{ 50.f, 1.f, 0.55f, 1.f};

        /**
         * Fonts
         */
        std::string
                TextFont{"FreeSans"},
                BoldFont{"FreeSansBold"},
                ObliqueFont{"FreeSansOblique"},
                BoldObliqueFont{"FreeSansBoldOblique"},
                FixedFont{"FreeMono"},
                FixedBoldFont{"FreeMonoBold"},
                FixedObliqueFont{"FreeMonoOblique"},
                FixedBoldObliqueFont{"FreeMonoBoldOblique"},
                EntypoFont{"entypo"};

        /**
         * Font Point Sizes
         */
        int ButtonPointSize{30},
            LabelPointSize{30},
            TextPointSize{20};

        /**
         * Padding
         */
         int ButtonPadding{5};
    };
}

