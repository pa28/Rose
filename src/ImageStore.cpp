/**
 * @file ImageStore.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-11
 */

#include <array>
#include "ImageStore.h"
#include "Font.h"
#include "Surface.h"
#include "Texture.h"

namespace rose {

    inline ImageId toImageId(int id) { return static_cast<ImageId>(id); }

    inline int fmImageId(ImageId id) { return static_cast<int>(id); }

    static constexpr std::array<IconImage, 34> IconImageList{{
                                                                     {ImageId::Compass, ENTYPO_ICON_COMPASS, color::DarkTextColour},
                                                                     {ImageId::Heart, ENTYPO_ICON_HEART, color::DarkRedHSVA.toRGBA()},
                                                                     {ImageId::HeartEmpty, ENTYPO_ICON_HEART_EMPTY, color::DarkTextColour},
                                                                     {ImageId::Cancel, ENTYPO_ICON_CANCEL_CIRCLED, color::DarkTextColour},
                                                                     {ImageId::Help, ENTYPO_ICON_CIRCLED_HELP, color::DarkTextColour},
                                                                     {ImageId::Info, ENTYPO_ICON_INFO, color::DarkTextColour},
                                                                     {ImageId::Check, ENTYPO_ICON_CHECK, color::DarkTextColour},
                                                                     {ImageId::Alert, ENTYPO_ICON_ALERT, color::DarkTextColour},
                                                                     {ImageId::Lock, ENTYPO_ICON_LOCK, color::DarkTextColour},
                                                                     {ImageId::UpBold, ENTYPO_ICON_UP_BOLD, color::DarkTextColour},
                                                                     {ImageId::UpOpenBig, ENTYPO_ICON_UP_OPEN_BIG, color::DarkTextColour},
                                                                     {ImageId::Left, ENTYPO_ICON_LEFT, color::DarkTextColour},
                                                                     {ImageId::Right, ENTYPO_ICON_RIGHT, color::DarkTextColour},
                                                                     {ImageId::ToEnd, ENTYPO_ICON_TO_END, color::DarkTextColour},
                                                                     {ImageId::Back, ENTYPO_ICON_BACK, color::DarkTextColour},
                                                                     {ImageId::LevelDown, ENTYPO_ICON_LEVEL_DOWN, color::DarkTextColour},
                                                                     {ImageId::OneDot, ENTYPO_ICON_DOT, color::DarkTextColour},
                                                                     {ImageId::TwoDots, ENTYPO_ICON_TWO_DOTS, color::DarkTextColour},
                                                                     {ImageId::ThreeDots, ENTYPO_ICON_THREE_DOTS, color::DarkTextColour},
                                                                     {ImageId::UpDir, ENTYPO_ICON_UP_DIR, color::DarkTextColour},
                                                                     {ImageId::DownDir, ENTYPO_ICON_DOWN_DIR, color::DarkTextColour},
                                                                     {ImageId::LeftDir, ENTYPO_ICON_LEFT_DIR, color::DarkTextColour},
                                                                     {ImageId::RightDir, ENTYPO_ICON_RIGHT_DIR, color::DarkTextColour},
                                                                     {ImageId::Keyboard, ENTYPO_ICON_KEYBOARD, color::DarkTextColour},
                                                                     {ImageId::Globe, ENTYPO_ICON_GLOBE, color::DarkTextColour},
                                                                     {ImageId::Network, ENTYPO_ICON_NETWORK, color::DarkTextColour},
                                                                     {ImageId::Rocket, ENTYPO_ICON_ROCKET, color::DarkTextColour},
                                                                     {ImageId::Location, ENTYPO_ICON_LOCATION, color::DarkTextColour},
                                                                     {ImageId::Camera, ENTYPO_ICON_CAMERA, color::DarkTextColour},
                                                                     {ImageId::ScaleNeedleUp, ENTYPO_ICON_UP_DIR, color::DarkTextColour},
                                                                     {ImageId::ScaleNeedleDown, ENTYPO_ICON_DOWN_DIR, color::DarkTextColour},
                                                                     {ImageId::ScaleNeedleLeft, ENTYPO_ICON_LEFT_DIR, color::DarkTextColour},
                                                                     {ImageId::ScaleNeedleRight, ENTYPO_ICON_RIGHT_DIR, color::DarkTextColour},
                                                                     {ImageId::IconTarget, ENTYPO_ICON_TARGET, color::DarkTextColour},
                                                             }};

