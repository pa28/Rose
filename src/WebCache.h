/**
 * @file WebCache.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-27
 * @brief Fetching and caching web resources.
 */

#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <filesystem>
#include <map>
#include <mutex>
#include <optional>
#include <fstream>
#include <list>
#include <utility>
#include <future>
#include <vector>
#include "CommonSignals.h"

namespace rose {

    struct WebCacheItem {
        uint32_t key;
        std::string_view name;
    };

    using WebCacheProtocol = Protocol<uint32_t,long>;

    using namespace std::filesystem;

    /**
     * @class WebCache
     * @brief Fetch web resources caching them in the local filesystem following XDG specifications.
     */
    class WebCache {
        GraphicsModelFrameProtocol::slot_type mFrameProtocol{};

    public:
        using key_t = uint32_t;
        using local_id_t = std::string;
        using result_t = std::tuple<long, key_t>;
        using item_map_t = std::map<key_t, local_id_t>;

        using AsyncList = std::vector<std::future<result_t>>;

        /**
         * @brief Convert a filesystem time to a system clock time point.
         * @tparam T The type of the file time point.
         * @param fileTimePoint The value of the file time point.
         * @return std::chrono::system_clock::time_point.
         */
        template<typename T>
        static std::chrono::system_clock::time_point fileClockToSystemClock(T fileTimePoint) {
            using namespace std::chrono_literals;
            using namespace std::chrono;

            auto sysWriteTime = time_point_cast<system_clock::duration>(
                    fileTimePoint - decltype(fileTimePoint)::clock::now() +
                    system_clock::now());
            return sysWriteTime;
        }

        /**
         * @brief Convert a filesystem time to a system clock time point.
         * @param filePath The filesystem path to get the last write time from.
         * @return std::chrono::system_clock::time_point.
         */
        static std::chrono::system_clock::time_point fileClockToSystemClock(const path &filePath) {
            return fileClockToSystemClock(std::filesystem::last_write_time(filePath));
        }

        WebCacheProtocol::signal_type cacheLoaded{};

    protected:
        std::error_code mEc{};          ///< The last error code returned from a std::filesystem operation.
        std::string mRootURI{};         ///< The root URI for items in the cache.
        path mStoreRoot{};              ///< The root of the filesystem backing store.
        mutable file_status mStoreStatus{};     ///< File status of the filesystem backing store root.
        std::mutex mMutex;              ///< Mutex for locking the WebFileCache

        item_map_t mItemMap{};          ///< The map of keys to local cache items.

        AsyncList mAsyncList{};         ///< The list of asynchronous fetches active.

        /// The duration local files are considered valid. The interval between cache refresh checks.
        std::chrono::system_clock::duration mCacheValidDuration{};

        void asyncFetchItem(item_map_t::value_type& item) {
            auto itemPath = mStoreRoot;
            auto tempPath = mStoreRoot;
            auto itemName = translateItemLocalId(item.second);
            itemPath.append(itemName);
            tempPath.append('.' + itemName);
            if (!itemPath.empty()) {
                std::optional<time_t> cacheFileTime{};
                if (exists(itemPath))
                    if (cacheFileTime = cacheTime(itemPath); !cacheFileTime) {
                        cacheLoaded.transmit(item.first, 503);
                        return;
                    }

                mAsyncList.emplace_back(
                        std::async(std::launch::async, fetch, item.first, constructUrl(item.second), itemPath, tempPath,
                                   cacheFileTime));
                CommonSignals::getCommonSignals().frameSignal.connect(mFrameProtocol);
            }
        }

    public:
        WebCache() = delete;

        virtual ~WebCache() = default;

        /**
         * @brief Constructor
         * @param rootUri The base URI locating all the items manage by the cache.
         * @param xdgDir The XDG compliant specification for the user caching directory. See rose::Environment.
         * @param storeRoot A relative path from xdgDir that this cache will use.
         * @param duration The period of time cache items should be considered valid before being reloaded.
         */
        WebCache(const std::string &rootUri, const path &xdgDir, const std::string &storeRoot,
                 std::chrono::system_clock::duration duration);

        template<typename It>
        WebCache(const std::string &rootUri, const path &xdgDir, const std::string &storeRoot,
                 std::chrono::system_clock::duration duration, It first, It last)
                 : WebCache(rootUri, xdgDir, storeRoot, duration) {
                     setCacheItem(first, last);
                 }

        /**
         * @brief Add or change a cache item.
         * @param key The key to identify the cache item.
         * @param localId The local identity in the filesystem cache, the file name.
         */
        void setCacheItem(key_t key, local_id_t localId) {
            mItemMap[key] = std::move(localId);
        }

