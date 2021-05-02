/**
 * @file SatelliteModel.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-28
 */

#include "SatelliteModel.h"
#include "Utilities.h"

namespace rose {

    Ephemeris::Ephemeris(const std::filesystem::path& filePath) {
        readFile(filePath);
    }

    void Ephemeris::readFile(const std::filesystem::path &filePath) {
        clear();
        mEphemerisSet.clear();

        std::ifstream ephemFile(filePath.string());
        std::string str((std::istreambuf_iterator<char>(ephemFile)),std::istreambuf_iterator<char>());
        mEphemerisSet = str;

        auto first = mEphemerisSet.c_str();
        auto last = mEphemerisSet.c_str() + mEphemerisSet.length();

        auto ptr = first;
        while (ptr < last) {
            auto data = ptr;
            while (*ptr != '\n' && ptr < last) ++ptr;
            std::string_view name{data, static_cast<std::string_view::size_type>(ptr - data)};
            ++ptr;
            data = ptr;
            while (*ptr != '\n' && ptr < last) ++ptr;
            std::string_view l1{data, static_cast<std::string_view::size_type>(ptr - data)};
            ++ptr;
            data = ptr;
            while (*ptr != '\n' && ptr < last) ++ptr;
            std::string_view l2{data, static_cast<std::string_view::size_type>(ptr - data)};
            emplace(name, std::array<std::string_view,3>{name,l1,l2});
            ++ptr;
        }
    }

    SatelliteModel::SatelliteModel() {
        mEphemerisCache = std::make_unique<ClearSkyEphemeris>("http://clearskyinstitute.com/ham/HamClock/",
                                                              Environment::getEnvironment().cacheHome(),
                                                              "Ephemeris", std::chrono::hours{24},
                                                              CS_Ephem.begin(), CS_Ephem.end());

        mCacheLoaded = WebCacheProtocol::createSlot();
        mCacheLoaded->receiver = [&](uint32_t id,long status) {
            std::cout << __PRETTY_FUNCTION__ << ' ' << id << ' ' << status << '\n';
            if (id == 1) {
                auto path = mEphemerisCache->itemLocalPath(id);
                mEphemeris.readFile(path);
            }
        };
        mEphemerisCache->cacheLoaded.connect(mCacheLoaded);

        mEphemerisCache->fetchAll();
    }

    SatelliteObservation::SatelliteObservation(const Observer& observer) {
        mObserver = observer;
        const SatelliteModel &model{SatelliteModel::getModel()};

        for (auto &ephemeris : model) {
            mConstellation.emplace_back(ephemeris.second);
        }
    }

    SatelliteObservation::SatelliteObservation(const Observer &observer, const std::string &object) {
        mObserver = observer;
        const SatelliteModel& model{SatelliteModel::getModel()};

        for (auto& ephemeris : model) {
            if (ephemeris.first == object)
                mConstellation.emplace_back(ephemeris.second);
        }
    }

    void SatelliteObservation::predict(const DateTime &dateTime) {
        auto start = std::chrono::high_resolution_clock::now();
        std::for_each(mConstellation.begin(), mConstellation.end(), [&dateTime](Satellite &satellite) {
            satellite.predict(dateTime);
        });
        std::for_each(mConstellation.begin(), mConstellation.end(), [&](Satellite &satellite) {
            auto[altitude, azimuth, range, rate] = satellite.topo(mObserver);
        });
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << __PRETTY_FUNCTION__ << ' ' << duration.count() << '\n';
    }

