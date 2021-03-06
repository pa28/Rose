/** @file Theme.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-22
  * @brief Application visual theme related values.
  */

/*
    Another significant redesign to update the coding standards to C++17,
    reduce the amount of bare pointer handling (especially in user code),
    and focus on the RaspberryPi environment.
    
    License terms for the changes as well as the base nanogui-sdl code are
    contained int the LICENSE.txt file.
    
    A significant redesign of this code was contributed by Christian Schueller.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include "Color.h"
#include "Constants.h"


namespace rose {

    /**
     * @class Theme
     * @brief Storage for values which affect the look and feel of the application.
     */
    class Theme {
    public:
        Theme();

        color::HSVA mBaseColorHSLA{dBaseColorHSLA};
        color::RGBA mBaseColor{dBaseColorHSLA};
        color::RGBA mTopColor{dTopColor};
        color::RGBA mBotColor{dBotColor};
        color::RGBA mLeftColor{dLeftColor};
        color::RGBA mRightColor{dRightColor};
        color::RGBA mInvertColor{dInvertColor};
        color::RGBA mTextColour{dBaseColorHSLA.contrasting()};
        color::RGBA mWhite{dWhite};
        color::RGBA mBlack{dBlack};
        color::RGBA mRed{dRed};
        color::RGBA mYellow{dYellow};
        color::RGBA mGreen{dGreen};

        BorderStyle mBorderStyle{dBorderStyle};
        CornerStyle mCornerStyle{dCornerStyle};
        int mFrameWidth{dFrameWidth};
        int mPadding{dPadding};
        int mButtonPadding{dButtonPadding};
        int mCPUNormalMax{dCPUNormalMax};
        int mCPUWarningMax{dCPUWarningMax};
        int mImageButtonSize{dImageButtonSize};
        std::string mFontRootPath{dFontRootPath};
        std::string mDefaultFontName{dDefaultFontName};

        std::string mTimeBoxTimeFont{dFixePitchFont};
        std::string mKeyboardFont{dFixePitchFont};
        std::string mTimeBoxHoursMinFmt{dTimeBoxHoursMinFmt};
        std::string mTimeBoxSecFmt{dTimeBoxSecFmt};
        std::string mTimeBoxSmallSecFmt{dTimeBoxSmallSecFmt};
        std::string mDateBoxFont{dDateBoxFont};
        std::string mTimeBoxDateFmt{dTimeBoxDateFmt};
        std::string mTimeBoxSmallDateFmt{dTimeBoxSmallDateFmt};
        int mTimeBoxFontSize{dTimeBoxFontSize};
        int mTimeBoxDateFontSize{dTimeBoxDateFontSize};
        int mDateBoxFontSize{dDateBoxFontSize};
        int mFontPointSize{dFontPointSize};
        int mKeyboardFontPointSize{dKeyboardPointSize};
        int mIconFontSize{dIconPointSize};
        int mTitleFontSize{dTitlePointSize};
        int mLabelBadgeSpace{dLabelBadgeSpace};
        std::string mIconFontName{dIconFontName};

//        static constexpr color::HSVA dBaseColorHSLA{{200.f, .00, .20, 1.0}};
//        static constexpr std::array<float, 5> dColorSetMods{0.2, -0.15, 0.1, -0.2, -0.1};
        static constexpr color::HSVA dBaseColorHSLA{{200.f, .00, .15, 1.0}};
        static constexpr std::array<float, 5> dColorSetMods{0.2, -0.15, 0.1, -0.15, -0.075};
        static constexpr color::RGBA dBaseColor{dBaseColorHSLA};
        static constexpr color::RGBA dTopColor{dBaseColorHSLA.modValue(0.2)};
        static constexpr color::RGBA dBotColor{dBaseColorHSLA.modValue(-0.15)};
        static constexpr color::RGBA dLeftColor{dBaseColorHSLA.modValue(0.1)};
        static constexpr color::RGBA dRightColor{dBaseColorHSLA.modValue(-0.15)};
        static constexpr color::RGBA dInvertColor{dBaseColorHSLA.modValue(-0.075)};

        static constexpr color::HSVA dWhite{{0.0f, 0.0f, 1.0f, 1.0f }};
        static constexpr color::HSVA dBlack{{0.0f, 0.0f, 0.0f, 1.0f }};
        static constexpr color::HSVA dRed{{10.f, 0.7f, 0.8f, 1.0f }};
        static constexpr color::HSVA dYellow{{60.f, 0.7f, 0.8f, 1.0f }};
        static constexpr color::HSVA dGreen{{120.f, 0.7f, 0.8f, 1.0f }};

        static constexpr BorderStyle dBorderStyle{BorderStyle::Unset};
        static constexpr CornerStyle dCornerStyle{CornerStyle::Square};
        static constexpr int dFrameWidth = 2;
        static constexpr int dPadding = 6;
        static constexpr int dButtonPadding = 4;
        static constexpr int dImageButtonSize = 50;
        static constexpr int dCPUNormalMax = 55000;
        static constexpr int dCPUWarningMax = 60000;

        static constexpr std::string_view dFontRootPath = "/usr/share/fonts:/usr/local/share/fonts";
        static constexpr std::string_view dDefaultFontName = "FreeSans";

        static constexpr std::string_view dFixePitchFont = "FreeMonoBold";    ///< Default font name
        static constexpr std::string_view dTimeBoxHoursMinFmt = "%R";           ///< Default hours minutes format
        static constexpr std::string_view dTimeBoxSecFmt = "%S %Z";             ///< Default seconds format
        static constexpr std::string_view dTimeBoxSmallSecFmt = "%Z";           ///< Small version seconds format
        static constexpr int dTimeBoxFontSize = 30;                             ///< Default time font size
        static constexpr int dTimeBoxDateFontSize = 20;                         ///< Default date font size

        static constexpr std::string_view dDateBoxFont = "FreeSansBold";
        static constexpr std::string_view dTimeBoxDateFmt = "%a %b %d, %Y";
        static constexpr std::string_view dTimeBoxSmallDateFmt = "%a %b %d";
        static constexpr int dDateBoxFontSize = 20;

        static constexpr int dFontPointSize = 30;
        static constexpr int dKeyboardPointSize = 40;
        static constexpr int dIconPointSize = 50;
        static constexpr int dTitlePointSize = 20;

        static constexpr int dLabelBadgeSpace = 5;

        static constexpr std::string_view dIconFontName = "entypo";

    };
}



