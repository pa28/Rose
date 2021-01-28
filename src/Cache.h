/** @file Cache.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-11-12
  * @brief Classes and functions for caching data fetched from the network.
  */

#pragma once

#include <memory>
#include <string>
#include <istream>
#include <future>
#include <mutex>
#include <ostream>
#include <optional>
#include <utility>
#include <filesystem>
#include <map>
#include <chrono>
#include "Constants.h"
#include "Signals.h"

namespace rose {

    using CacheURI = std::string;           ///< Type for a cache URI value
    using CacheObjectURI = std::string;     ///< Type for a cache object URI value

    class CacheObject;

    /**
     * @class CacheLocalStore
     * @brief A pure virtual class specifying the interface to a cache store system.
     */
    class CacheLocalStore {
    public:
        CacheLocalStore() = default;
        virtual ~CacheLocalStore() = default;

        /**
         * @brief Create an item on a Cache.
         * @param object The cache object to be created.
         * @return A CacheError code.
         */
        virtual CacheError create(const CacheObject &object) = 0;

        /**
         * @brief Find an item on a Cache.
         * @param object The cache object to be found.
         * @return A CacheError code.
         */
        virtual bool find(const CacheObject &object) = 0;

        /**
         * @brief Open a cache object for writing.
         * @param object The cache object to open for writing
         * @param temp True if a temporary should be opened to use write then move idiom.
         * @return A std::ostream to write to.
         */
        virtual std::ofstream openWrite(const CacheObject &object, bool temp) = 0;

        /**
         * @brief Move object from temporary cache location to permanent cache location
         * @param object The object to move from a temporary to permanent location.
         */
        virtual void moveFromTemp(const CacheObject &object) = 0;

        /**
         * @brief Modify the object cache time.
         * @param object The cache object to modify the object cache time on.
         * @param cacheTimeInc The amount to increment the object cache time by, or 0 to set it to the current
         * system time.
         */
        virtual void
        setCacheTime(const CacheObject &object, std::chrono::system_clock::duration cacheTimeInc) = 0;

        /**
         * @brief Remove a temporary cache location.
         * @details This is used when an attempt to update a cached object either failed, or did not result
         * in new data.
         * @param object The object for which the temporary should be removed.
         */
        virtual void removeTemp(const CacheObject &object) = 0;

        /**
         * @brief Open a cache object for reading.
         * @param object The object to open for reading.
         * @return A std::istream to read from.
         */
        virtual std::ifstream openRead(const CacheObject &object) = 0;

        /**
         * @brief Get a filesystem name locator for the object, if available.
         * @return A std::optional<std::string>
         */
        [[nodiscard]] virtual std::optional<std::string> fileSystemName(const CacheObject &object) const { return std::nullopt; }
    };

    /**
     * @class CacheObject
     * @brief The base class for an object to be cached.
     */
    class CacheObject {
    protected:
        std::string mObjectSrcName{};   ///< The name of the resource at the source.
        std::string mObjectUsrName{};   ///< The name to presetn to the user.

        long mStatusCode;               ///< The status code for the last fetch.

        bool mFirstProcess{false};      ///< Used to mange handling on first "fetch".

    public:
        CacheObject() = delete;
        virtual ~CacheObject() = default;

        /**
         * Constructor
         * @param srcName The name of the object on the source.
         * @param userName The name of the object to present to the user.
         */
        explicit CacheObject(std::string srcName, std::string userName = "")
            : mObjectSrcName(std::move(srcName)), mObjectUsrName(std::move(userName)), mStatusCode(0) {}

        /**
         * @brief Accessor for the object source name.
         * @return The object source name.
         */
        [[nodiscard]] const std::string& objectSrcName() const { return mObjectSrcName; }

        /**
         * @brief Accessor for the object user name.
         * @return The object user name.
         */
        [[nodiscard]] const std::string& objectUsrName() const { return mObjectUsrName; }

        /**
         * @brief Set the cash fetch status code.
         * @param statusCode The status code set by the cache source.
         */
        void setStatusCode(long statusCode) { mStatusCode = statusCode; }

        /**
         * @brief Get the cache fetch status code.
         * @return The status code set by the cache source.
         */
        [[nodiscard]] long getStatusCode() const { return mStatusCode; }

        /**
         * @brief Called after the first process.
         * @details When true the object has been initially processed from the cache either after a fetch, or
         * directly from an existing cache entry. If the value mFirstProcess is false during a fetch, status
         * codes of 304 are treated as if they were 200.
         */
        void setFirstProcess() { mFirstProcess = true; }

