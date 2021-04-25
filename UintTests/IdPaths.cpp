//
// Created by richard on 2021-04-25.
//

#include <array>
#include <iostream>
#include <iomanip>
#include "StructuredTypes.h"

static std::vector<std::string_view> SampleAbsolutePaths{
        "/Screen/Window/Manager/MapProjection",
        "/Screen/PopupWindow/Frame/Column/Row/TextButton"
};

static std::vector<std::string_view> SampleRelativePaths{
        "*/TextButton",
        "?/TextButton"
};

struct Test {
    size_t testCount{0};
    size_t passCount{0};
    std::string testName{};

    virtual void performTest() {}

    void operator()() {
        performTest();
    }
};

struct Parsing : Test {
    const std::vector<std::string_view> &testData;

    explicit Parsing(const std::string name, const std::vector<std::string_view> &data) : testData(data) {
        testName = name;
    }

    void performTest() override {
        for (auto &path : SampleAbsolutePaths) {
            auto idPath = rose::parsePathIdString(path);
            auto pathStr = idPath.str();
            if (path == pathStr) {
                ++passCount;
            } else {
                std::cerr << std::setw(12) << std::left << testName
                          << "Test " << std::setw(3) << testCount
                          << " FAILED\n"
                          << std::setw(24) << std::right << "Expected: " << path << '\n'
                          << std::setw(24) << std::right << "Got: " << pathStr << '\n';
            }
            ++testCount;
        }
    }
};

static std::vector<std::shared_ptr<Test>> TestList{
        std::make_shared<Parsing>("AbsPaths", SampleAbsolutePaths),
        std::make_shared<Parsing>("RelPaths", SampleRelativePaths)
};

int main(int argc, char **argv) {
    size_t totalTests = 0;
    size_t totalPasses = 0;
    for (auto &test : TestList) {
        test->performTest();
        std::cout << std::setw(12) << std::left << test->testName
                  << "  Tests: " << std::setw(4) << test->testCount
                  << " Passed: " << std::setw(4) << test->passCount << '\n';
        totalPasses += test->passCount;
        totalTests += test->testCount;
    }

    std::cout << "Total Tests: " << std::right << std::setw(5) << totalTests
              << "\nTotal Passed: " << std::setw(4) << totalPasses
              << "\nTotal Failed: " << std::setw(4) << totalTests - totalPasses;

    return totalPasses == totalTests ? 0 : 1;
}
