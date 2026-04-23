#include "Database.h"

#include <iostream>

Database& Database::getInstance()
{
    static Database instance;
    return instance;
}

Database::Database()
    : m_db(nullptr)
{
}

Database::~Database()
{
    close();
}

bool Database::open(const std::string& dbPath)
{
    if (m_db)
    {
        return true;
    }

    if (sqlite3_open(dbPath.c_str(), &m_db) != SQLITE_OK)
    {
        std::cerr << "Blad bazy: " << sqlite3_errmsg(m_db) << "\n";
        m_db = nullptr;
        return false;
    }

    std::cout << "Baza danych otwarta: " << dbPath << "\n";
    return true;
}

void Database::close()
{
    if (m_db)
    {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool Database::execute(const std::string& sql)
{
    if (!m_db)
    {
        return false;
    }

    char* errorMsg = nullptr;
    if (sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errorMsg) != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errorMsg << "\n";
        sqlite3_free(errorMsg);
        return false;
    }
    return true;
}

std::vector<std::vector<std::string>> Database::query(const std::string& sql)
{
    std::vector<std::vector<std::string>> results;

    if (!m_db)
    {
        return results;
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        return results;
    }

    int cols = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::vector<std::string> row;
        for (int i = 0; i < cols; i++)
        {
            const char* val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row.push_back(val ? val : "");
        }
        results.push_back(row);
    }

    sqlite3_finalize(stmt);
    return results;
}

int Database::lastInsertId()
{
    if (!m_db)
    {
        return -1;
    }
    return static_cast<int>(sqlite3_last_insert_rowid(m_db));
}

bool Database::createTables()
{
    execute("PRAGMA foreign_keys = ON;");

    execute(R"(
        CREATE TABLE IF NOT EXISTS accounts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            owner_name TEXT NOT NULL DEFAULT '',
            created_at TEXT NOT NULL DEFAULT (datetime('now','localtime'))
        );
    )");

    execute(R"(
        CREATE TABLE IF NOT EXISTS balances (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            account_id INTEGER NOT NULL,
            currency TEXT NOT NULL DEFAULT 'PLN',
            amount REAL NOT NULL DEFAULT 0.0,
            FOREIGN KEY (account_id) REFERENCES accounts(id) ON DELETE CASCADE
        );
    )");

    execute(R"(
        CREATE TABLE IF NOT EXISTS cards (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            card_number TEXT UNIQUE NOT NULL,
            pin TEXT NOT NULL,
            account_id INTEGER NOT NULL,
            is_active INTEGER NOT NULL DEFAULT 1,
            FOREIGN KEY (account_id) REFERENCES accounts(id) ON DELETE CASCADE
        );
    )");

    execute(R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            from_account_id INTEGER,
            to_account_id INTEGER,
            type TEXT NOT NULL,
            amount REAL NOT NULL,
            currency TEXT NOT NULL DEFAULT 'PLN',
            timestamp TEXT NOT NULL DEFAULT (datetime('now','localtime'))
        );
    )");

    return true;
}