    void SatelliteObservation::passPrediction(uint maxCount, const std::string &favorite) {
        auto start = std::chrono::high_resolution_clock::now();
        DateTime now{true};
        std::vector<SatellitePassData> passData{};

        // Initialize the pass prediction data
        std::transform(mConstellation.begin(), mConstellation.end(), std::back_inserter(passData),
                       [&](Satellite &satellite) -> SatellitePassData {
                           SatellitePassData pass;
                           pass.satellite = satellite;
                           pass.srchTime = now + -FINE_DT;
                           pass.satellite.predict(pass.srchTime);
                           pass.setTopo(mObserver);
                           pass.setGeo();
                           pass.periodDays = pass.satellite.period();
                           if (pass.altitude < SAT_MIN_EL) {
                               pass.srchTime += pass.deltaTime;
                           }
                           return pass;
                       });

        passData.erase(std::remove_if(passData.begin(), passData.end(), [&](SatellitePassData &pass) -> bool {
            return pass.satellite.getName() == "Moon";
        }), passData.end());

        bool search = std::any_of(passData.begin(), passData.end(),
                    [&](SatellitePassData &pass) -> bool { return pass.search(now); });
        while (search) {
            search = false;
            for (auto &pass : passData) {
                pass.satellite.predict(pass.srchTime);
                pass.setTopo(mObserver);
                pass.setGeo();
                pass.maxAltitude = std::max(pass.maxAltitude, pass.altitude);

                // check for rising or setting events
                if (pass.altitude >= SAT_MIN_EL) {
                    pass.everUp = true;
                    if (pass.prevAltitude < SAT_MIN_EL) {
                        if (pass.deltaTime == FINE_DT) {
                            // found a refined set event (recall we are going backwards),
                            // record and resume forward time.
                            pass.setTime = pass.srchTime;
                            pass.setAz = pass.azimuth;
                            pass.setOk = true;
                            pass.deltaTime = COARSE_DT;
                            pass.prevAltitude = pass.altitude;
                        } else if (!pass.riseOk) {
                            // found a coarse rise event, go back slower looking for better set
                            pass.deltaTime = FINE_DT;
                            pass.prevAltitude = pass.altitude;
                        }
                    }
                } else {
                    pass.everDown = true;
                    if (pass.prevAltitude > SAT_MIN_EL) {
                        if (pass.deltaTime == FINE_DT) {
                            // found a refined rise event (recall we are going backwards).
                            // record and resume forward time but skip if set is within COARSE_DT because we
                            // would jump over it and find the NEXT set.
                            DateTime check_set = pass.srchTime + COARSE_DT;
                            pass.satellite.predict(check_set);
                            auto[check_tel, check_taz, check_trange, check_trate] = pass.satellite.topo(mObserver);
                            if (check_tel >= SAT_MIN_EL) {
                                pass.riseTime = pass.srchTime;
                                pass.riseAz = pass.azimuth;
                                pass.riseOk = true;
                            }
                            // regardless, resume forward search
                            pass.deltaTime = COARSE_DT;
                            pass.prevAltitude = pass.altitude;
                        } else if (!pass.setOk) {
                            // found a coarse set event, go back slower looking for better rise
                            pass.deltaTime = FINE_DT;
                            pass.prevAltitude = pass.altitude;
                        }
                    }
                }
                pass.srchTime += pass.deltaTime;
                pass.prevAltitude = pass.altitude;

                search |= pass.search(now);
            }
        }

        std::cout << __PRETTY_FUNCTION__ << " Satellite count: " << passData.size() << '\n';

        passData.erase(std::remove_if(passData.begin(), passData.end(), [&](SatellitePassData &pass) -> bool {
            return !pass.goodPass(15.);
        }), passData.end());

        std::cout << __PRETTY_FUNCTION__ << " Good pass count: " << passData.size() << '\n';

        std::sort(passData.begin(), passData.end(), [](SatellitePassData &p0, SatellitePassData &p1){
            return p0.riseTime < p1.riseTime;
        });

        if (passData.size() > maxCount) {
            auto count = (favorite.empty() ? maxCount : maxCount - 1);
            passData.erase(std::remove_if(passData.begin() + count, passData.end(), [&favorite](SatellitePassData &pass) -> bool {
                return !(pass.satellite.getName() == favorite);
            }), passData.end());
        }

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << __PRETTY_FUNCTION__ << " Time: " << duration.count() << "ms\n";

        auto relative = time(nullptr);
        for (auto &pass : passData) {
            std::cout << pass.satellite.getName() << ": " << pass.passTimeString(relative) << '\n';
        }
    }

    std::string SatellitePassData::passTimeString(time_t relative) const {
        auto mkTimeStr = [](std::ostream &s, time_t t, time_t relative) {
            static constexpr size_t bufferLength = 64;
            static constexpr char fmtMinSec[] = "%M:%S";
            static constexpr char fmtHourMin[] = "%Hh%M";
            static constexpr char fmtDayHourMin[] = "%jd%Hh%M";
            static constexpr char fmtDate[] = "%F";

            time_t timer = t - relative;
            auto lt = localtime(&timer);
            timer += lt->tm_gmtoff;
            auto tm = gmtime(&timer);
            char buffer[bufferLength];
            char *fmt = const_cast<char *>(fmtMinSec);
            if (timer >= 172800)
                fmt = const_cast<char *>(fmtDate);
            if (timer >= 86400)
                fmt = const_cast<char *>(fmtDayHourMin);
            else if (timer >= 3600)
                fmt = const_cast<char *>(fmtHourMin);
            auto length = strftime(buffer, bufferLength, fmt, tm);
            s << buffer;
        };

        DateTime now{true};
        if (riseOk && riseTime > now) {
            std::stringstream strm{};
            auto rise = riseTime.mktime();
            mkTimeStr(strm, rise, relative);
            strm << " - ";
            if (setOk) {
                mkTimeStr(strm, setTime.mktime(), relative ? rise : 0);
            }
            return strm.str();
        } else if (setOk && setTime > now) {
            std::stringstream strm{};
            mkTimeStr(strm, setTime.mktime(), relative);
            return strm.str();
        }

        return "Set.";
    }
}
