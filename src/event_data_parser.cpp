//Logging
#include <glog/logging.h>

//C++
#include <cmath>
#include <fstream>

//Boost
#include <boost/filesystem.hpp>

/** Default Values **/
static const int MOG_ID = 1;
static const int VIBE_ID = 2;
static const int EVENT_ID = 32;

/** Function Headers */
void help();
double calcMean(std::vector<double> vals);
double calcVariance(std::vector<double> vals, const double mean);
void openTSVEventFile(std::string event_filename, int &video_id, std::vector<double> &vibe_vals, std::vector<double> &mog_vals);
std::vector<std::pair<size_t,size_t>> calculateEventTimes(std::vector<double> &vals, double dist, double fps);

// TODO Update the help info
void help() {
    LOG(INFO) << "--------------------------------------------------------------------------";
    LOG(INFO) << "This program shows how to use background subtraction methods provided by ";
    LOG(INFO) << " OpenCV. You can process both videos (-vid) and images (-img).";
    LOG(INFO) << "Usage:";
    LOG(INFO) << "./wildlife_bgsub <video filename> <event filename>";
    LOG(INFO) << "for example: ./wildlife_bgsub video.avi events.dat";
    LOG(INFO) << "--------------------------------------------------------------------------";
}

double calcMean(std::vector<double> vals) {
    double val = 0;
    for (size_t i = 0; i < vals.size(); i++) {
        val += vals[i];
    }
    return val/vals.size();
}

double calcVariance(std::vector<double> vals, const double mean) {
    double var = 0;
    for (size_t i = 0; i < vals.size(); i++) {
        double diff = vals[i] - mean;
        var = diff * diff;
    }
    var = var/vals.size();
    return var;
}

void openTSVEventFile(std::string event_filename, int &video_id, std::vector<double> &vibe_vals, std::vector<double> &mog_vals) {
    std::ifstream infile(event_filename);

    std::string event_bool, vibe_val, mog_val;

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        if (!(iss >> video_id >> event_bool >> vibe_val >> mog_val)) {
            break;
        }
        vibe_vals.push_back(atof(vibe_val.c_str()));
        mog_vals.push_back(atof(mog_val.c_str()));
    }
    infile.close();
}

std::vector<std::pair<size_t,size_t>> calculateEventTimes(std::vector<double> &vals, double dist, double fps) {
    std::vector<std::pair<size_t,size_t>> output;

    double mean = calcMean(vals);
    double stdev = sqrt(calcVariance(vals, mean));

    bool event_happening = false;
    size_t start_time = 0;
    LOG(INFO) << "Mean: " << mean;
    LOG(INFO) << "Stdev * Dist: " << dist * stdev;
    for (size_t time = 0; time < vals.size(); time++) {
        if (vals[time] >= mean + (dist * stdev)) {
            event_happening = true;
            start_time = time;
        } else if (event_happening && vals[time] < mean + (dist * stdev)) {
            event_happening = false;
            output.push_back(std::pair<size_t,size_t>(start_time/fps, time/fps));
        }
    }
    return output;
}

/**
 * @function main
 */
int main(int argc, char* argv[])
{
    FLAGS_logtostderr = 1;
    google::InitGoogleLogging(argv[0]);

    //print help information
    help();

    //check for the input parameter correctness
    if(argc != 3) {
        LOG(ERROR) << "Incorret input list";
        return EXIT_FAILURE;
    }

    std::string tsv_event_filename(argv[1]);

    std::vector<size_t> event_times;
    std::vector<double> vibe_vals;
    std::vector<double> mog_vals;

    double fps = 10;

    int video_id;
    openTSVEventFile(tsv_event_filename, video_id, vibe_vals, mog_vals);
    std::string video_id_str = std::to_string(video_id);

    std::vector<std::pair<size_t,size_t>> vibe_events = calculateEventTimes(vibe_vals, 3, fps);
    std::vector<std::pair<size_t,size_t>> mog_events = calculateEventTimes(mog_vals, 3, fps);

    // Open files
    boost::filesystem::path dir(video_id_str);
    boost::filesystem::create_directory(dir);
    std::ofstream event_file(video_id_str + "/significant_events.csv");

    for (size_t i = 0; i < vibe_events.size(); i++) {
        std::pair<size_t,size_t> pair = vibe_events[i];
        event_file << video_id << "," << EVENT_ID << "," << VIBE_ID << "," << pair.first << "," << pair.second << std::endl;
    }

    for (size_t i = 0; i < mog_events.size(); i++) {
        std::pair<size_t,size_t> pair = mog_events[i];
        event_file << video_id << "," << EVENT_ID << "," << MOG_ID << "," << pair.first << "," << pair.second << std::endl;
    }

    // Close files
    event_file.close();

    return EXIT_SUCCESS;
}