    void ImageStore::createIcon(gm::Context &context, IconImage iconImage) {

        FontCache &fontCache{FontCache::getFontCache()};
        auto font = fontCache.getFont("entypo", 50);

        if (!font)
            throw (std::runtime_error(StringCompositor("Can not find font '", "entypo", "'.")));

        gm::Surface surface{
                TTF_RenderUTF8_Blended(font.get(), utf8(iconImage.code).data(), iconImage.color.toSdlColor())};

        int minX = surface->w;
        int minY = surface->h;
        int maxX = 0, maxY = 0;

        for (auto y = 0; y < surface->h; ++y) {
            for (auto x = 0; x < surface->w; ++x) {
                auto rgba = gm::getRGBA(surface->format, surface.pixel(x, y));
                if (rgba.a() > 0) {
                    minX = std::min(minX, x);
                    minY = std::min(minY, y);
                    maxX = std::max(maxX, x);
                    maxY = std::max(maxY, y);
                }
            }
        }

        gm::Surface minimal{maxX - minX + 1, maxY - minY + 1};
        for (auto y = 0; y < minimal->h; ++y) {
            for (auto x = 0; x < minimal->w; ++x) {
                auto rgba = gm::getRGBA(surface->format, surface.pixel(minX + x, minY + y));
                minimal.pixel(x, y) = gm::mapRGBA(minimal->format, rgba);
            }
        }

        gm::Texture texture{minimal.toTexture(context)};
        mImageMap.emplace(iconImage.key, std::move(texture));
    }

    void ImageStore::initialize(gm::Context &context) {
        if (mInitialized)
            return;

        createIcons(context, IconImageList.begin(), IconImageList.end());
        createRoundCorners(context, 5, 10, 2,
                           color::DarkTopColor, color::DarkBotColor, color::DarkLeftColor, color::DarkRightColor);
        createSquareCorners(context, 10, 2,
                            color::DarkTopColor, color::DarkBotColor, color::DarkLeftColor, color::DarkRightColor);
        createCenters(context, 5, 10);
        mInitialized = true;
    }

    void ImageStore::setImage(ImageId imageId, gm::Texture &&texture) {
        mImageMap[imageId] = std::move(texture);
    }

    void copyFullTexture(gm::Context &renderer, gm::Texture &src, gm::Texture &dst) {
        gm::RenderTargetGuard renderTargetGuard(renderer, dst);
        renderer.renderCopy(src);
        gm::TextureSetBlendMode(dst, SDL_BLENDMODE_BLEND);
    }

    gm::Surface createBlankSurface(int size) {
        gm::Surface surface{size, size, 32, rmask, gmask, bmask, amask};
        surface.setBlendMode(SDL_BLENDMODE_BLEND);
        surface.fillRectangle(color::RGBA::TransparentBlack);
        return std::move(surface);
    }

