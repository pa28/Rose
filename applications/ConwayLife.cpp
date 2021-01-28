//
// Created by richard on 2020-12-27.
//

#include <iomanip>
#include <memory>

#include "Button.h"
#include "Color.h"
#include "ConwayLife.h"
#include "PopupWindow.h"
#include "Manipulators.h"
#include "ScreenMetrics.h"
#include "ScrollArea.h"
#include "Slider.h"

using namespace rose;

void ConwayLife::build() {
    mTranslateFingerEvents = true;

    mButtonRx = std::make_shared<rose::Slot<Button::SignalType>>();
    mButtonRx->setCallback([=](uint32_t, Button::SignalType status){
        if (status.first) {
            switch (status.second) {
                case ExitButton:
                    createPopup<ExitDialog>() << mButtonRx;
                    needsLayout();
                    break;
                case ExitDialogOk:
                    mRunEventLoop = false;
                    break;
                case ExitDialogCancel:
                    break;
                case GunButton:
                    mBoard->reInitialize(GosperGliderGun);
                    break;
                case AcornButton:
                    mBoard->reInitialize(Acorn);
                    break;
                case Switch1Button:
                    mBoard->reInitialize(Switch1);
                    break;
                case Switch2Button:
                    mBoard->reInitialize(Switch2);
                    break;
                case TributeButton:
                    mBoard->reInitialize(Tribute);
                    break;
            }

            if (status.second != ExitButton)
                needsLayout();
        }
    });

    mRateRx = std::make_shared<rose::Slot<Slider::SignalType>>();
    mRateRx->setCallback([=](uint32_t, Slider::SignalType rate){
        mTimer.setInterval((Uint32)((1. - rate.first) * 900) + 100);
    });

    createRoundCorners(mRenderer, 5, 10, 2,
                       Theme::dTopColor, Theme::dBotColor, Theme::dLeftColor, Theme::dRightColor);

    createSquareCorners(mRenderer, 10, 2,
                        Theme::dTopColor, Theme::dBotColor, Theme::dLeftColor, Theme::dRightColor);

    createStandardIcons();
    createCenters(mRenderer, 5, 10);

    auto mainWindow = createWindow() << BackgroundColor(mTheme.mBaseColor);
    auto row = mainWindow << wdg<Row>();

    auto column = row << wdg<Column>();
    mExit = column << wdg<Button>("Exit") << ExitButton << Elastic(true);
    mGun = column << wdg<Button>("Gun") << GunButton << Elastic(true);
    mAcorn = column << wdg<Button>("Acorn") << AcornButton << Elastic(true);
    mSwitch1 = column << wdg<Button>("Switch1") << Switch1Button << Elastic(true);
    mSwitch2 = column << wdg<Button>("Switch2") << Switch2Button << Elastic(true);
    mTribute = column << wdg<Button>("Tribute") << TributeButton << Elastic(true);
    mGeneration = column << wdg<Label>("") << FontSize{20} << FontName{theme().mTimeBoxTimeFont};
    mSlider = column << wdg<Slider>() << Orientation{rose::Orientation::Horizontal} << BorderStyle::Notch;

    mBoard = row << wdg<ScrollArea>() << Size{mWidth,mHeight}
                    << wdg<Board>();

    mTimer.timerTick.connect(mBoard->mTimerRx);
    mBoard->initialize(mStartPattern);

    mExit->txPushed.connect(mButtonRx);
    mGun->txPushed.connect(mButtonRx);
    mAcorn->txPushed.connect(mButtonRx);
    mSwitch1->txPushed.connect(mButtonRx);
    mSwitch2->txPushed.connect(mButtonRx);
    mBoard->mGenCountTx.connect(mGeneration->mTextRx);
    mSlider->valueTx.connect(mRateRx);
    mTribute->txPushed.connect(mButtonRx);

    mSlider->setValue(0.f, true);
//    mSlider->finalValue(0);
}

Board::Board(size_t screenWidth, size_t screenHeight, size_t cellSize, size_t width, size_t height) : Board() {
    mBoardWidth = width;
    mBoardHeight = height;
    mCellPixels = cellSize;
    mArrayLen = mBoardHeight * mBoardWidth;
    board = std::make_unique<Cell[]>(mArrayLen);
}

