#ifndef __DRIVER_H__
#define __DRIVER_H__
#include "Robot.h"
#include "PointSequence.h"

class Driver {
private:
    bool _isDriving;

    Robot _robot;
public:
    Driver();
    virtual ~Driver();
    void loop();

    void trace_path(const PointSequence& path);
};



#endif // __DRIVER_H__