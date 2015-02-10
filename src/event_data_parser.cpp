//Logging
#include <glog/logging.h>

//C++
#include <cmath>
#include <fstream>

/** Function Headers */
void help();
double calcMean(std::vector<double> vals);
double calcVariance(std::vector<double> vals, const double mean);
void openEventFile(std::string event_filename, std::vector<double> &vibe_vals, std::vector<double> &mog_vals);
std::vector<std::pair<size_t,size_t>> calculateEventTimes(std::vector<double> &vals, double dist, int fps);

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
    for (double x : vals) {
        val += x;
    }
    return val/vals.size();
}

double calcVariance(std::vector<double> vals, const double mean) {
    double var = 0;
    for (double x : vals) {
        double diff = x - mean;
        var = diff * diff;
    }
    var = var/vals.size();
    return var;
}

void openEventFile(std::string event_filename, std::vector<double> &vibe_vals, std::vector<double> &mog_vals) {
    LOG(INFO) << "Load event file: '" << event_filename << "'";

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
    if(argc != 2) {
        LOG(ERROR) << "Incorret input list";
        return EXIT_FAILURE;
    }

    std::string event_filename(argv[1]);

    std::vector<double> vibe_vals;
    std::vector<double> mog_vals;

    openEventFile(event_filename, vibe_vals, mog_vals);

    std::vector<std::pair<size_t,size_t>> vibe_events = calculateEventTimes(vibe_vals, 1, 10);
    std::vector<std::pair<size_t,size_t>> mog_events = calculateEventTimes(mog_vals, 1, 10);

    // Open files
    std::ofstream vibe_event_file("significant_vibe_events.dat");
    std::ofstream mog_event_file("significant_mog_events.dat");

    for (std::pair<size_t,size_t> pair : vibe_events) {
        vibe_event_file << pair.first << '\t' << pair.second << std::endl;
    }

    for (std::pair<size_t,size_t> pair : mog_events) {
        mog_event_file << pair.first << '\t' << pair.second << std::endl;
    }

    // Close files
    vibe_event_file.close();
    mog_event_file.close();

    return EXIT_SUCCESS;
}

std::vector<std::pair<size_t,size_t>> calculateEventTimes(std::vector<double> &vals, double dist, int fps) {
    std::vector<std::pair<size_t,size_t>> output;

    double mean = calcMean(vals);
    double stdev = sqrt(calcVariance(vals, mean));

    bool event_happening = false;
    size_t start_time = 0;
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