    void
    ImageStore::createSquareCorners(gm::Context &context, int radius, int borderWidth,
                                    rose::color::RGBA top, rose::color::RGBA bot,
                                    rose::color::RGBA left, rose::color::RGBA right) {
        gm::Surface surfaceBevelOut, surfaceNotchIn, surfaceBevelIn, surfaceNotchOut;
        int scale = 1;

        int rS = radius * scale;        // The radius at scale
        int bS = borderWidth * scale;   // The border width at scale
        int size = radius * scale * 2;  // The size of the side of a square to accommodate rS

        surfaceBevelOut = createBlankSurface(size);
        surfaceNotchIn = createBlankSurface(size);
        surfaceBevelIn = createBlankSurface(size);
        surfaceNotchOut = createBlankSurface(size);

        for (int d = 0; d < borderWidth; ++d) {
            color::RGBA color, invColor;
            auto outside = d < borderWidth / 2;

            // For each line in the frame
            for (int line = 0; line < 4; ++line) {
                Rectangle rect;
                switch (line) {
                    case 0: // Top
                        color = top;
                        invColor = bot;
                        rect.x = d;
                        rect.y = d;
                        rect.w = size - d * 2 - 1;
                        rect.h = 1;
                        break;
                    case 1: // Bottom
                        color = bot;
                        invColor = top;
                        rect.x = d;
                        rect.y = size - d - 1;
                        rect.w = size - d * 2 - 1;
                        rect.h = 1;
                        break;
                    case 2: // Left
                        color = left;
                        invColor = right;
                        rect.x = d;
                        rect.y = d;
                        rect.w = 1;
                        rect.h = size - d * 2 - 1;;
                        break;
                    case 3: // Right
                        color = right;
                        invColor = left;
                        rect.x = size - d - 1;
                        rect.y = d;
                        rect.w = 1;
                        rect.h = size - d * 2 - 1;;
                        break;
                    default:
                        break;
                }
                if (line < 4) {
                    auto notchColor = outside ? invColor : color;
                    auto invNotchColor = outside ? color : invColor;
                    surfaceBevelOut.fillRectangle(rect, color);
                    surfaceBevelIn.fillRectangle(rect, invColor);
                    surfaceNotchIn.fillRectangle(rect, notchColor);
                    surfaceNotchOut.fillRectangle(rect, invNotchColor);
                }
            }
        }

        gm::Texture texture;

        texture = surfaceBevelOut.toTexture(context);
        gm::Texture bevelOutCornerTrim = CreateTexture(context, Size{radius * 2, radius * 2});
        copyFullTexture(context, texture, bevelOutCornerTrim);
        setImage(ImageId::BevelOutSquareCorners, std::move(bevelOutCornerTrim));

        texture = surfaceBevelIn.toTexture(context);
        gm::Texture bevelInCornerTrim = CreateTexture(context, Size{radius * 2, radius * 2});
        copyFullTexture(context, texture, bevelInCornerTrim);
        setImage(ImageId::BevelInSquareCorners, std::move(bevelInCornerTrim));

        texture = surfaceNotchOut.toTexture(context);
        gm::Texture notchOutCornerTrim = CreateTexture(context, Size{radius * 2, radius * 2});
        copyFullTexture(context, texture, notchOutCornerTrim);
        setImage(ImageId::NotchOutSquareCorners, std::move(notchOutCornerTrim));

        texture = surfaceNotchIn.toTexture(context);
        gm::Texture notchInCornerTrim = CreateTexture(context, Size{radius * 2, radius * 2});
        copyFullTexture(context, texture, notchInCornerTrim);
        setImage(ImageId::NotchInSquareCorners, std::move(notchInCornerTrim));
    }

