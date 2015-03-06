#include "db_insert.hpp"

#include <glog/logging.h>

//C++
#include <fstream>

#define mysql_query_check(conn, query) __mysql_check (conn, query, __FILE__, __LINE__)

void __mysql_check(MYSQL *conn, std::string query, const char *file, const int line) {
    mysql_query(conn, query.c_str());

    if (mysql_errno(conn) != 0) {
        std::ostringstream ex_msg;
        ex_msg << "ERROR in MySQL query: '" << query.c_str() << "'. Error: " << mysql_errno(conn) << " -- '" << mysql_error(conn) << "'. Thrown on " << file << ":" << line;
        LOG(ERROR) << ex_msg.str();
        exit(EXIT_FAILURE);
    }
}

DBInsert::DBInsert() {
    this->db_conn = mysql_init(NULL);

    // Get database info from file
    std::string db_host, db_name, db_password, db_user;
    std::ifstream db_info_file("../wildlife_db_info");

    db_info_file >> db_host >> db_name >> db_user >> db_password;
    db_info_file.close();

    LOG(INFO) << "Parsed DB Info:";
    LOG(INFO) << "\tHost: " << db_host;
    LOG(INFO) << "\tName: " << db_name;
    LOG(INFO) << "\tUser: " << db_user;
    LOG(INFO) << "\tPass: " << db_password;

    if (mysql_real_connect(this->db_conn, db_host.c_str(), db_user.c_str(), db_password.c_str(), db_name.c_str(), 0, NULL, 0) == NULL) {
        LOG(ERROR) << "Error connecting to database: " << mysql_errno(this->db_conn) << ", '" << mysql_error(this->db_conn) << "'";
        exit(EXIT_FAILURE);
    }
}

DBInsert::~DBInsert() {
    delete this->db_conn;
}

void DBInsert::openEventFile(std::string event_filename, std::vector<std::vector<size_t>*> &events) {
    std::ifstream infile(event_filename);

    std::string line, event_id_str, video_id_str, start_s_str, end_s_str;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string field;
        std::vector<size_t> *data = new std::vector<size_t>();
        while (std::getline(iss, field, ',')) {
            data->push_back(atoi(field.c_str()));
        }
        events.push_back(data);
    }
    infile.close();
    LOG(INFO) << "Loaded " << events.size() << " events.";
}

void DBInsert::parseFile(const int version_id, std::string filename) {
    std::vector<std::vector<size_t>*> events;
    openEventFile(filename, events);

    for (size_t i = 0; i < events.size(); i++) {
        std::vector<size_t> *event = events[i];
        std::ostringstream insert_event_query;
        size_t video_id = event->at(0);
        size_t event_id = event->at(1);
        size_t algorithm_id = event->at(2);
        size_t start_time_s = event->at(3);
        size_t end_time_s = event->at(4);
        insert_event_query << "INSERT INTO computed_events VALUES (NULL, " << algorithm_id << ", " << event_id << ", " << video_id << ", " << version_id << ", " << start_time_s << ", " << end_time_s << ");";
        LOG(INFO) << insert_event_query.str();
        mysql_query_check(this->db_conn, insert_event_query.str());
    }

    LOG(INFO) << "Added events to DB";
}

int main(int argc, char** argv) {
    FLAGS_logtostderr = 1;
    google::InitGoogleLogging(argv[0]);

    if(argc != 3) {
        LOG(ERROR) << "Incorret input list, requires version number and a CSV input file of events.";
        return EXIT_FAILURE;
    }

    std::string version_id_str(argv[1]);
    std::string csv_event_filename(argv[2]);

    DBInsert database;
    database.parseFile(std::stoi(version_id_str.c_str()), csv_event_filename);

    return EXIT_SUCCESS;
}
