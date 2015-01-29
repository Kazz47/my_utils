#ifndef EVENT_H
#define EVENT_H

#include "event_type.hpp"

class Event {
    private:
        int type_id;
        int start_time;
        int end_time;

    public:
        Event(const int, const int, const int);
        void setTypeId(const int);
        void setStartTime(const int);
        void setEndTime(const int);
        int getTypeId();
        int getStartTime();
        int getEndTime();
};

#endif //EVENT_H