        /**
         * @brief Get the value of the first process flag.
         * @return the value of mFirstProcess.
         */
        [[nodiscard]] bool getFirstProcess() const { return mFirstProcess; }
    };

    /**
     * @class CacheSource
     * @brief Virtual ase class for data sources to be cached.
     */
    class CacheSource {
    protected:
        std::chrono::system_clock::duration mCacheValidDuration{};

    public:
        CacheSource();
        virtual ~CacheSource() = default;

        /**
         * @brief Fetch a cache object from a source.
         * @param cacheObject The object to fetch
         * @param ostrm The output stream to write the fetched object on.
         * @param cacheTime The time on the cache object.
         */
        virtual void fetch(CacheObject &cacheObject, std::ostream &ostrm, time_t cacheTime) = 0;

        /**
         * @brief Access the cache object validity period.
         * @return A system_clock duration of cache object validity.
         */
        [[nodiscard]] auto cacheValidDuration() const { return mCacheValidDuration; }
    };

    /**
     * @class Cache
     * @brief A collection CacheObject objects associated with
     */
    class Cache : public std::map<uint32_t,CacheObject> {
    protected:
        std::unique_ptr<CacheLocalStore> localStore;     ///< The local store for the Cache.
        std::unique_ptr<CacheSource> source;             ///< The source for cache objects.

    public:
        Cache() = default;
        virtual ~Cache() = default;
    };

    /**
     * @class CacheFileSystem
     * @brief A CacheLocalStore implemented on the local filesystem.
     */
    class CacheFileSystem : public CacheLocalStore {
    protected:
        std::error_code mErrorCode;         ///< Store the last error code
        std::filesystem::path mRootPath;    ///< Store the filesystem path to the cache storage

    public:
        CacheFileSystem() = delete;
        ~CacheFileSystem() override = default;

        /**
         * @brief Return false if mErrorCode is zero.
         * @return true if there is an error code, false otherwise.
         */
        explicit operator bool() const noexcept {
            return mErrorCode.operator bool();
        }

        /**
         * @brief Constructor
         * @param rootPath The root path of the filesystem cache tree.
         */
        explicit CacheFileSystem(std::filesystem::path rootPath);

        /**
         * @brief Constructor
         * @param rootPath The root path of the filesystem cache tree.
         */
        explicit CacheFileSystem(const std::string& rootPath) : CacheFileSystem(std::filesystem::path(rootPath)) {}

        /**
         * @brief Constructor
         * @param rootPath The root path to all application related filesystem cache.
         * @param cacheName The specific cache name, will be a directory under rootPath.
         */
        CacheFileSystem(const std::filesystem::path& rootPath, const std::string& cacheName) : CacheFileSystem(rootPath) {
            mRootPath.append(cacheName);
            if (!mErrorCode)
                std::filesystem::create_directory(mRootPath, mErrorCode);
        }

        /**
         * @brief Accessor for cache root path
         * @return The filesystem path to the cache.
         */
        [[nodiscard]] const std::filesystem::path& rootPath() const { return mRootPath; }

        /**
         * @brief Create an item on a Cache.
         * @param object The cache object to be created.
         * @return A CacheError code.
         */
        CacheError create(const CacheObject &object) override;

        /**
         * @brief Find an item on a Cache.
         * @param object The cache object to be found.
         * @return A CacheError code.
         */
        bool find(const CacheObject &object) override;

        /**
         * @brief Open a cache object for writing.
         * @return A std::ostream to write to.
         */
        std::ofstream openWrite(const CacheObject &object, bool temp) override;

        /**
         * @brief Open a cache object for reading.
         * @return A std::istream to read from.
         */
        std::ifstream openRead(const CacheObject &object) override;

        /**
         * @brief Move object from temporary cache location to permanent cache location
         * @param object The object to move.
         */
        void moveFromTemp(const CacheObject &object) override;

        /**
         * @brief Remove a temporary cache location.
         * @details This is used when an attempt to update a cached object either failed, or did not result
         * in new data.
         * @param object The object to remove.
         */
        void removeTemp(const CacheObject &object) override;

        /**
         * @brief Set file update time to the current system time.
         */
        void setCacheTime(const CacheObject &object, std::chrono::system_clock::duration cacheTimeInc) override;

        /**
         * @brief Get a filesystem name locator for the object, if available.
         * @return A std::optional<std::string>
         */
        [[nodiscard]] std::optional<std::string> fileSystemName(const CacheObject &object) const override;
    };