        /**
         * @brief Add or change cache items from a container.
         * @tparam It The iterator type.
         * @param first The first item iterator.
         * @param last The last item iterator.
         */
        template<typename It>
        void setCacheItem(It first, It last) {
            while (first != last) {
                setCacheItem(first->key, std::string(first->name));
                first++;
            }
        }

        /**
         * @brief Test to see if the store exists.
         * @return True if it exits and is a directory.
         */
        bool storeExists() const {
            if (!status_known(mStoreStatus))
                mStoreStatus = status(mStoreRoot);

            return exists(mStoreStatus) && is_directory(mStoreStatus);
        }

        /**
         * @brief Translate a local id.
         * @details This provides a method for derived cache types to modify the local name to something more
         * suitable to the backing filesystem including prepending a relative path. The default is to do nothing.
         * @param localId The local id to translate.
         * @return The translated local id.
         */
        virtual std::string translateItemLocalId(local_id_t localId) {
            return localId;
        }

        /**
         * @brief Get the filesystem path of a local item by key.
         * @param key The key to search for.
         * @return The translated local id. See translateItemLocalId().
         */
        path itemLocalPath(key_t key) {
            if (auto item = mItemMap.find(key); item != mItemMap.end()) {
                auto itemPath = mStoreRoot;
                itemPath.append(translateItemLocalId(item->second));
                return itemPath;
            }
            return path{};
        }

        /**
         * @brief Get the local path to an item if it is known and exists in the local store.
         * @param key The item key.
         * @return std::optional<path> with the file path if it exists.
         */
        std::optional<path> localItemExists(key_t key) {
            if (auto itemPath = itemLocalPath(key); !itemPath.empty()) {
                if (exists(itemPath))
                    return itemPath;
            }
            return std::nullopt;
        }

        /**
         * @brief Get the time the item was cached if the cache time has expired.
         * @param itemPath The item path (returned by itemLocalPath()).
         * @return A time_t expressing the last write time of the local file if cache time has expired, an
         * empty std::optional otherwise.
         */
        std::optional<time_t> cacheTime(const path &itemPath) {
            auto cacheFiletime = fileClockToSystemClock(std::filesystem::last_write_time(itemPath));
            auto cacheFileAge = std::chrono::system_clock::now() - cacheFiletime;
            if (cacheFileAge > mCacheValidDuration)
                return std::chrono::system_clock::to_time_t(cacheFiletime);
            return std::nullopt;
        }

        /**
         * @brief Fetch a cache item.
         * @param key The item key.
         * @param itemUrl The item url (returned by constructUrl()).
         * @param itemPath The full item path.
         * @param cacheFileTime The std::optional returned by cacheTime().
         * @return A tuple containing the returned HTTP status code and the item key.
         */
        static WebCache::result_t
        fetch(WebCache::key_t key, const std::string &itemUrl, const path &itemPath, const path &tempPath,
              std::optional<time_t> cacheFileTime);

        /**
         * @brief Construct the appropriate URL for the item.
         * @details Default procedure is to append the local id to the rout URI.
         * @param localId The local id.
         * @return The constructed URL.
         */
        virtual std::string constructUrl(const local_id_t &localId) {
            return mRootURI + localId;
        }

        bool itemKnown(key_t key) {
            return mItemMap.find(key) != mItemMap.end();
        }

        /**
         * @brief Fetch all cache items which have not been previously cached or have expired cache times.
         */
        bool fetchAll() {
            AsyncList asyncList{};
            std::lock_guard<std::mutex> lockGuard{mMutex};

            for (auto &item : mItemMap) {
                asyncFetchItem(item);
            }
            return !mAsyncList.empty();
        }

        bool fetchItem(key_t key) {
            auto item = mItemMap.find(key);
            if (item != mItemMap.end()) {
                asyncFetchItem(*item);
            }
            return !mAsyncList.empty();
        }

        bool pendingFutures() const {
            return !mAsyncList.empty();
        }

        bool processFutures() {
            if (mAsyncList.empty())
                return false;

            std::lock_guard<std::mutex> lockGuard{mMutex};
            std::chrono::milliseconds span{100};
            for (auto &item : mAsyncList) {
                try {
                    if (item.valid()) {
                        if (auto futureStatus = item.wait_for(span); futureStatus == std::future_status::ready) {
                            auto[status, key] = item.get();
                            cacheLoaded.transmit(key,status);
                        }
                    }
                } catch (const std::exception &e) {
                    std::cout << __PRETTY_FUNCTION__ << ' ' << e.what() << '\n';
                }
            }

            mAsyncList.erase(std::remove_if(mAsyncList.begin(), mAsyncList.end(),
                                           [](std::future<result_t> &f) -> bool { return !f.valid(); }),
                            mAsyncList.end());

            return !mAsyncList.empty();
        }
    };
}
