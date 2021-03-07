//
// Created by richard on 2020-12-05.
//

#include <iostream>
#include "Settings.h"

namespace rose {

    Settings::Settings(const std::filesystem::path &configPath) {
        mDbPath = configPath;
        mDbPath.append("settings");
        std::filesystem::create_directories(configPath);
    }

    Settings::Settings(const std::filesystem::path &configPath, const std::string &name) {
        mDbPath = configPath;
        mDbPath.append(name);
        std::filesystem::create_directories(configPath);
    }

    void Settings::initializeDatabase() {
        try {
            soci::session sql(soci::sqlite3, mDbPath.string());
            sql << "CREATE TABLE IF NOT EXISTS " << string_table << " ("
                << "name TEXT PRIMARY KEY,"
                << "value TEXT);";

            sql << "CREATE TABLE IF NOT EXISTS " << int_table << " ("
                << "name TEXT PRIMARY KEY,"
                << "value INTEGER);";

            sql << "CREATE TABLE IF NOT EXISTS " << real_table << " ("
                << "name TEXT PRIMARY KEY,"
                << "value REAL);";

            sql << "CREATE TABLE IF NOT EXISTS " << int_pair_table << " ("
                << "name TEXT PRIMARY KEY,"
                << "a INT, b INT);";

            sql << "CREATE TABLE IF NOT EXISTS " << real_pair_table << " ("
                << "name TEXT PRIMARY KEY,"
                << "a REAL, b REAL);";

            sql << "CREATE TABLE IF NOT EXISTS " << color_table << " ("
                << "name TEXT PRIMARY KEY,"
                << "r REAL, b REAL, g REAL, a REAL);";
        } catch (std::exception const &e) {
            std::cerr << e.what() << '\n';
        }
    }

    void Settings::transmitDataUpdate(const std::string& dataName) {
        dataChangeTx.transmit(dataName);
    }
}
