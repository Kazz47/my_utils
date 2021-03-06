#ifndef BOINC_UTILS_H
#define BOINC_UTILS_H

//Logging
#include <glog/logging.h>

//C++
#include <iostream>
#include <stdexcept>
#ifndef _WIN32
#include <sys/time.h>
#endif

//BOINC
#ifdef _BOINC_APP_
#ifdef _WIN32
#include "boinc_win.h"
#include "str_util.h"
#include "diagnostics.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"
#else
#include "boinc/diagnostics.h"
#include "boinc/util.h"
#include "boinc/filesys.h"
#include "boinc/boinc_api.h"
#include "boinc/mfile.h"
#endif
#endif

struct WILDLIFE_SHMEM {
    //BOINC Values
    double update_time;
    double fraction_done;
    double cpu_time;
    double fps;
#ifdef _BOINC_APP_
    BOINC_STATUS status;
#endif
    //Custom Values
    unsigned int frame_pos;
    char species[256];
    char filename[256];
};

std::string getBoincFilename(std::string filename) throw(std::runtime_error);
double calculateFPS();
double getTimeInSeconds();

#endif //BOINC_UTILS_H
