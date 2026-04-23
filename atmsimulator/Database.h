#pragma once

#include <string>
#include <vector>

#include "sqlite3.h"

class Database
{
public:
    static Database& getInstance();

    bool open(const std::string& dbPath);
    void close();
    bool execute(const std::string& sql);
    std::vector<std::vector<std::string>> query(const std::string& sql);
    int lastInsertId();
    bool createTables();

private:
    Database();
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* m_db;
};
