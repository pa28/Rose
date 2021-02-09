//
// Created by richard on 2020-11-12.
//

#include "Cache.h"

#include <fstream>
#include <utility>
#include <iostream>
#include <iomanip>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>
#include "Utilities.h"

namespace rose {
    CacheSource::CacheSource() {
        mCacheValidDuration = std::chrono::hours{1};
    }

    CacheFileSystem::CacheFileSystem(std::filesystem::path rootPath) : mRootPath(std::move(rootPath)) {
        std::filesystem::create_directories(mRootPath, mErrorCode);
    }

    CacheError CacheFileSystem::create(const CacheObject &object) {
        return CacheErrorWrite;
    }

    bool CacheFileSystem::find(const CacheObject &object) {
        auto objectPath = mRootPath;
        objectPath.append(object.objectSrcName());
        auto res = std::filesystem::exists(objectPath, mErrorCode);
        if (mErrorCode)
            return false;
        return res;
    }

    std::ofstream CacheFileSystem::openWrite(const CacheObject &object, bool temp) {
        auto objectPath = mRootPath;
        if (temp)
            objectPath.append("." + object.objectSrcName());
        else
            objectPath.append(object.objectSrcName());
        std::ofstream ostrm{objectPath, std::ios_base::out};
        return ostrm;
    }

    std::ifstream CacheFileSystem::openRead(const CacheObject &object) {
        auto objectPath = mRootPath.append(object.objectSrcName());
        std::ifstream istrm{objectPath, std::ios_base::in};
        return istrm;
    }

    std::optional<std::string> CacheFileSystem::fileSystemName(const CacheObject &object) const {
        return mRootPath.string();
    }

    void CacheFileSystem::moveFromTemp(const CacheObject &object) {
        auto objectTempPath = mRootPath;
        objectTempPath.append("." + object.objectSrcName());
        auto objectPath = mRootPath;
        objectPath.append(object.objectSrcName());
        std::filesystem::rename(objectTempPath, objectPath, mErrorCode);
    }

    void CacheFileSystem::removeTemp(const CacheObject &object) {
        auto objectPath = mRootPath;
        objectPath.append("." + object.objectSrcName());
        std::filesystem::remove(objectPath, mErrorCode);
    }

    void CacheFileSystem::setCacheTime(const CacheObject &object,
                                       std::chrono::system_clock::duration cacheTimeInc) {
        auto objectPath = mRootPath;
        objectPath.append(object.objectSrcName());
        auto ftime = std::filesystem::last_write_time(objectPath);
        if (cacheTimeInc.count() == 0) {
            auto cftime = decltype(ftime)::clock::now();
            std::filesystem::last_write_time(objectPath, cftime);
        } else {
            std::filesystem::last_write_time(objectPath, ftime + cacheTimeInc);
        }
    }

    CacheWebSource::CacheWebSource(std::string sourceURI) : mSourceURI(std::move(sourceURI)) {
    }

    CacheWebSource::CacheWebSource(std::string sourceURI, std::chrono::system_clock::duration cacheValidityDuration)
            : CacheWebSource(std::move(sourceURI)) {
        mCacheValidDuration = cacheValidityDuration;
    }

    void CacheWebSource::fetch(CacheObject &cacheObject, std::ostream &ostrm, time_t cacheTime) {
        std::list<std::string> headers;

        try {
            if (cacheTime) {
                std::stringstream ss;
                ss << "If-Modified-Since: " << std::put_time(std::gmtime(&cacheTime), "%a, %d %b %Y %T %Z");
                headers.push_back(ss.str());
            }

            curlpp::options::Url url(mSourceURI + cacheObject.objectSrcName());
            curlpp::options::WriteStream ws(&ostrm);
            curlpp::Easy request;
            request.setOpt(new curlpp::options::HttpHeader(headers));
            request.setOpt(url);
            request.setOpt(ws);
            request.perform();
            mResponseCode = curlpp::infos::ResponseCode::get(request);
            cacheObject.setStatusCode(mResponseCode);
        } catch (const cURLpp::RuntimeError &e) {
            std::cerr << __PRETTY_FUNCTION__ << ' ' << e.what() << '\n';
            mResponseCode = 599;
        } catch (const cURLpp::LogicError &e) {
            std::cerr << __PRETTY_FUNCTION__ << ' ' << e.what() << '\n';
            mResponseCode = 599;
        }
    }

