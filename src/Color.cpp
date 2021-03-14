/** @file Color.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 2.0
  * @date 2021-01-20
  */

#include <iostream>
#include "Color.h"

namespace rose::color {

    const RGBA RGBA::TransparentBlack{RGBA{0.f,0.f,0.f,0.f}};

    const RGBA RGBA::OpaqueBlack{RGBA{0.f,0.f,0.f,1.f}};

    const RGBA RGBA::OpaqueWhite{RGBA{1.f,1.f,1.f,1.f}};

    HSVA RGBA::toHSVA() const {
        auto cMax = std::max(r(), std::max(g(), b()));
        auto cMin = std::min(r(), std::min(g(), b()));
        auto delta = cMax - cMin;

        float hue = 0.f, saturation = 0.f, value = 0.f;

        if (delta == 0.f) {
            hue = 0.f;
        } else if (cMax == r()) {
            hue = 60.f * fmod((g()-b()) / delta, 6.f);
        } else if (cMax == g()) {
            hue = 60.f * ((b()-r())/delta + 2.f);
        } else if (cMax == b()) {
            hue = 60.f * ((r()-g())/delta + 4.f);
        }

        value = (cMax + cMin) / 2.f;

        if (delta != 0.f) {
            saturation = delta / (1.f-abs(2.f*value-1.f));
        }

        return HSVA{hue, saturation, value, a()};
    }
}
