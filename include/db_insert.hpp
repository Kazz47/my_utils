#ifndef DB_INSERT_H
#define DB_INSERT_H

#include <string>
#include <vector>

#include "mysql/mysql.h"

/**
 * DBInsert class.
 */
class DBInsert {
public:
    DBInsert();
    ~DBInsert();
    void parseFile(const int version_id, std::string filename);

private:
    MYSQL *db_conn;
    void openEventFile(std::string event_filename, std::vector<std::vector<size_t>*> &events);
};

#endif //DB_INSERT_H