    WebFileCache::WebFileCache() {
        mCheckFutures = std::make_shared<Slot<int>>();
        mCheckFutures->setCallback([=](uint32_t, int) {
            checkFutures();
        });

        mCheckValidity = std::make_shared<Slot<int>>();
        mCheckValidity->setCallback([=](uint32_t, int) {
            fetchAll();
        });
    }

    WebFileCache::WebFileCache(const std::string &sourceURI, const std::filesystem::path &rootPath,
                               const std::string &cacheName) : WebFileCache() {
        source = std::make_unique<CacheWebSource>(sourceURI);
        localStore = std::make_unique<CacheFileSystem>(rootPath, cacheName);
    }

    WebFileCache::WebFileCache(const std::string &sourceURI, const std::filesystem::path &rootPath,
                               const std::string &cacheName,
                               std::chrono::system_clock::duration cacheValidityDuration) : WebFileCache() {
        source = std::make_unique<CacheWebSource>(sourceURI, cacheValidityDuration);
        localStore = std::make_unique<CacheFileSystem>(rootPath, cacheName);
    }

    std::optional<uint32_t> WebFileCache::asyncFetch(WebFileCache *self, uint32_t id, time_t cacheTime) {
        auto &cacheObject{self->find(id)->second};

        auto ostrm = self->localStore->openWrite(cacheObject, true);
        if (ostrm) {
            self->source->fetch(cacheObject, ostrm, cacheTime);
            ostrm.close();
            auto response = dynamic_cast<CacheWebSource *>(self->source.get())->responseCode();
            cacheObject.setStatusCode(response);
            if (response == 200) {
                self->localStore->moveFromTemp(cacheObject);
            } else if (response == 304) {
                self->localStore->setCacheTime(cacheObject, std::chrono::system_clock::duration() / 10);
            } else {
                self->localStore->removeTemp(cacheObject);
            }
        } else {
            std::cerr << "Can not write to cache \"" << cacheObject.objectSrcName() << "\"\n";
            return std::nullopt;
        }

        return id;
    }

    void WebFileCache::fetchAll() {
        std::lock_guard<std::mutex> lockGuard(mMutex);

        for (auto &object : *this) {
            bool found = false;
            time_t cacheTime = 0;

            if (localStore->find(object.second)) {
                found = true;
                auto path = dynamic_cast<CacheFileSystem *>(localStore.get())->rootPath();
                path.append(object.second.objectSrcName());
                auto cacheFiletime = util::fileClockToSystemClock(std::filesystem::last_write_time(path));
                auto cacheFileAge = std::chrono::system_clock::now() - cacheFiletime;
                if (cacheFileAge > source->cacheValidDuration())
                    cacheTime = std::chrono::system_clock::to_time_t(cacheFiletime);
            }

            if (!found || cacheTime > 0) {
                auto fut = std::async(std::launch::async, &WebFileCache::asyncFetch, this, object.first, cacheTime);
                mFutureList.emplace_back(std::move(fut));
            } else {
                if (!object.second.getFirstProcess()) {
                    itemFetched.transmit(mSignalSerialNumber(), object.first);
                    object.second.setFirstProcess();
                }
            }
        }
    }

    void WebFileCache::connect(Signal<int> &futureCheck, Signal<int> &validityCheck) {
        futureCheck.connect(mCheckFutures);
        validityCheck.connect(mCheckValidity);
    }

    void WebFileCache::checkFutures() {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        if (!mFutureList.empty()) {
            for (auto &fut : mFutureList) {
                if (fut.valid()) {
                    auto result = fut.get();
                    if (result) {
                        auto& cacheObject{find(result.value())->second};
                        if (cacheObject.getStatusCode() == 200 ||
                                (cacheObject.getStatusCode() == 304 && !cacheObject.getFirstProcess())) {
                            itemFetched.transmit(mSignalSerialNumber(), result.value());
                            cacheObject.setFirstProcess();
                        }
                    } else {
                        std::cerr << "Future failed.\n";
                    }
                }
            }
        }
    }
}