    void ImageStore::createRoundCorners(gm::Context &context, int scale, int radius, int borderWidth,
                                        color::RGBA top, color::RGBA bot,
                                        color::RGBA left, color::RGBA right) {

        gm::Surface surfaceBevelOut, surfaceNotchIn, surfaceBevelIn, surfaceNotchOut, roundCorner;

        int rS = radius * scale;        // The radius at scale
        int bS = borderWidth * scale;   // The border width at scale
        int size = radius * scale * 2;  // The size of the side of a square to accommodate rS

        surfaceBevelOut = createBlankSurface(size);
        surfaceNotchIn = createBlankSurface(size);
        surfaceBevelIn = createBlankSurface(size);
        surfaceNotchOut = createBlankSurface(size);
        roundCorner = createBlankSurface(size);

        auto pixelFormat = surfaceBevelOut->format;

        auto r0 = rS * rS;                      // the maximum radius squared of the border corner
        auto rn = (rS - bS / 2) * (rS - bS / 2);    // the notch switch over radius squared
        auto r1 = (rS - bS) * (rS - bS);        // the minimum radius squared

        for (int x = 0; x < size; ++x) {
            for (int y = 0; y < size; ++y) {
                int xr = rS - x;                // x component of the current radius
                int yr = rS - y;                // y component of the current radius
                int r2 = xr * xr + yr * yr;

                // Outside the max radius is transparent
                if (r2 > r0) {
                    auto pixel = color::set_a_value(surfaceBevelOut.pixel(x, y), 0);
                    surfaceBevelOut.pixel(x, y) = pixel;
                    surfaceBevelIn.pixel(x, y) = pixel;
                    surfaceNotchOut.pixel(x, y) = pixel;
                    surfaceNotchIn.pixel(x, y) = pixel;
                    roundCorner.pixel(x, y) = gm::mapRGBA(pixelFormat, color::RGBA::TransparentBlack);
                } else if (r2 > r1) {
                    roundCorner.pixel(x, y) = gm::mapRGBA(pixelFormat, color::RGBA::TransparentBlack);
                    float s = (float) (yr * yr) / (float) r2;
                    float c = (float) (xr * xr) / (float) r2;
                    auto tl = gm::mapRGBA(pixelFormat, top * s + left * c);
                    auto tr = gm::mapRGBA(pixelFormat, top * s + right * c);
                    auto bl = gm::mapRGBA(pixelFormat, bot * s + left * c);
                    auto br = gm::mapRGBA(pixelFormat, bot * s + right * c);
                    if (xr > 0)
                        if (yr > 0) {
                            surfaceBevelOut.pixel(x, y) = tl;
                            surfaceBevelIn.pixel(x, y) = br;
                            if (r2 > rn) {
                                surfaceNotchIn.pixel(x, y) = br;
                                surfaceNotchOut.pixel(x, y) = tl;
                            } else {
                                surfaceNotchIn.pixel(x, y) = tl;
                                surfaceNotchOut.pixel(x, y) = br;
                            }
                        } else {
                            surfaceBevelOut.pixel(x, y) = bl;
                            surfaceBevelIn.pixel(x, y) = tr;
                            if (r2 > rn) {
                                surfaceNotchIn.pixel(x, y) = tr;
                                surfaceNotchOut.pixel(x, y) = bl;
                            } else {
                                surfaceNotchIn.pixel(x, y) = bl;
                                surfaceNotchOut.pixel(x, y) = tr;
                            }
                        }
                    else if (yr > 0) {
                        surfaceBevelOut.pixel(x, y) = tr;
                        surfaceBevelIn.pixel(x, y) = bl;
                        if (r2 > rn) {
                            surfaceNotchIn.pixel(x, y) = bl;
                            surfaceNotchOut.pixel(x, y) = tr;
                        } else {
                            surfaceNotchIn.pixel(x, y) = tr;
                            surfaceNotchOut.pixel(x, y) = bl;
                        }
                    } else {
                        surfaceBevelOut.pixel(x, y) = br;
                        surfaceBevelIn.pixel(x, y) = tl;
                        if (r2 > rn) {
                            surfaceNotchIn.pixel(x, y) = tl;
                            surfaceNotchOut.pixel(x, y) = br;
                        } else {
                            surfaceNotchIn.pixel(x, y) = br;
                            surfaceNotchOut.pixel(x, y) = tl;
                        }
                    }
                    // inside the min radius is transparent
                } else {
                    roundCorner.pixel(x, y) = gm::mapRGBA(pixelFormat, color::RGBA::OpaqueBlack);
                    auto pixel = color::set_a_value(surfaceBevelOut.pixel(x, y), 0);
                    surfaceBevelOut.pixel(x, y) = pixel;
                    surfaceBevelIn.pixel(x, y) = pixel;
                    surfaceNotchOut.pixel(x, y) = pixel;
                    surfaceNotchIn.pixel(x, y) = pixel;
                }
            }
        }

        gm::Texture texture;

        texture = surfaceBevelOut.toTexture(context);
        gm::Texture bevelOutCornerTrim = gm::CreateTexture(context, Size{radius * 2, radius * 2});
        copyFullTexture(context, texture, bevelOutCornerTrim);
        setImage(ImageId::BevelOutRoundCorners, std::move(bevelOutCornerTrim));

        texture = surfaceBevelIn.toTexture(context);
        gm::Texture bevelInCornerTrim = gm::CreateTexture(context, Size{radius * 2, radius * 2});
        copyFullTexture(context, texture, bevelInCornerTrim);
        setImage(ImageId::BevelInRoundCorners, std::move(bevelInCornerTrim));

        texture = surfaceNotchOut.toTexture(context);
        gm::Texture notchOutCornerTrim = gm::CreateTexture(context, Size{radius * 2, radius * 2});
        copyFullTexture(context, texture, notchOutCornerTrim);
        setImage(ImageId::NotchOutRoundCorners, std::move(notchOutCornerTrim));

        texture = surfaceNotchIn.toTexture(context);
        gm::Texture notchInCornerTrim = gm::CreateTexture(context, Size{radius * 2, radius * 2});
        copyFullTexture(context, texture, notchInCornerTrim);
        setImage(ImageId::NotchInRoundCorners, std::move(notchInCornerTrim));

        texture = roundCorner.toTexture(context);
        gm::Texture roundCornerTrim = gm::CreateTexture(context, Size{radius * 2, radius * 2});
        copyFullTexture(context, texture, roundCornerTrim);
        gm::TextureSetBlendMode(roundCornerTrim, SDL_BLENDMODE_NONE);
        setImage(ImageId::RoundCornerTrim, std::move(roundCornerTrim));
    }

