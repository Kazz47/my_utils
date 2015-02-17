//Logging
#include <glog/logging.h>

//C++
#include <cmath>
#include <fstream>

/** Default Values **/
static const int EVENT_ID = 32;

/** Function Headers */
void help();
double calcMean(std::vector<double> vals);
double calcVariance(std::vector<double> vals, const double mean);
void openTSVEventFile(std::string event_filename, std::vector<double> &vibe_vals, std::vector<double> &mog_vals);
void openEventFile(std::string event_filename, std::vector<size_t> &event_times);
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

int getVideoId(std::string path) {
    int firstIndex = path.find_last_of("/\\");
    int lastIndex = path.find_last_of(".");
    return atoi(path.substr(firstIndex+1, lastIndex).c_str());
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

void openTSVEventFile(std::string event_filename, std::vector<double> &vibe_vals, std::vector<double> &mog_vals) {
    std::ifstream infile(event_filename);

    std::string event_bool, vibe_val, mog_val;

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        if (!(iss >> event_bool >> vibe_val >> mog_val)) {
            break;
        }
        vibe_vals.push_back(atof(vibe_val.c_str()));
        mog_vals.push_back(atof(mog_val.c_str()));
    }
    infile.close();
}

void openEventFile(std::string event_filename, std::vector<size_t> &event_times) {
    std::ifstream infile(event_filename);

    std::string event_id, time;

    std::string line;
    size_t temp;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        if (!(iss >> event_id >> time)) {
            break;
        }
        temp = atoi(time.c_str());
        event_times.push_back(temp);
    }
    infile.close();
    LOG(INFO) << "Loaded " << event_times.size() << " event times.";
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
    std::string input_event_filename(argv[2]);

    int video_id = getVideoId(input_event_filename);

    std::vector<size_t> event_times;
    std::vector<double> vibe_vals;
    std::vector<double> mog_vals;

    double fps = 10;

    openEventFile(input_event_filename, event_times);
    openTSVEventFile(tsv_event_filename, vibe_vals, mog_vals);

    std::vector<std::pair<size_t,size_t>> vibe_events = calculateEventTimes(vibe_vals, 3, fps);
    std::vector<std::pair<size_t,size_t>> mog_events = calculateEventTimes(mog_vals, 3, fps);

    // Open files
    std::ofstream vibe_event_file("significant_vibe_events.dat");
    std::ofstream mog_event_file("significant_mog_events.dat");

    double range = 30;
    double matches = 0;
    for (size_t i = 0; i < vibe_events.size(); i++) {
        std::pair<size_t,size_t> pair = vibe_events[i];
        vibe_event_file << EVENT_ID << "," << video_id << "," << pair.first << "," << pair.second << std::endl;
    }
    LOG(INFO) << "ViBe Total: " << vibe_events.size();
    LOG(INFO) << "ViBe Accuracy: " << matches/event_times.size();

    matches = 0;
    for (size_t i = 0; i < mog_events.size(); i++) {
        std::pair<size_t,size_t> pair = mog_events[i];
        mog_event_file << EVENT_ID << "," << video_id << "," << pair.first << "," << pair.second << std::endl;
    }
    LOG(INFO) << "MOG Total: " << mog_events.size();
    LOG(INFO) << "MOG Accuracy: " << matches/event_times.size();

    // Close files
    vibe_event_file.close();
    mog_event_file.close();

    return EXIT_SUCCESS;
}

