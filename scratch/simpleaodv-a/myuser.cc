#include "myuser.h"

#include "ns3/mobility-model.h"
#include "ns3/simulator.h"
#include "ns3/waypoint.h"
#include "ns3/waypoint-mobility-model.h"
#include "ns3/vector.h"
#include <fstream>
#include "simple-protocol.h"


namespace ns3 {
namespace myuser {

void MyUser::Setup(uint32_t time)
{
    StartTimer(time);
}

  Ipv4Address MyUser::GetIpv4Address(){
  Ptr<Ipv4> ipv4 = this->GetObject<Ipv4>();
  Ptr<simple::RoutingProtocol> proto = ipv4->GetObject<simple::RoutingProtocol>();
  my_ip = proto->GetIpv4Address();

  return my_ip;
  }

MyUser::MyUser ()
  : m_period (Seconds (10.0)),
    m_fieldX (1000.0),
    m_fieldY (1000.0),
    m_rng (CreateObject<UniformRandomVariable> ())
{
  m_timer.SetFunction (&MyUser::MoveOnce, this);
  
}

void
MyUser::StartMove (double fieldX, double fieldY, Time period)
{
  m_fieldX = fieldX;
  m_fieldY = fieldY;
  m_period = period;

  StartTimer (10);

}

void
MyUser::MoveOnce (uint32_t time)
{

  std::cout << GetIpv4Address() << std::endl;

  Ptr<WaypointMobilityModel> mob = GetObject<WaypointMobilityModel> ();
  Ptr<UniformRandomVariable> w_uniformRandomVariable = CreateObject<UniformRandomVariable>();

  double x = w_uniformRandomVariable->GetInteger(0,1000);
  double y = w_uniformRandomVariable->GetInteger(0,1000);

  Waypoint wpt2(Seconds(time) + Simulator::Now()+ NanoSeconds(1),Vector(x,y,0));
  mob->AddWaypoint(wpt2);

  UpdateTimer(time);

}

void MyUser::UpdateTimer(uint32_t time)
{
    updatetimer.Cancel();
    updatetimer.SetFunction(&MyUser::MoveOnce,this);
    updatetimer.SetArguments(time);
    //updatetimer.Remove();
    updatetimer.Schedule(Seconds(time));

}

void MyUser::StartTimer(uint32_t time)
{
    starttimer.Cancel();
    starttimer.SetFunction(&MyUser::MoveOnce,this);
    starttimer.SetArguments(time);
    //starttimer.Remove();
    starttimer.Schedule(Seconds(time));

}

} // namespace myuser
} // namespace ns3
