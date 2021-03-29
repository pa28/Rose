/** @file Settings.h
  * @author Richard Buckley <richard.buckley@ieee.org>
  * @version 1.0
  * @date 2020-12-05
  * @brief The Settings database.
  */

#pragma once

#include <filesystem>
#include <iostream>
#include <soci/soci.h>
#include <sqlite3/soci-sqlite3.h>
#include "Color.h"
#include "Signals.h"
#include "Types.h"

namespace rose {
    using namespace std;

    struct SettingsUpdateProtocol : public Protocol<std::string> {};

    /**
     * @class Settings
     * @brief A settings database.
     */
    class Settings {
    protected:
        std::filesystem::path mDbPath;                  ///< The file path to the settings database

        static constexpr std::string_view string_table = "settings_string";         ///< The string table
        static constexpr std::string_view int_table = "settings_int";               ///< The integer table
        static constexpr std::string_view real_table = "settings_real";             ///< The real table
        static constexpr std::string_view int_pair_table = "settings_int_pair";     ///< The integer pair table
        static constexpr std::string_view real_pair_table = "settings_real_pair";   ///< The real pair table
        static constexpr std::string_view color_table = "settings_color";           ///< The color table

        /**
         * @brief Send out notification that data has changed.
         * @param dataName The name of the data item that has changed.
         */
        void transmitDataUpdate(const std::string& dataName);

        Settings();

    public:

        static Settings& getSettings() {
            static Settings instance{};
            return instance;
        }

        /**
         * @brief Initialize the database, creating the required tables if they have not been created.
         */
        void initializeDatabase();

        /// The Signal to notify of settings updates.
        SettingsUpdateProtocol::signal_type dataChangeTx{};

        /**
         * @brief Get a value from settings the database.
         * @details Select the table to search based on the type of value sought.
         * @tparam T The type of the value to retrieve.
         * @tparam S The type of the name of the value.
         * @param sql The database session to use.
         * @param name The name of the value
         * @return A std::optional<T> containing the value if found, empty if not found.
         */
        template<typename T, typename S>
        std::optional<T> getDatabaseValue(soci::session &sql, S name) {
            soci::indicator ind;
            if constexpr (is_integral_v<T>) {
                T value;
                sql << "SELECT value FROM " << int_table << " WHERE name = \"" << name << '"', soci::into(value, ind);
                if (sql.got_data() && ind == soci::i_ok) {
                    return value;
                }
                return nullopt;
            } else if constexpr (is_floating_point_v<T>) {
                T value;
                sql << "SELECT value FROM " << real_table << " WHERE name = \"" << name << '"', soci::into(value, ind);
                if (sql.got_data() && ind == soci::i_ok) {
                    return value;
                }
                return nullopt;
            } else if constexpr (is_base_of_v<std::array<int,2>, T>) {
                soci::row r;
                sql << "SELECT a,b FROM " << int_pair_table << " WHERE name = \"" << name << '"', soci::into(r);
                if (sql.got_data() && r.get_indicator(0) == soci::i_ok && r.get_indicator(1) == soci::i_ok) {
                    return T{r.get<int>(0), r.get<int>(1)};
                }
                return nullopt;
            } else if constexpr (is_base_of_v<Size, T>) {
                soci::row r;
                sql << "SELECT a,b FROM " << int_pair_table << " WHERE name = \"" << name << '"', soci::into(r);
                if (sql.got_data() && r.get_indicator(0) == soci::i_ok && r.get_indicator(1) == soci::i_ok) {
                    return T{r.get<int>(0), r.get<int>(1)};
                }
                return nullopt;
            } else if constexpr (is_base_of_v<Position, T>) {
                soci::row r;
                sql << "SELECT a,b FROM " << int_pair_table << " WHERE name = \"" << name << '"', soci::into(r);
                if (sql.got_data() && r.get_indicator(0) == soci::i_ok && r.get_indicator(1) == soci::i_ok) {
                    return T{r.get<int>(0), r.get<int>(1)};
                }
                return nullopt;
            } else if constexpr (is_base_of_v<std::array<double,2>, T>) {
                soci::row r;
                sql << "SELECT a,b FROM " << real_pair_table << " WHERE name = \"" << name << '"', soci::into(r);
                if (sql.got_data() && r.get_indicator(0) == soci::i_ok && r.get_indicator(1) == soci::i_ok) {
                    return T{r.get<double>(0), r.get<double>(1)};
                }
                return nullopt;
            } else if constexpr (is_same_v<T, string> || is_same_v<T, const string> ||
                                 is_same_v<T, string_view> || is_same_v<T, const string_view> ||
                                 is_same_v<T, char *> || is_same_v<T, const char *>) {
                T value;
                sql << "SELECT value FROM " << string_table << " WHERE name = \"" << name << '"', soci::into(value, ind);
                if (sql.got_data() && ind == soci::i_ok) {
                    return value;
                }
                return nullopt;
            } else if constexpr (is_same_v<T, color::RGBA>) {
                soci::row r;
                sql << "SELECT r,g,b,a FROM " << color_table << " WHERE name = \"" << name << '"', soci::into(r);
                if (sql.got_data() && r.get_indicator(0) == soci::i_ok && r.get_indicator(1) == soci::i_ok
                    && r.get_indicator(2) == soci::i_ok && r.get_indicator(3) == soci::i_ok) {
                    color::RGBA value{};
                    value.r() = r.get<double>(0);
                    value.g() = r.get<double>(1);
                    value.b() = r.get<double>(2);
                    value.a() = r.get<double>(3);
                    return value;
                }
                return nullopt;
            } else if constexpr (is_same_v<T, color::HSVA>) {
                soci::row r;
                sql << "SELECT r,g,b,a FROM " << color_table << " WHERE name = \"" << name << '"', soci::into(r);
                if (sql.got_data() && r.get_indicator(0) == soci::i_ok && r.get_indicator(1) == soci::i_ok
                    && r.get_indicator(2) == soci::i_ok && r.get_indicator(3) == soci::i_ok) {
                    color::HSVA value{};
                    value.hue() = r.get<double>(0);
                    value.saturation() = r.get<double>(1);
                    value.value() = r.get<double>(2);
                    value.alpha() = r.get<double>(3);
                    return value;
                }
                return nullopt;
            } else {
                static_assert(is_integral_v<T>, "Value type not supported by Settings implementation." );
            }
        }

