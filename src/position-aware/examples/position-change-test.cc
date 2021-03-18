#include "ns3/config.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/nstime.h"
#include "ns3/position-aware-helper.h"
#include "ns3/position-aware.h"
#include "ns3/string.h"
#include "ns3/vector.h"
#include <iostream>
using namespace ns3;

class PositionChange
{
protected:
  using ThisType = PositionChange;

public:
  void PositionChangeCallback (Ptr<const PositionAware> _position_aware);
  void TimeoutCallback (Ptr<const PositionAware> _position_aware);

  void Create ();
  void Run ();

  Vector3D lastPosition;
  Time lastTime;
  NodeContainer nodes_;
};

int
main (int argc, char **argv)
{
  PositionChange test;
  test.Create ();
  test.Run ();
  return 0;
}

void
PositionChange::Create ()
{
  std::cout << "Creating Nodes" << std::endl;
  nodes_.Create (2);

  std::cout << "Installing Mobility" << std::endl;
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator");
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes_.Get (0));
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (nodes_.Get (1));
  nodes_.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (
      ns3::Vector3D (100.0, 0.0, 0.0));

  std::cout << "Install Position Aware" << std::endl;
  PositionAwareHelper position_aware (Seconds (4), 50.0);
  position_aware.Install (nodes_);

  std::cout << "Connecting Callbacks" << std::endl;
  nodes_.Get (0)->GetObject<PositionAware> ()->TraceConnectWithoutContext (
      "Timeout", MakeCallback (&ThisType::TimeoutCallback, this));
  nodes_.Get (1)->GetObject<PositionAware> ()->TraceConnectWithoutContext (
      "PositionChange", MakeCallback (&ThisType::PositionChangeCallback, this));

  lastPosition = nodes_.Get (1)->GetObject<MobilityModel> ()->GetPosition ();
  lastTime = ns3::Time ("0s");
}

void
PositionChange::Run ()
{
  Simulator::Stop (Seconds (12));
  Simulator::Run ();
  Simulator::Destroy ();
}

void
PositionChange::PositionChangeCallback (Ptr<const PositionAware> _position_aware)
{
  Ptr<Node> node = _position_aware->GetObject<Node> ();
  Ptr<MobilityModel> mobility = _position_aware->GetObject<MobilityModel> ();
  std::cout << "[Node " << node->GetId () << "]"
            << " Position Change: " << mobility->GetPosition () << std::endl;
  if (50.0 != CalculateDistance (lastPosition, mobility->GetPosition ()))
    {
      std::cerr << "Position change error" << std::endl;
      exit (-2);
    }
  lastPosition = mobility->GetPosition ();
}

void
PositionChange::TimeoutCallback (Ptr<const PositionAware> _position_aware)
{
  Ptr<Node> node = _position_aware->GetObject<Node> ();
  Ptr<MobilityModel> mobility = _position_aware->GetObject<MobilityModel> ();
  std::cout << "[Node " << node->GetId () << "]"
            << " Timeout" << std::endl;
  if (Seconds (4) != Simulator::Now () - lastTime)
    {
      std::cerr << "Timeout at wrong time" << std::endl;
      exit (-1);
    }
  lastTime = Simulator::Now ();
}