    void ImageStore::createCenters(gm::Context &context, int scale, int radius) {
        static constexpr std::array<ImageId, 8> centerList{ImageId::CenterRoundBaseColor,
                                                           ImageId::CenterRoundInvertColor,
                                                           ImageId::CenterRoundRedColor,
                                                           ImageId::CenterRoundGreenColor,
                                                           ImageId::CenterRoundBlueColor,
                                                           ImageId::CenterRoundYellowColor,
                                                           ImageId::CenterSquareBaseColor,
                                                           ImageId::CenterSquareInvertColor};

        int rS = radius * scale;        // The radius at scale
        auto size = 2 * rS;
        auto r0 = rS * rS;              // the maximum radius squared of the border corner

        for (auto id : centerList) {
            color::RGBA color;
            color::HSVA hsva;
            switch (id) {
                case ImageId::CenterRoundBaseColor:
                case ImageId::CenterSquareBaseColor:
                    color = color::DarkBaseColor;
                    break;
                case ImageId::CenterRoundInvertColor:
                case ImageId::CenterSquareInvertColor:
                    color = color::DarkInvertColor;
                    break;
                case ImageId::CenterRoundRedColor:
                    color = color::DarkBaseColor.toHSVA().withHue(0).withMinSaturation(0.6).toRGBA();
                    break;
                case ImageId::CenterRoundGreenColor:
                    color = color::DarkBaseColor.toHSVA().withHue(120).withMinSaturation(0.6).toRGBA();
                    break;
                case ImageId::CenterRoundBlueColor:
                    color = color::DarkBaseColor.toHSVA().withHue(240).withMinSaturation(0.6).toRGBA();
                    break;
                case ImageId::CenterRoundYellowColor:
                    color = color::DarkBaseColor.toHSVA().withHue(60).withMinSaturation(0.6).toRGBA();
                    break;
                default:
                    break;
            }

            gm::Texture texture;

            gm::Surface surface{size, size, 32, rmask, gmask, bmask, amask};
            surface.setBlendMode(SDL_BLENDMODE_BLEND);
            surface.fillRectangle(color);

            switch (id) {
                case ImageId::CenterSquareBaseColor:
                case ImageId::CenterSquareInvertColor:
                    surface.fillRectangle(color);
                    break;
                case ImageId::CenterRoundBaseColor:
                case ImageId::CenterRoundInvertColor:
                case ImageId::CenterRoundRedColor:
                case ImageId::CenterRoundGreenColor:
                case ImageId::CenterRoundYellowColor:
                case ImageId::CenterRoundBlueColor:
                    for (int x = 0; x < size; ++x) {
                        for (int y = 0; y < size; ++y) {
                            int xr = rS - x;                // x component of the current radius
                            int yr = rS - y;                // y component of the current radius
                            int r2 = xr * xr + yr * yr;

                            if (r2 > r0) {
                                surface.pixel(x, y) = color::set_a_value(surface.pixel(x, y), 0);
                            }
                        }
                    }
                    break;
                default:
                    break;
            }

            texture = surface.toTexture(context);
            gm::Texture center = gm::CreateTexture(context, Size{radius * 2, radius * 2});
            gm::RenderTargetGuard renderTargetGuard(context, center);
            context.renderCopy(texture);
            gm::TextureSetBlendMode(center, SDL_BLENDMODE_BLEND);
            setImage(id, std::move(center));
        }
    }
}
