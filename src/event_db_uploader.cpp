#include "db_insert.hpp"

#include <glog/logging.h>

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

void DBInsert::openEventFile(std::string event_filename, std::vector<std::vector<size_t>> &events) {
    std::ifstream infile(event_filename);

    std::string line, event_id_str, video_id_str, start_s_str, end_s_str;
    std::vector<size_t> data;
    while (std::getline(infile, line, ',')) {
        std::istringstream iss(line);
        if (!(iss >> event_id_str >> video_id_str >> start_s_str >> end_s_str)) {
            break;
        }
        data.push_back(atoi(event_id_str.c_str()));
        data.push_back(atoi(video_id_str.c_str()));
        data.push_back(atoi(start_s_str.c_str()));
        data.push_back(atoi(end_s_str.c_str()));
        events.push_back(data);
    }
    infile.close();
    LOG(INFO) << "Loaded " << events.size() << " events.";
}

void DBInsert::parseFile(std::string filename) {
    std::vector<std::vector<size_t>> events;
    openEventFile(filename, events);

    std::ostringstream full_video_query;
    full_video_query << "SELECT species_id, location_id FROM video_2 WHERE id = 6511;";

    mysql_query_check(this->db_conn, full_video_query.str());
    MYSQL_RES *video_result = mysql_store_result(this->db_conn);

    LOG(INFO) << " got video result";

    MYSQL_ROW full_video_row = mysql_fetch_row(video_result);
    std::string species_id(full_video_row[0]);
    std::string location_id(full_video_row[1]);

    mysql_free_result(video_result);
}

int main(int argc, char** argv) {
    FLAGS_logtostderr = 1;
    google::InitGoogleLogging(argv[0]);

    if(argc != 2) {
        LOG(ERROR) << "Incorret input list";
        return EXIT_FAILURE;
    }

    std::string csv_event_filename(argv[1]);

    DBInsert database;
    database.parseFile(csv_event_filename);

    return EXIT_SUCCESS;
}
