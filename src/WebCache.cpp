/**
 * @file Cache.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-27
 */

#include "WebCache.h"
#include <iomanip>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

namespace rose {
    WebCache::WebCache(const std::string& rootUri, const path& xdgDir, const std::string& storeRoot, std::chrono::system_clock::duration duration) {
        mRootURI = rootUri;
        mStoreRoot = xdgDir;
        mStoreRoot.append(storeRoot);
        mCacheValidDuration = duration;

        std::cout << __PRETTY_FUNCTION__ << ' ' << mStoreRoot.string() << '\n';

        if (!create_directories(mStoreRoot, mEc) && mEc.value()) {
            std::cerr << "Creation of backing store directory \"" << mStoreRoot.string() << "\" failed "
                      << mEc << '\n';
        }

        mStoreStatus = status(mStoreRoot);
    }

    WebCache::result_t WebCache::fetch(WebCache::key_t key, const std::string &itemUrl, const path &itemPath,
                                                      std::optional<time_t> cacheFileTime) {
        std::ofstream strm{itemPath.c_str(), std::ofstream::trunc};
        long responseCode{599};

        if (strm) {
            try {
                std::list<std::string> headers;

                if (cacheFileTime) {
                    std::stringstream ss;
                    ss << "If-Modified-Since: " << std::put_time(std::gmtime(&cacheFileTime.value()), "%a, %d %b %Y %T %Z");
                    headers.push_back(ss.str());
                }

                curlpp::options::Url url(itemUrl);
                curlpp::options::WriteStream ws(&strm);
                curlpp::Easy request;
                request.setOpt(new curlpp::options::HttpHeader(headers));
                request.setOpt(url);
                request.setOpt(ws);
                request.perform();
                responseCode = curlpp::infos::ResponseCode::get(request);
            } catch (const cURLpp::RuntimeError &e) {
                std::cerr << __PRETTY_FUNCTION__ << ' ' << e.what() << '\n';
            } catch (const cURLpp::LogicError &e) {
                std::cerr << __PRETTY_FUNCTION__ << ' ' << e.what() << '\n';
            }

            strm.close();
        }
        return std::make_tuple(responseCode, key);
    }

}
