#include "boinc_utils.hpp"

std::string getBoincFilename(std::string filename) throw(std::runtime_error) {
    std::string resolved_path = filename;
#ifdef _BOINC_APP_
    if(boinc_resolve_filename_s(filename.c_str(), resolved_path)) {
        LOG(ERROR) << "Could not resolve filename '" << filename.c_str() << "'";
        throw std::runtime_error("Boinc could not resolve filename");
    }
#endif
    return resolved_path;
}

void updateSHMEM(WILDLIFE_SHMEM *shmem, const unsigned int &frame_pos, const double &fps) {
    if(shmem == NULL) {
        return;
    }
    //BOINC Values
    shmem->update_time = getTimeInSeconds();
#ifdef _BOINC_APP_
    shmem->fraction_done = boinc_get_fraction_done();
    shmem->cpu_time = boinc_worker_thread_cpu_time();
    boinc_get_status(&shmem->status);
#endif
    //Custom Values
    shmem->fps = fps;
    shmem->frame_pos = frame_pos;
}

double calculateFPS(unsigned int &previous_time) {
    unsigned int current_time;

#ifdef WIN32
    SYSTEMTIME st;
    GetSystemTime(&st);
    current_time = st.wSecond * 1000000 + st.wMilliseconds * 1000;
#else
    struct timeval time;
    gettimeofday(&time, NULL);
    current_time = time.tv_sec * 1000000 + time.tv_usec;
#endif

    if(previous_time == 0) previous_time = current_time;
    unsigned int time_interval = current_time - previous_time;

    double fps = 1.0/time_interval;
    previous_time = current_time;
    return fps;
}

double getTimeInSeconds() {
    time_t timer;
    struct tm y2k = {0};
    y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
    y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;
    time(&timer);
    return difftime(timer,mktime(&y2k));
}
