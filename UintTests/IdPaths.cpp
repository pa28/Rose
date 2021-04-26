//
// Created by richard on 2021-04-25.
//

#include <array>
#include <iostream>
#include <iomanip>
#include "StructuredTypes.h"

static std::vector<std::string_view> SampleAbsolutePaths{
        "/Screen/Window/Manager/MapProjection",
        "/Screen/PopupWindow/Frame/Column/Row/TextButton",
        "/Screen/PopupWindow/Frame/Column/Row/TextButton:Close",
        "/Screen/PopupWindow/Frame/Column/Row/:Close"
};

static std::vector<std::string_view> SampleRelativePaths{
        "*/TextButton",
        "?/TextButton",
        "*/TextButton:Close",
        "?/:Close"
};

static std::vector<std::string_view> SampleMatchTargets{
        "/Screen/PopupWindow/Frame/Column/Row/TextButton",
        "/Screen/PopupWindow/Frame/Column/Row/TextButton:Close",
        "/Screen/Window/Manager/Row/Column/TextButton",
        "/Screen/Window/Manager/Row/Column/TextButton:Callsign"
};

static std::vector<std::string_view> SampleSearchPaths{
        "*/PopupWindow/*/TextButton",
        "*/PopupWindow/*/Close",
        "*/Callsign"
};

static std::vector<int> SearchResults {
    0, 1, 3
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
        for (auto &path : testData) {
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

struct Search : Test {
    const std::vector<std::string_view> &searchTargets;
    const std::vector<std::string_view> &searchPaths;
    const std::vector<int> &answers;

    explicit Search(const std::string name, const std::vector<std::string_view> &targets,
                    const std::vector<std::string_view> &paths,
                    const std::vector<int> &searchRes) : searchTargets(targets), searchPaths(paths), answers(searchRes) {
        testName = name;
    }

    void performTest() override {
        auto answerIdx = answers.begin();
        for (auto &path : searchPaths) {
            int selected = -1;
            int maxScore = 0;
            auto idPath = rose::parsePathIdString(path);
            int n = 0;
            for (auto &target : searchTargets) {
                auto tgtPath = rose::parsePathIdString(target);
                auto [match,score] = idPath.compare(tgtPath);
                if (match) {
                    if (selected < 0 || score > maxScore) {
                        selected = n;
                        maxScore = score;
                    }
                }
                ++n;
            }
            if (selected == (*answerIdx)) {
                passCount++;
            } else {
                std::cerr << std::setw(12) << std::left << testName << "Test: " << testCount << " Failed.\n";
            }
            testCount++;
            ++answerIdx;
        }
    }
};

static std::vector<std::shared_ptr<Test>> TestList{
        std::make_shared<Parsing>("AbsPaths", SampleAbsolutePaths),
        std::make_shared<Parsing>("RelPaths", SampleRelativePaths),
        std::make_shared<Parsing>("MatchTgt", SampleMatchTargets),
        std::make_shared<Parsing>("SearchPaths", SampleSearchPaths),
        std::make_shared<Search>("Matching", SampleMatchTargets, SampleSearchPaths, SearchResults)
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
