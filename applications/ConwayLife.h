/** @file ConwayLife.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-12-27
  * @brief Conway's Game of Life
  */

#pragma once

#include <array>
#include <exception>
#include <memory>
#include <mutex>
#include "Constants.h"
#include "ScreenMetrics.h"
#include "Rose.h"
#include "Signals.h"
#include "Slider.h"
#include "Timer.h"

static constexpr size_t ScreenWidth = 800;
static constexpr size_t ScreenHeight = 480;
static constexpr size_t PixelSize = 5;
static constexpr size_t BoardWidth = ScreenWidth / PixelSize;
static constexpr size_t BoardHeight = ScreenHeight / PixelSize;

/**
 * @brief Set the general location of the pattern initialization
 */
enum PatternOrigin {
    TopLeft,    ///< Top left corner of the board
    BottomLeft, ///< Bottom left corner of the board
    Center,     ///< Centered on the board.
};

struct StartPattern {
    PatternOrigin   origin;         ///< The general origin of the pattern
    size_t pixelSize;               ///< The size of the square pixels that represent cells.
    rose::Size boardSize;        ///< The size of the board in cells
    rose::Position offset;       ///< The off set from origin to place the initial pattern in. Corrects for pattern size.
    rose::Position delay;        ///< The delay in generations before automatic scrolling
    rose::Position velocity;     ///< The velocity and direction of automatic scrolling in generations per cell
    std::string_view data;          ///< A string encoding the initial pattern.
};

static StartPattern Switch1 = {
        TopLeft, 6, rose::Size{400,240}, rose::Position {350,200},
        rose::Position {0,0}, rose::Position{0,0},
        "1: 7; 2: 5 7 8; 3: 5 7; 4: 5; 5: 3; 6: 1 3;"
};

static StartPattern Switch2 = {
        TopLeft, 6, rose::Size{400,240}, rose::Position { 350, 200},
        rose::Position { 0, 0}, rose::Position{0,0},
        "1: 1 2 3 5; 2: 1; 3: 4 5; 4: 2 3 5; 5: 1 3 5;"
};

static StartPattern Acorn = {
        Center, 6, rose::Size{400, 240}, rose::Position{0, 0},
        rose::Position{0, 0}, rose::Position{0, 0},
        "1:2; 2:4; 3:1 2 5 6 7;"
};

static StartPattern GosperGliderGun = {
        TopLeft, 10, rose::Size{100, 50}, rose::Position{0, 0},
        rose::Position{0, 0}, rose::Position{0, 0},
        "1:25; 2:23 25; 3:13 14 21 22 35 36; 4:12 16 21 22 35 36; 5:1 2 11 17 21 22;"
        "6:1 2 11 15 17 18 23 25; 7:11 17 25; 8:12 16; 9:13 14;"
};

static StartPattern Tribute = {
        TopLeft, 10, rose::Size{80, 50}, rose::Position{ 5, 38},
        rose::Position{0, 0}, rose::Position{0, 0},
        "1:3 4 5; 2:3 5; 3:3 5; 4:4; 5:1 3 4 5; 6: 2 4 6; 7:5; 8:3 5; 9:3 5;"
};

/**
 * @brief The state of a cell.
 */
enum State {
    Dead,   ///< Cell is empty
    Live,   ///< Cell is living
    Dying,  ///< Cell is dead on the next generation.
    Born,   ///< Cell is alive on the next generation.
};

struct Cell {
    State state{Dead};         ///< The current state of the cell.
    uint32_t age{};

    void setState(State newState) { state = newState; age = 0; }
};

class Board : public rose::Widget {
protected:
    std::mutex boardLock{};
    rose::SignalSerialNumber mSignalSerialNumber{};

    std::unique_ptr<Cell[]>   board{};
    size_t mBoardWidth{}, mBoardHeight{}, mCellPixels{}, mArrayLen{}, mGeneration{};

