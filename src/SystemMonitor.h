/** @file SystemMonitor.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-10-26
  * @brief A class to display system related information.
  */

#pragma once

#include "Label.h"
#include "Signals.h"
#include "Timer.h"

namespace rose {

    /**
     * @class SystemMonitor
     * @brief A widget to display some system values to be monitored.
     */
    class SystemMonitor : public Row {
    protected:
        /**
         * @brief Read the CPU temperature from the system.
         */
        void readCPUTemperature();

        /**
         * @brief Read processor usage and estimate this process' contribution.
         */
        void readProcessUsage();

        /**
         * @brief Count the number of CPUs in the system.
         */
        void cpuCount();

        color::RGBA mCPUWarning;                ///< CPU temperature warning color.
        color::RGBA mCPUAlert;                  ///< CPU temperature alert color.
        bool mHasTemperatureDevice{true};       ///< True if temperature data is found.
        std::shared_ptr<Label> mTemperature;    ///< The Label to display temperature.
        std::shared_ptr<Label> mUsage;          ///< The Label to display usage.
        int mCpuCount{0};                       ///< The number of CPUs in the system.
        int cpuTimeUse{},                       ///< The current CPU time use
            cpuTimeStart{0};                    ///< The CPU time use at the start of this interval
        int procTimeUse{},                      ///< The current process time use
            procTimeStart{0};                   ///< The process time use at teh start of this interval

        SecondTickPtr mSecondTick{};            ///< The source of timing data.

    public:
        static constexpr std::string_view ProcSelfStat = "/proc/self/stat";     ///< The process stats path
        static constexpr std::string_view ProcStat = "/proc/stat";              ///< The system stats apth
        static constexpr std::string_view ProcCpuInfo = "/proc/cpuinfo";        ///< The CPU info path

        void initializeComposite() override;    ///< See Widget::initializeComposite().

        std::shared_ptr<Slot<int>> rxTrigger;   ///< The slot to receive interval triggers on.

        SystemMonitor() = delete;               ///< Deleted default constructor.

        ~SystemMonitor() override = default;    ///< Default destructor.

        /**
         * Construct a SystemMonitor Widget.
         * @param secondTick The source of timing data.
         */
        explicit SystemMonitor(SecondTickPtr secondTick);
    };
}