    /**
     * @class CacheWebSource
     * @brief A cache source on the World Wide Web.
     */
    class CacheWebSource : public CacheSource {
    protected:
        std::string mSourceURI;     ///< The URI for the source.
        long mResponseCode{0};      ///< The returned response code.

    public:
        CacheWebSource() = delete;
        ~CacheWebSource() override = default;

        /**
         * @brief Constructor
         * @details The sourceURI will be prepended to the CacheObject source name to create the CacheObject
         * source URI. The default cache validity period is 1 hour on the system clock.
         * @param sourceURI The source URI.
         */
        explicit CacheWebSource(std::string  sourceURI);

        /**
         * @brief Constructor
         * @details The sourceURI will be prepended to the CacheObject source name to create the CacheObject
         * source URI. The cache validity period is set to the value specified in cacheValidityDuration.
         * @param sourceURI The source URI.
         * @param cacheValidityDuration The duration that a cache object is considered valid.
         */
        CacheWebSource(std::string sourceURI, std::chrono::system_clock::duration cacheValidityDuration);

        /**
         * @brief Fetch a source object.
         * @param cacheObject The object to fetch.
         * @param ostrm An output stream to write the object to.
         * @param cacheTime The time for the object currently in cache.
         */
        void fetch(CacheObject &cacheObject, std::ostream &ostrm, time_t cacheTime) override;

        /**
         * @brief Get the response code returned by the server.
         * @return the response code.
         */
        [[nodiscard]] long responseCode() const { return mResponseCode; }
    };

    /**
     * @class WebFileCache
     * @brief A cache with a source on the World Wide Web, and local store on the filesystem.
     */
    class WebFileCache : public Cache {
    protected:
        WebFileCache();

        SignalSerialNumber mSignalSerialNumber{};   ///< The signal serial number for this object

        std::mutex mMutex;      ///< Mutex for locking the WebFileCache

        std::shared_ptr<Slot<int>> mCheckValidity;      ///< Slot for check validity timing signal
        std::shared_ptr<Slot<int>> mCheckFutures;       ///< Slot for check futures timeing signal

        std::vector<std::future<std::optional<uint32_t>>> mFutureList;  ///< The list of outstanding futures

        void checkFutures();    ///< Check for completed futures and process.

        /**
         * @brief A std::async capable method of fetching a single CacheObject.
         * @param self A pointer to the WebFileCache (this).
         * @param id The identifier of the CacheObject
         * @param cacheTime The cache time of the object.
         * @return The identifier if the fetch was successful, empty if not.
         */
        static std::optional<uint32_t> asyncFetch(WebFileCache *self, uint32_t id, time_t cacheTime);

    public:
        ~WebFileCache() override = default;

        /**
         * @brief Constructor
         * @details See: CacheWebSource and CacheFileSystem
         * @param sourceURI The source URI to initialize the cache source.
         * @param rootPath The filesystem root path to initialize the local store.
         * @param cacheName The cache name to initialize the local store.
         */
        WebFileCache(const std::string& sourceURI,
                     const std::filesystem::path& rootPath, const std::string& cacheName);

        /**
         * @brief Constructor
         * @details See: CacheWebSource and CacheFileSystem
         * @param sourceURI The source URI to initialize the cache source.
         * @param rootPath The filesystem root path to initialize the local store.
         * @param cacheName The cache name to initialize the local store.
         * @param cacheValidityDuration The duration cache objects should be considered valid for.
         */
        WebFileCache(const std::string& sourceURI, const std::filesystem::path& rootPath,
                     const std::string& cacheName, std::chrono::system_clock::duration cacheValidityDuration);

        /**
         * @brief Connect the WebFileCache to signals to trigger regular data checks.
         * @param futureCheck The signal used to time checks for completed asynchronous fetches.
         * @param validityCheck The signal used to time checks for cache object validity.
         */
        void connect(Signal<int> &futureCheck, Signal<int> &validityCheck);

        Signal<uint32_t> itemFetched{};     ///< Signal to transmit when a cache object is fetched.

        explicit operator bool() const { return localStore.operator bool(); } ///< Determine local store validity.

        /**
         * @brief Get the Cache root path.
         * @return std::filesystem::path to the cache root.
         */
        [[nodiscard]] auto cacheRootPath() const {
            return dynamic_cast<CacheFileSystem*>(localStore.get())->rootPath();
        }

        /**
         * @brief Attempt to fetch all expired or not yet fetched objects.
         */
        void fetchAll();
    };
}
