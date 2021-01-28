//
// Created by richard on 2020-10-26.
//

#include "SystemMonitor.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include "Color.h"
#include "Math.h"

#ifdef BCMHOST
static constexpr std::string_view SystemTempDevice = "/sys/class/thermal/thermal_zone0/temp";
#else
static constexpr std::string_view SystemTempDevice = "/sys/class/thermal/thermal_zone2/temp";
#endif

namespace rose {
    SystemMonitor::SystemMonitor(SecondTickPtr secondTick)
            : Row(),
              mSecondTick(secondTick) {
    }

    void SystemMonitor::initializeComposite() {
        mTemperature = add<Label>()->withFontSize(15);
        mUsage = add<Label>()->withFontSize(15);
        mUsage->setText("000.0%");

        rxTrigger = std::make_shared<Slot<int>>();
        rxTrigger->setCallback([&](uint32_t sn, int) {
            readCPUTemperature();
            readProcessUsage();
        });

        cpuCount();
        readCPUTemperature();
        readProcessUsage();

        mSecondTick->txSecond.connect(rxTrigger);
    }

    void SystemMonitor::readCPUTemperature() {
        static int divisor{4};
        static int count{0};

        mCPUWarning = color::RGBA(255u, 255u, 0u, 255u);
        mCPUAlert = color::RGBA(255u, 0u, 0u, 255u);


        if (mHasTemperatureDevice) {
            if (count == 0) {
                std::ifstream ifs;
                ifs.open(std::string(SystemTempDevice), std::ofstream::in);
                if (ifs) {
                    int temperature;
                    std::stringstream sstrm;
                    ifs >> temperature;
                    ifs.close();
                    sstrm << roundToInt((float) temperature / 1000.) << "C ";
                    mTemperature->setText(sstrm.str());
                } else {
                    // TODO: Better error reporting.
                    mHasTemperatureDevice = false;
                    std::cerr << "Can not open " << SystemTempDevice << std::endl;
                }
            }
        }
    }

    void SystemMonitor::readProcessUsage() {
        static int divisor{4};
        static int count{1};

        if (count == 0) {
            std::ifstream ifs;
            ifs.open(std::string(ProcSelfStat), std::ofstream::in);
            int utime, stime;
            if (ifs) {
                std::string not_needed;
                for (int i = 0; i < 13; ++i) {
                    getline(ifs, not_needed, ' ');
                }
                ifs >> utime >> stime;
                ifs.close();
                if (procTimeStart)
                    procTimeUse = (utime + stime) - procTimeStart;
                procTimeStart = utime + stime;
            }

            ifs.open(std::string(ProcStat), std::ofstream::in);
            if (ifs) {
                std::string cpu;
                getline(ifs, cpu, '\n');
                ifs.close();
                std::stringstream sstrm(cpu);
                getline(sstrm, cpu, ' ');
                int total{0};
                while (sstrm) {
                    int value;
                    sstrm >> value;
                    total += value;
                }
                if (cpuTimeStart) {
                    cpuTimeUse = total - cpuTimeStart;
                }
                cpuTimeStart = total;

            }
            if (cpuTimeUse) {
                std::stringstream sstrm;
                sstrm << " " << std::fixed << std::setw(5) << std::setprecision(1)
                      << mCpuCount * ((100. * (float) procTimeUse) / (float) cpuTimeUse) << '%';
                mUsage->setText(sstrm.str());
            }
        }

        count = (count + 1) % divisor;
    }

    void SystemMonitor::cpuCount() {
        static constexpr std::string_view ProcessorLine = "processor";
        std::ifstream ifs;
        ifs.open(std::string(ProcCpuInfo), std::ofstream::in);
        if (ifs) {
            std::string line;
            while (getline(ifs, line, '\n')) {
                if (line.substr(0, ProcessorLine.size()) == ProcessorLine) {
                    line = line.substr(line.find_last_of(' '));
                    std::stringstream sstrm{line.substr(line.find_last_of(' '))};
                    sstrm >> mCpuCount;
                }
            }
        }

        ++mCpuCount;
#ifdef X86HOST
        mCpuCount /= 2;
#endif
    }
}
