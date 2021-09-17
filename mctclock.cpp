#include "mctclock.h"

MCTClock::MCTClock()
{


}

QString MCTClock::getTime(){
    QTime time = QTime::currentTime();

    return time.toString("hh:mm:ss");
}

