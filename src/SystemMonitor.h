/** @file SystemMonitor.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 2.0
  * @date 2021-02-10
  * @brief A class to display system related information.
  */

#pragma once

#include "Label.h"
#include "Signals.h"
#include "Timer.h"

namespace rose {

    enum CpuTimeIndex : std::size_t {
        USER = 0, NICE, SYSTEM, IDLE, IOWAIT, IRQ, SOFTIRQ, STEAL, GUEST, GUEST_NICE, ARRAY_SIZE
    };

    /**
     * @class SystemData
     * @brief Gather system data to transmit on Signal objects.
     */
    class SystemData {
    protected:
        SignalSerialNumber mSignalSerialNumber{};    /// Signal serial number identifying the object.

        bool mHasTemperatureDevice{true};       ///< True if temperature data is found.

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

        std::array<uint64_t,CpuTimeIndex::ARRAY_SIZE> mCpuTime{};
        std::array<uint64_t,CpuTimeIndex::ARRAY_SIZE> mPastCpuTime{};
        std::array<uint64_t,CpuTimeIndex::ARRAY_SIZE> mProcTime{};
        std::array<uint64_t,CpuTimeIndex::ARRAY_SIZE> mPastProcTime{};

        int mCpuCount{0};        ///< The number of CPUs in the system.
        float mCpuTimeUse{};     ///< The current CPU time use
        float mCpuTotalTime{};   ///< The total CPU time available.
        int mProcTimeUse{};      ///< The current process time use
        int mProcTimeStart{};    ///< The process time use at interval start

        float mUsage{0.f};           ///< The process usage as percent of a CPU.
        float mTemperature{0.f};     ///< The cpu temperature.

        static constexpr std::string_view ProcSelfStat = "/proc/self/stat";     ///< The process stats path
        static constexpr std::string_view ProcStat = "/proc/stat";              ///< The system stats apth
        static constexpr std::string_view ProcCpuInfo = "/proc/cpuinfo";        ///< The CPU info path

    public:
        SystemData();
        ~SystemData() = default;

        std::shared_ptr<Slot<int>> rxTrigger;   ///< The slot to receive interval triggers on.

        using TemperatureSignal = std::array<float,3>;  ///< The type used for system temperature signal.
        Signal<TemperatureSignal> txTemperature{};      ///< The Signal object to transmit temperature.

        using ProcessSignal = std::array<float,3>;      ///< The type used for process usage signal.
        Signal<ProcessSignal> txProcess{};              ///< The Signal object to transmit process usage.

        using SystemSignal = std::array<float,3>;       ///< The type used for the system usage signal.
        Signal<SystemSignal> txSystem{};
    };

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



