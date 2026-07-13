#ifndef MYUSER_H_
#define MYUSER_H_

#include "simple-rtable.h"
#include "simple-id-cache.h"
#include "simple-packet.h"

#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/timer.h"
#include "ns3/waypoint-mobility-model.h"

#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <math.h>
#include <string>
#include <sstream>
#include "data-buffer.h"
#include "data-counter.h"
#include "unicast-data-container.h"
#include <string>

namespace ns3 {
namespace myuser {

class MyUser : public Node
{
public:
    MyUser ();

    void Setup (uint32_t time);

    void StartMove (double fieldX, double fieldY,Time period = Seconds(10.0));

    void MoveOnce (uint32_t time = 10);

    Ipv4Address GetIpv4Address();


private:

    void UpdateTimer(uint32_t time);
    void StartTimer(uint32_t time);


    Timer m_timer;
    Timer updatetimer;
    Timer starttimer;

    Time m_period;
    double m_fieldX;
    double m_fieldY;

    Ptr<UniformRandomVariable> m_rng;

    Ipv4Address my_ip;

};

} //namespace myuser
} //namespace ns3

#endif /* MYUSER_H_ */