    Cell& at(size_t x, size_t y) {
        size_t l = y * mBoardWidth + x;
        if (l < mArrayLen)
            return board.get()[l];
        else
            throw std::range_error("Board co-ordinates out of range.");
    }

    Cell& check(size_t x, size_t y) {
        if (x < mBoardWidth && y < mBoardHeight)
            return at(x,y);
        else
            throw std::range_error("Board co-ordinates out of range.");
    }

public:
    Board() = default;

    Board(size_t screenWidth, size_t screenHeight, size_t cellSize, size_t boardWidth, size_t boardHeight);

    ~Board() override = default;

    void initialize(StartPattern &startPattern);

    void reInitialize(StartPattern &startPattern);

    void initializeComposite() override;

    rose::Rectangle initialLayout(rose::sdl::Renderer &renderer, rose::Rectangle available) override {
        return rose::Rectangle{0, 0, (int)(mBoardWidth * mCellPixels), (int)(mBoardHeight * mCellPixels)};
    }

    void draw(rose::sdl::Renderer &renderer, rose::Rectangle parentRect) override;

    void generation();

    size_t countCell(int x, int y);

    std::shared_ptr<rose::Slot<uint32_t>> mTimerRx;

    rose::Signal<std::string> mGenCountTx;

    rose::Position getCenter() const { return rose::Position{(int)mBoardWidth / 2, (int)mBoardHeight / 2}; }
};

/**
 * Signal tokens used to identify which button was pressed.
 */
enum UserSignalTokenValues : rose::SignalToken {
    ExitButton = rose::SignalTokenValues::FirstUserSignalToken,         ///< Exit the application.
    GunButton,          ///< Restart with the Gosner Glider Gun pattern.
    AcornButton,        ///< Restart with the Acorn pattern.
    Switch1Button,      ///< Restart with the Switch1 pattern.
    Switch2Button,      ///< Restart with the Switch2 pattern.
    TributeButton,      ///< Conway's Tribute pattern.
};

class ConwayLife : public rose::Rose {
protected:

    rose::Timer mTimer;

    StartPattern& mStartPattern{GosperGliderGun};

    std::shared_ptr<rose::Button> mExit{},  ///< Exit the application
            mAcorn{},   ///< Restart with the Acorn pattern.
            mGun{},     ///< Restart with the Gosper Glider Gun pattern.
            mSwitch1{}, ///< Restart with the Switch1 pattern.
            mSwitch2{}, ///< Restart with the Switch2 pattern.
            mTribute{};

    std::shared_ptr<rose::Label> mGeneration{};
    std::shared_ptr<rose::Slider> mSlider{};

public:
    std::shared_ptr<rose::Slot<rose::Button::SignalType>> mButtonRx;
    std::shared_ptr<rose::Slot<rose::Slider::SignalType>> mRateRx;

    std::shared_ptr<Board> mBoard{};

    ConwayLife() = delete;

    ~ConwayLife() override = default;

    ConwayLife(int argc, char **argv, const std::string_view title) : rose::Rose(rose::Size{800, 480},
                                                                          argc, argv, title), mTimer(1000) {
        if (mCmdLineParser.cmdOptionExists("-rate")) {
            mTimer.setInterval(std::strtoul(mCmdLineParser.getCmdOption("-rate").c_str(), nullptr, 10));
        }

        if (mCmdLineParser.cmdOptionExists("-pattern")) {
            auto pattern = mCmdLineParser.getCmdOption("-pattern");
            if (pattern == "gun")
                mStartPattern = GosperGliderGun;
            else if (pattern == "acorn")
                mStartPattern = Acorn;
            else if (pattern == "switch1")
                mStartPattern = Switch1;
            else if (pattern == "switch2")
                mStartPattern = Switch2;
            else if (pattern == "tribute")
                mStartPattern = Tribute;
            else
                std::cout << "Available patterns are: gun, acorn, switch1, switch2\n";
        }
    }

    void build();

};