        /**
         * @brief Set a value in the settings database.
         * @details Select the table to store the value based on the type of value.
         * @tparam T The type of the value to retrieve.
         * @tparam S The type of the name of the value.
         * @param sql The database session to use.
         * @param name The name of the value
         * @param value The value to store.
         */
        template<typename T, typename S>
        void setDatabaseValue(soci::session &sql, S name, T value) {
            if constexpr (is_integral_v<T>) {
                sql << "INSERT OR REPLACE INTO " << int_table << " (name,value) VALUES (\"" << name << "\"," << value << ')';
            } else if constexpr (is_floating_point_v<T>) {
                sql << "INSERT OR REPLACE INTO " << real_table << " (name,value) VALUES (\"" << name << "\"," << value << ')';
            } else if constexpr (is_base_of_v<std::array<int,2>,T>) {
                sql << "INSERT OR REPLACE INTO " << int_pair_table << " (name,a,b) VALUES (\"" << name << "\","
                    << value.at(0) << ',' << value.at(1) << ')';
            } else if constexpr (is_base_of_v<Size,T>) {
                sql << "INSERT OR REPLACE INTO " << int_pair_table << " (name,a,b) VALUES (\"" << name << "\","
                    << value.w << ',' << value.h << ')';
            } else if constexpr (is_base_of_v<Position,T>) {
                sql << "INSERT OR REPLACE INTO " << int_pair_table << " (name,a,b) VALUES (\"" << name << "\","
                    << value.x << ',' << value.y << ')';
            } else if constexpr (is_base_of_v<std::array<double,2>,T>) {
                sql << "INSERT OR REPLACE INTO " << real_pair_table << " (name,a,b) VALUES (\"" << name << "\"," << value.at(0) << ',' << value.at(1) << ')';
            } else if constexpr (is_same_v<T,string> || is_same_v<T,const string> ||
                    is_same_v<T,string_view> || is_same_v<T,const string_view> ||
                            is_same_v<T,char*> || is_same_v<T,const char *>) {
                sql << "INSERT OR REPLACE INTO " << string_table << " (name,value) VALUES (\"" << name << "\",\"" << value << "\")";
            } else if constexpr (is_same_v<T, color::RGBA>) {
                sql << "INSERT OR REPLACE INTO " << color_table << " (name,r,g,b,a) VALUES (\"" << name << "\","
                    << value.r() << ',' << value.g() << ',' << value.b() << ',' << value.a() << ");";
            } else if constexpr (is_same_v<T, color::HSVA>) {
                sql << "INSERT OR REPLACE INTO " << color_table << " (name,r,g,b,a) VALUES (\"" << name << "\","
                    << (float)value.mHue << ',' << value.mSaturation << ',' << value.mLightness << ',' << value.mAlpha << ");";
            } else {
                static_assert(is_integral_v<T>, "Value type not supported by Settings implementation." );
                return;
            }
            transmitDataUpdate(std::string{name});
        }

        /**
         * @brief Set a value in the settings database.
         * @tparam T The type of the value.
         * @tparam S The type of the name of the value.
         * @param name The name of the value.
         * @param value The value.
         */
        template<typename T, typename S>
        void setValue(S name, T value) {
            try {
                soci::session sql(soci::sqlite3, mDbPath.string());
                setDatabaseValue<T>(sql, name, value);
            } catch (std::exception const &e) {
                std::cerr << e.what() << '\n';
            }
        }

        /**
         * @brief Get a value in the settings database.
         * @tparam T The type of the value.
         * @tparam S The type of the name of the value.
         * @param name The name of the value.
         * @return A std::optional<T> with the value if found, empty if not found.
         */
        template<typename T, typename S>
        std::optional<T> getValue(S name) {
            try {
                soci::session sql(soci::sqlite3, mDbPath.string());
                return getDatabaseValue<T,S>(sql, name);
            } catch (std::exception const &e) {
                std::cerr << e.what() << '\n';
                return std::nullopt;
            }
        }

        /**
         * @brief Get a value in the settings database, returning the default value if not found.
         * @tparam T The type of the value.
         * @tparam S The type of the name of the value.
         * @param name The name of the value.
         * @param defaultValue The default value to return if the value is not in the database.
         * @return The found value, or the default value.
         */
        template<typename T, typename S>
        T getValue(S name, T defaultValue) {
            auto value = getValue<T,S>(name);
            if (value)
                return value.value();
            return defaultValue;
        }
    };
}

