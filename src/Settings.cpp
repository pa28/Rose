//
// Created by richard on 2020-12-05.
//

#include "Settings.h"
#include "Utilities.h"
#include <iostream>

namespace rose {

    Settings::Settings() {
        Environment& environment{Environment::getEnvironment()};
        mDbPath = environment.configHome();
        std::filesystem::create_directories(mDbPath);
        mDbPath.append("settings.db");
        initializeDatabase();
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
