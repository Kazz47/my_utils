#include "event.hpp"

Event::Event() {
}

Event::Event(const int type_id, const int start_time, const int end_time) {
    this->type_id = type_id;
    this->start_time = start_time;
    this->end_time = end_time;
}

// Accessors

void Event::setTypeId(const int type_id) {
    this->type_id = type_id;
}

void Event::setStartTime(const int start_time) {
    this->start_time = end_time;
}

void Event::setEndTime(const int end_time) {
    this->end_time = end_time;
}

int Event::getTypeId() {
    return this->type_id;
}

int Event::getStartTime() {
    return this->start_time;
}

int Event::getEndTime() {
    return this->end_time;
}

