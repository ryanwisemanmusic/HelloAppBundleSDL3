#include "gameScores.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

DatabaseManager::DatabaseManager(const std::string &dbFile)
{
    std::string dbFolder = "database";
    std::string dbPath = dbFolder + "/" + dbFile;

    // Ensure database directory exists
    if (mkdir(dbFolder.c_str(), 0777) != 0 && errno != EEXIST)
    {
        std::cerr << "Failed to create database directory!" 
        << std::endl;
    }

    // Open the SQLite database
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
    {
        std::cerr << "Cannot open database. Error: " << 
        sqlite3_errmsg(db) << std::endl;
    }
    else
    {
        std::cout << "Database created at: " << dbPath << std::endl;
        std::string createTableSQL = 
            "CREATE TABLE IF NOT EXISTS scores ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "player_name TEXT NOT NULL, "
            "score INTEGER NOT NULL, "
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
            ");";
        
        executeSQL(createTableSQL);
    }
}

DatabaseManager::~DatabaseManager()
{
    sqlite3_close(db);
}

bool DatabaseManager::executeSQL(const std::string &sql)
{
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

bool DatabaseManager::insertTestScore(
    const std::string &player_name, int score)
{
    const std::string insertSQL = 
    "INSERT INTO scores (player_name, score) VALUES(?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(
        db, insertSQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "SQL prepare error: " << 
        sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, player_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, score);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    if (!success)
    {
        std::cerr << "SQL execution error: " << 
        sqlite3_errmsg(db) << std::endl;
    }

    return success;
}

void DatabaseManager::queryScores()
{
    const std::string querySQL = 
    "SELECT id, player_name, score, timestamp FROM scores;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(
        db, querySQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << 
        "Failed to prepare query: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const char* player_name = 
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int score = sqlite3_column_int(stmt, 2);
        const char* timestamp = 
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        std::cout << "ID: " << id << ", Player: " << 
        (player_name ? player_name : "NULL")
                  << ", Score: " << score << ", Timestamp: " << 
                  (timestamp ? timestamp : "NULL") << std::endl;
    }

    if (sqlite3_errcode(db) != SQLITE_DONE)
    {
        std::cerr << "Query execution error: " << 
        sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