void Board::initialize(StartPattern &startPattern) {
    std::lock_guard<mutex> lockGuard{boardLock};
    std::stringstream data(std::string{startPattern.data});
    std::string row;
    int xOffset = 0;
    int yOffset = 0;
    mBoardWidth = startPattern.boardSize.width();
    mBoardHeight = startPattern.boardSize.height();
    mCellPixels = startPattern.pixelSize;

    mPos = Position::Zero;
    mSize = Size{(int)(mBoardWidth * mCellPixels), (int)(mBoardHeight * mCellPixels)};

    mArrayLen = mBoardWidth * mBoardHeight;
    board = std::make_unique<Cell[]>(mArrayLen);

    switch (startPattern.origin) {
        case TopLeft:
            break;
        case Center:
            xOffset = (int)mBoardWidth / 2;
            yOffset = (int)mBoardHeight / 2;
    }

    xOffset += startPattern.offset.x();
    yOffset += startPattern.offset.y();

    auto xScrollOffset = -xOffset * (int)mCellPixels;
    auto yScrollOffset = -yOffset * (int)mCellPixels;

    xScrollOffset += xScrollOffset ? 400 : 0;
    yScrollOffset += yScrollOffset ? 240 : 0;

    while (std::getline(data, row, ';')) {
        std::stringstream rowData{row};
        std::string value;
        std::getline(rowData, value, ':');
        auto y = strtol(value.c_str(), nullptr, 10);
        long x;
        while (rowData) {
            rowData >> x;
            if (rowData)
                at(x + xOffset, y + yOffset) = Cell{Live,0};
        }
    }

    if (auto scrollArea = parent<ScrollArea>(); scrollArea)
        scrollArea->setScrollOffset(Position{xScrollOffset,yScrollOffset});

    mGeneration = 0;
}

void Board::reInitialize(StartPattern &startPattern) {
    initialize(startPattern);
}

void Board::initializeComposite() {
    mTimerRx = std::make_shared<rose::Slot<uint32_t>>();
    mTimerRx->setCallback([=](uint32_t, int count){
        generation();
        std::stringstream genstrm{};
        genstrm << "Gen " << std::setw(6) << mGeneration;
        mGenCountTx.transmit(mSignalSerialNumber.serialNumber(), genstrm.str());
        setNeedsDrawing();
    });
    setSize(Size{(int)mBoardWidth * (int)mCellPixels, (int)mBoardHeight * (int)mCellPixels});
    mClassName = "Board";
}

void Board::draw(sdl::Renderer &renderer, Rectangle parentRect) {
    auto bordRect = clampAvailableArea(parentRect, mPos, mSize);
    Widget::draw(renderer, parentRect);
    auto screenPos = bordRect.getPosition();

    Rectangle screen{screenPos, mSize.value()};
    std::lock_guard<mutex> lockGuard{boardLock};
    for (size_t x = 0; x < mBoardWidth; ++x) {
        for (size_t y = 0; y < mBoardHeight; ++y) {
            color::RGBA cellColor;
            Rectangle cellRect{(int)x * (int)mCellPixels + screenPos.x(), (int)y * (int)mCellPixels + screenPos.y(),
                                  (int)mCellPixels, (int)mCellPixels};
            if (cellRect.overlap(screen))
                switch (at(x,y).state) {
                    case Live:
                        cellColor = color::RGBA{0u, 0u, 128u, 255u};
                        renderer.fillRect( cellRect, cellColor);
                        break;
                    case Dying:
                        cellColor = color::RGBA{128u, 0u, 0u, 255u};
                        renderer.fillRect( cellRect, cellColor);
                        break;
                    case Born:
                        cellColor = color::RGBA{0u, 128u, 0u, 255u};
                        renderer.fillRect( cellRect, cellColor);
                        break;
                    case Dead:
                        break;
                }
        }
    }
}

size_t Board::countCell(int x, int y) {
    size_t count = 0;
    for (int cx = x-1; cx < x+2; ++cx) {
        for (int cy = y-1; cy < y+2; ++cy) {
            if (cx >= 0 && cy >= 0 && cx < mBoardWidth && cy < mBoardHeight && !(cx == x && cy == y)) {
                auto cell = check(cx,cy);
                if (cell.state == Live || cell.state == Dying)
                    count++;
            }
        }
    }
    return count;
}

void Board::generation() {
    std::lock_guard<mutex> lockGuard{boardLock};
    for (size_t x = 0; x < mBoardWidth; ++x)
        for (size_t y = 0; y < mBoardHeight; ++y)
            switch (at(x, y).state) {
                case Dying:
                    at(x, y).setState(Dead);
                    break;
                case Born:
                    at(x, y).setState(Live);
                    break;
                default:
                    at(x, y).age++;
                    break;
            }

    for (size_t x = 0; x < mBoardWidth; ++x)
        for (size_t y = 0; y < mBoardHeight; ++y) {
            auto cell = at(x, y);
            auto count = countCell(x, y);
            if (cell.state == Live && (count < 2 || count > 3)) {
                at(x,y).setState(Dying);
            } else if (cell.state == Dead && count == 3) {
                at(x,y).setState(Born);
            } else if (cell.state == Born || cell.state == Dying){
                std::cout << "Incorrect cell state: " << x << ',' << y << "\n";
            }
        }

    ++mGeneration;
}

int main(int argc, char **argv) {

    auto app = std::make_shared<ConwayLife>(argc,argv,"Conway's Game of Life");

    app->build();
    app->initialLayout(app->getRenderer());
//    app.mBoard->generation();

    if (app) {
        app->eventLoop();
    }

    return static_cast<int>(app->getErrorCode());
}

