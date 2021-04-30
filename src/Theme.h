/**
 * @file Theme.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-23
 */

#pragma once

#include "Color.h"
#include "Types.h"

namespace rose {

    enum class ThemeColor : size_t {
        Base,
        Top,
        Bottom,
        Left,
        Right,
        Invert,
        Text,
        Red,
        Green,
        Yellow,
        RedText,
        GreenText,
        YellowText,
    };

    /**
     * @class Theme
     * @brief A set of values that provide defaults forming a cohesive visual and operational theme.
     */
    class Theme {
    protected:
        Theme() = default;
    public:

        static Theme &getTheme() {
            static Theme instance{};
            return instance;
        }

        /**
         * Colors
         */
        color::HSVA BaseColorHSVA{200.f, .00, .25, 1.0};

        std::array<color::HSVA, 13> _hsva{{
                                                 BaseColorHSVA,
                                                 BaseColorHSVA.modValue(0.2),
                                                 BaseColorHSVA.modValue(-0.15),
                                                 BaseColorHSVA.modValue(0.1),
                                                 BaseColorHSVA.modValue(-0.15),
                                                 BaseColorHSVA.modValue(-0.075),
                                                 BaseColorHSVA.contrasting(),
                                                 {0.f, 1.f, 0.55f, 1.f},
                                                 {79.f, 1.f, .4f, 1.f},
                                                 {50.f, 1.f, 0.55f, 1.f},
                                                 {0.f, 1.f, 0.75f, 1.f},
                                                 {79.f, 1.f, 0.75f, 1.f},
                                                  {50.f, 1.f, 0.75f, 1.f}
                                         }};

        std::array<color::RGBA, 13> _rgba{{
                                                 _hsva[0].toRGBA(),
                                                 _hsva[1].toRGBA(),
                                                 _hsva[2].toRGBA(),
                                                 _hsva[3].toRGBA(),
                                                 _hsva[4].toRGBA(),
                                                 _hsva[5].toRGBA(),
                                                 _hsva[6].toRGBA(),
                                                 _hsva[7].toRGBA(),
                                                 _hsva[8].toRGBA(),
                                                 _hsva[9].toRGBA(),
                                                 _hsva[10].toRGBA(),
                                                 _hsva[11].toRGBA(),
                                                 _hsva[12].toRGBA()
                                         }};

        color::RGBA rgba(ThemeColor themeColor) {
            return _rgba[static_cast<std::size_t>(themeColor)];
        };

        color::HSVA hsva(ThemeColor themeColor) {
            return _hsva[static_cast<std::size_t>(themeColor)];
        }

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

        /**
         * Sizes
         */
        Size ImageLabelSize{40,40};

        /**
         * Frame styles
         */
        FrameSettings CleanFrame{
                { rgba(ThemeColor::Base), rgba(ThemeColor::Invert)},
                {BorderStyle::None, BorderStyle::None}
        };

        FrameSettings BevelFrame{
                { rgba(ThemeColor::Base), rgba(ThemeColor::Invert)},
                {BorderStyle::BevelOut, BorderStyle::BevelIn}
        };

        FrameSettings SemiBevelFrame{
                { rgba(ThemeColor::Base), rgba(ThemeColor::Invert)},
                {BorderStyle::NotchIn, BorderStyle::BevelIn}
        };
    };
}

