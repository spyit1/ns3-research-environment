/*
 * test.cc
 *
 *  Created on: 2015/06/05
 *      Author: terami
 */

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* test-proto.cc: simulate PROTO routing protocol
 * F.Qian, Jun. 2013
 *
 *               m2    m3
 *   172.16.2.0/24 \   / 172.16.3.0/24
 *       (p2p)      \ /      (p2p)
 *                   m1
 *                   |
 *            (p2p)  | 172.16.1.0/24
 *                   R
 *                   |
 *  192.168.1.0/24   |
 *    ---+-------+---+---+-------+---(CSMA)
 *       |       |       |       |
 *      n0      n1      n2      n3 192.168.1.4
 */

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include <cmath>
#include "ns3/energy-module.h"
#include "ns3/random-waypoint-mobility-model.h"
#include "ns3/waypoint.h"
#include "ns3/waypoint-mobility-model.h"
#include "ns3/random-variable-stream.h"


#include "simple-helper.h"
#include "simple-packet.h"
#include "simple-rtable.h"
#include "simple-protocol.h"
#include "myuser.h"

#include <iostream>
#include <string>
#include <sstream>
#include <time.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("test");

class NetSim {
public :
	// initialize simulation parameters
	NetSim();
	~NetSim();

	// command line options processing
	void Configure(int, char **);

	// run simulation
	void RunSimulation ();
	void Initialize_ClusterView();

	uint32_t getsinkNode();

private :
	// configure default attributes
	void ConfigureSetDefault ();

	// make network topology
	void MakeNetworkTopology();

	// configure data link layer entities
	void ConfigureDataLinkLayer();

	// configure network layer entities
	void ConfigureNetworkLayer ();

	// configure transport layer entities and setup applications
	void ConfigureL4withDUPApp(NodeContainer, uint32_t, uint32_t);

	// generate application traffics send it to socket
	void GenerateTraffic (Ptr<Socket>, Time);

	// receive packets from socket
	void ReceivePacket (Ptr<Socket>);

	// set node's peer address as (IP address, port number)
	InetSocketAddress setSocketAddress(Ptr<Node>, uint32_t);

	// attach energy mode to devices with (initial energy, Tx current, Rx current)
	void AttachEnergyModelToDevices(double, double, double);
	// trace remained energy for node n[traceNum]
	void TraceRemainingEnergy(EnergySourceContainer);
	// trace total consumed energy for node n[traceNum]
	void TraceTotalEnergy(EnergySourceContainer);
	// get total consumed energy for node n[traceNum] with (oldValue, new Value)
	void TotalEnergy (double, double);
	// get remained energy for node n[traceNum] with (oldValue, new Value)
	void GetRemainingEnergy (double, double);
	// notify when node n[traceNum]'s energy was drained
	void NotifyDrained();

	// record ASCII/PCAP traces to files
	void traceSimEvents();
	// trace routing tables to file
	void traceRoutingTables();

	// set up flow monitor
	void SetFlowMonitor ();
	// get statistics for flow between node1 and node2
	void RunFlowMonitor (uint32_t, uint32_t);

private :

	uint32_t numRandomwayNodes;
	uint32_t numConstantNodes;
	uint32_t numWaypointNodes;
	uint32_t numMyUserNodes;

	double     distance;   // distance between two nodes in m
	double	simulationTime; // simulation time(s)
	uint32_t numPackets;   // number of nodes in the grid
	uint32_t packetSize;   // size of application packet sent in bytes
	uint32_t sourceNode;   // source node id
	uint32_t   sinkNode;   // destination node id
	double     interval;   // time interval between send packets
	bool        tracing;   // enable Routing Tables traces
	int nodeSpeed; //in m/s
	int nodePause; //in s
	std::string phyMode;   // Wifi Physical layer mode
	std::string traceEng;  // enable remained energy consumption traces
	uint32_t    traceNum;  // trace energy consumption for node
	std::string rtproto;   // set default routing protocol(Aodv,Dsdv,Olsr,Dsr)

	uint32_t totalReceived;// total received packets
	uint32_t totalSent;    // total sent packets

	Ptr<PositionAllocator> taPositionAlloc;
	ObjectFactory pos;
	std::stringstream ssSpeed;
	std::stringstream ssPause;
	std::stringstream ssField_x;
	std::stringstream ssField_y;

	std::string traceFile;

private :
	// node container for save all nodes
	NodeContainer randomwayNodes;
	NodeContainer constantNodes;
	NodeContainer waypointNodes;
	NodeContainer myuserNodes;
	// individual nodes, pressented as n[i], for i=0,1,...,num_node-1
	Ptr<Node> *n_randomway;
	Ptr<Node> *n_constant;
	Ptr<Node> *n_waypoint;
	Ptr<ns3::myuser::MyUser> *n_myuser;

	// network device container for all nodes
	NetDeviceContainer randomwayDevices;
	NetDeviceContainer constantDevices;
	NetDeviceContainer waypointDevices;
	NetDeviceContainer myuserDevices;

	// helper used to assign positions and mobility models to nodes
	MobilityHelper mobility;

	// helper for create and manage tans model PHY objects
	YansWifiPhyHelper wifiPhy;
	// helper for store IPv4 address information on an interface
	Ipv4InterfaceContainer ifs;

	SimpleHelper simple;

	FlowMonitorHelper  *flowmon;
	Ptr<FlowMonitor>   monitor;

	double _remainingEnergy;// save current remained energy for the node
	bool   _energyDrained;  // set it to true if energy is drained.
};

void NetSim::Initialize_ClusterView(){
	std::ofstream fout;
		fout.open("simpleaodv_broadcast_unicast_view.txt");

		if(!fout){
			exit(1);
		}
		fout<<numRandomwayNodes+numConstantNodes+numWaypointNodes+numMyUserNodes<<","<<"1000"<<","<<"1000"<<std::endl;
		fout.close();
}


NetSim::NetSim() : totalReceived(0), totalSent(0)
{
	numRandomwayNodes  = 0;
	numWaypointNodes = 0;
	numConstantNodes = 0;
	numMyUserNodes = 3;

	simulationTime = 500;
	distance   = 100;
	packetSize = 128;
	numPackets = 10;
	nodeSpeed = 2;
	nodePause = 0;
	sourceNode = 0;
	sinkNode   = 0;
	interval   = 0.5;
	tracing    = true;      // turn on routing table traces
	traceNum   = 12;        // trace energy consumption for center node
	phyMode    = std::string("DsssRate11Mbps");
	traceEng   = std::string("remained"); // trace remained energy consumption
	rtproto    = std::string("ers");     // set default routing protocol

}

NetSim::~NetSim()
{
	for (uint32_t i = 0; i < numRandomwayNodes; ++i) n_randomway[i] = 0;
	n_randomway=0;

	for (uint32_t i = 0; i < numConstantNodes; ++i) n_constant[i] = 0;
	n_constant=0;

	for (uint32_t i = 0; i < numWaypointNodes; ++i) n_waypoint[i] = 0;
	n_waypoint=0;

	for (uint32_t i = 0; i < numMyUserNodes; ++i) n_myuser[i] = 0;
	n_myuser=0;
}

void
NetSim::ConfigureSetDefault ()
{
		NS_LOG_FUNCTION(this);

	Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
	Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));
	Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue(150.0));
	//Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));
}

void
NetSim::MakeNetworkTopology()
{
	//NS_LOG_FUNCTION(this);

	n_randomway = new Ptr<Node> [numRandomwayNodes];
	for (uint32_t i = 0; i < numRandomwayNodes; ++i) {
		n_randomway[i] = CreateObject<Node> ();
		//n[i]->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
		//n[i]->AggregateObject (CreateObject<RandomWaypointMobilityModel> ());
		randomwayNodes.Add(n_randomway[i]);
	}

	n_waypoint = new Ptr<Node> [numWaypointNodes];
	for (uint32_t i = 0; i < numWaypointNodes; ++i) {
		n_waypoint[i] = CreateObject<Node> ();
		//n[i]->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
		//n[i]->AggregateObject (CreateObject<RandomWaypointMobilityModel> ());
		waypointNodes.Add(n_waypoint[i]);
	}

	n_myuser = new Ptr<ns3::myuser::MyUser> [numMyUserNodes];
	for (uint32_t i = 0; i < numMyUserNodes; ++i) {
		n_myuser[i] = CreateObject<ns3::myuser::MyUser> ();
		//n[i]->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
		//n[i]->AggregateObject (CreateObject<RandomWaypointMobilityModel> ());
		myuserNodes.Add(n_myuser[i]);
	}


	pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");

	ssField_x<<"ns3::UniformRandomVariable[Min=0.0|Max=1000]";
	ssField_y<<"ns3::UniformRandomVariable[Min=0.0|Max=1000]";

	pos.Set ("X", StringValue (ssField_x.str()));
	pos.Set ("Y", StringValue (ssField_y.str()));
	int64_t streamIndex = 0; // used to get consistent mobility across scenarios

	taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
	streamIndex += taPositionAlloc->AssignStreams (streamIndex);

//	int nodeSpeed = 2; //in m/s
//	int nodePause = 0; //in s

	ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
	ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";

	mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
			"Speed", StringValue (ssSpeed.str ()),
			"Pause",StringValue(ssPause.str()),
			"PositionAllocator", PointerValue (taPositionAlloc));
	std::cout << ssSpeed.str() << std::endl;
	mobility.SetPositionAllocator (taPositionAlloc);
	mobility.Install (randomwayNodes);
	streamIndex += mobility.AssignStreams (randomwayNodes, streamIndex);

	mobility.SetMobilityModel("ns3::WaypointMobilityModel",
		"InitialPositionIsWaypoint",
		BooleanValue(false));
	mobility.Install (waypointNodes);
	for(uint32_t i = 0; i < numWaypointNodes; i++){
		Ptr<WaypointMobilityModel> mob = n_waypoint[i]->GetObject<WaypointMobilityModel>();
		Waypoint wpt(Seconds(0.0), Vector(100*i,0,0));
		mob->AddWaypoint(wpt);
		Ptr<UniformRandomVariable> w_uniformRandomVariable = CreateObject<UniformRandomVariable>();
		uint32_t x = w_uniformRandomVariable->GetInteger(0,100);
		uint32_t y = w_uniformRandomVariable->GetInteger(0,100);
		Waypoint wpt2 (Seconds(1+Simulator::Now().GetInteger()/1000000000)+NanoSeconds(1),Vector(x,y,0));
		mob->AddWaypoint(wpt2);
	}

	mobility.SetMobilityModel("ns3::WaypointMobilityModel",
		"InitialPositionIsWaypoint",
		BooleanValue(false));
	mobility.Install (myuserNodes);
	for(uint32_t i = 0; i < numMyUserNodes; i++){
		Ptr<WaypointMobilityModel> mob = n_myuser[i]->GetObject<WaypointMobilityModel>();
		Waypoint wpt(Seconds(0.0), Vector(100*i,0,0));
		mob->AddWaypoint(wpt);
		n_myuser[i]->Setup(10);
	}

	n_constant = new Ptr<Node> [numConstantNodes];
	for (uint32_t i = 0; i < numConstantNodes; ++i) {
		n_constant[i] = CreateObject<Node> ();
		//n[i]->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
		//n[i]->AggregateObject (CreateObject<RandomWaypointMobilityModel> ());
		constantNodes.Add(n_constant[i]);
	}

	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue (0.0),
			"MinY", DoubleValue (0.0),
			"DeltaX", DoubleValue (distance),
			"DeltaY", DoubleValue (distance),
			"GridWidth", UintegerValue (10),
			"LayoutType", StringValue ("RowFirst"));
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (constantNodes);
}

void
NetSim::ConfigureDataLinkLayer()
{
	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel");

	YansWifiPhyHelper wifiPhy;
	// set it to zero; otherwise, gain will be added
	wifiPhy.Set ("RxGain", DoubleValue (-10) );
	// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
	wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
	wifiPhy.SetChannel (wifiChannel.Create ());

	// Add a non-QoS upper mac, and disable rate control
//	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
//	wifiMac.SetType ("ns3::AdhocWifiMac");
//
//	WifiHelper wifi;
//	wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
//	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
//			"DataMode",StringValue (phyMode),
//			"ControlMode",StringValue (phyMode));
//	// Set it to adhoc mode
//	randomwayDevices = wifi.Install (wifiPhy, wifiMac, randomwayNodes);
//	constantDevices = wifi.Install (wifiPhy, wifiMac, constantNodes);


	// Add a non-QoS upper mac, and disable rate control
	WifiMacHelper wifiMac;
	wifiMac.SetType ("ns3::AdhocWifiMac");

	WifiHelper wifi;
	wifi.SetStandard (WIFI_STANDARD_80211g);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
			"DataMode",StringValue (phyMode),
			"ControlMode",StringValue (phyMode));
	// Set it to adhoc mode
	randomwayDevices = wifi.Install (wifiPhy, wifiMac, randomwayNodes);
	constantDevices = wifi.Install (wifiPhy, wifiMac, constantNodes);
	waypointDevices = wifi.Install (wifiPhy, wifiMac, waypointNodes);
	myuserDevices = wifi.Install (wifiPhy, wifiMac, myuserNodes);
	
}


void
NetSim::NotifyDrained()
{
	//std::cout <<"Energy was Drained. Stop send.\n";
	_energyDrained = true;
}

void
NetSim::AttachEnergyModelToDevices(double initialEnergy, double txcurrent, double rxcurrent)
{
	// configure energy source
	BasicEnergySourceHelper basicSourceHelper;
	basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (initialEnergy));
	EnergySourceContainer e_randomway = basicSourceHelper.Install(randomwayNodes);
	EnergySourceContainer e_constant = basicSourceHelper.Install(constantNodes);
	EnergySourceContainer e_waypoint = basicSourceHelper.Install(waypointNodes);
	EnergySourceContainer e_myuser = basicSourceHelper.Install(myuserNodes);

	// transmit at 0dBm = 17.4mA, receive mode = 19.7mA
	WifiRadioEnergyModelHelper radioEnergyHelper;
	radioEnergyHelper.Set ("TxCurrentA", DoubleValue (txcurrent));
	radioEnergyHelper.Set ("RxCurrentA", DoubleValue (rxcurrent));
	radioEnergyHelper.SetDepletionCallback(
			MakeCallback(&NetSim::NotifyDrained, this));
	DeviceEnergyModelContainer deviceModelsR = radioEnergyHelper.Install (randomwayDevices, e_randomway);
	DeviceEnergyModelContainer deviceModelsC = radioEnergyHelper.Install (constantDevices, e_constant);
	DeviceEnergyModelContainer deviceModelsW = radioEnergyHelper.Install (waypointDevices, e_waypoint);
	DeviceEnergyModelContainer deviceModelsU = radioEnergyHelper.Install (myuserDevices, e_myuser);
}

void // Trace function for remaining energy at node.
NetSim::GetRemainingEnergy (double oldValue, double remainingEnergy)
{
	if(_energyDrained==false) {
		// show current remaining energy(J)
		//		std::cout << std::setw(10) << Simulator::Now ().GetSeconds ()
		//	 	<< "\t "
		//		<< remainingEnergy <<std::endl;
		_remainingEnergy = remainingEnergy;
	}
}

void
NetSim::ConfigureNetworkLayer ()
{
	InternetStackHelper internet;
	internet.SetRoutingHelper(simple);
	internet.Install(randomwayNodes);
	internet.Install(constantNodes);
	internet.Install(waypointNodes);
	internet.Install(myuserNodes);

	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.0.0.0", "255.0.0.0");
	ipv4.Assign (randomwayDevices);
	ipv4.Assign (constantDevices);
	ipv4.Assign (waypointDevices);
	ipv4.Assign (myuserDevices);
}

void
NetSim::TraceRemainingEnergy(EnergySourceContainer e)
{
	// all energy sources are connected to resource node n>0
	Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (e.Get (traceNum));
	basicSourcePtr->TraceConnectWithoutContext ("RemainingEnergy",
			MakeCallback (&NetSim::GetRemainingEnergy, this));
}

InetSocketAddress
NetSim::setSocketAddress(Ptr<Node> node, uint32_t port)
{
	Ipv4InterfaceAddress adr = node->GetObject <Ipv4> ()->GetAddress(1, 0);
	std::cout << "Set address " << adr << " to node[" << node->GetId() <<"]" << std::endl;
//	if(node->GetId()==sinkNode){
//		return InetSocketAddress (Ipv4Address::GetAny(), port);
//	}else
		return InetSocketAddress (Ipv4Address(adr.GetLocal()), port);
}



void
NetSim::ReceivePacket (Ptr<Socket> socket)
{
	Ptr<Packet> packet;
	Address from;

	while ((packet = socket->RecvFrom (from))) {
		//データパケットが宛先に届いた際の処理
		if (packet->GetSize () > 0) {
			printf("-------------------------------------------------------------\n");
			std::cout << "\nReceived Data_packet!!!!!  "<< Simulator::Now ().GetSeconds () << "s   "
					<< " received "<< packet->GetSize () << " byte  "
					<< std::endl;

#ifdef DEBUG
			InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (from);
			std::cout << setw(10) << Simulator::Now ().GetSeconds ()
       						<< " received "<< packet->GetSize ()
       						<< " bytes from: (" << iaddr.GetIpv4 ()
       						<< ", " << iaddr.GetPort () << ")"
       						<< std::endl;
#endif
			totalReceived++;
		}
	}
}

void
NetSim::GenerateTraffic (Ptr<Socket> socket, Time pktInterval)
{

	if (numPackets > 0) {
		socket->Send (Create<Packet> (packetSize));
		std::cout << "send packet" << std::endl;
		Simulator::Schedule (pktInterval, &NetSim::GenerateTraffic, this, socket, pktInterval);
		totalSent ++;
		numPackets --;
	} else {
		socket->Close ();
	}
}

void
NetSim::ConfigureL4withDUPApp(NodeContainer Nodes,uint32_t src, uint32_t dst)
{
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");


	Ptr<Socket> destination = Socket::CreateSocket (Nodes.Get(dst), tid);
	InetSocketAddress dstSocketAddr = setSocketAddress (Nodes.Get(dst), 8888);
	destination->Bind (dstSocketAddr);
	destination->SetRecvCallback (MakeCallback (&NetSim::ReceivePacket, this));

	// set sender(n0)'s socket
	Ptr<Socket> source = Socket::CreateSocket (Nodes.Get(src), tid);
	InetSocketAddress srcSocketAddr = setSocketAddress (Nodes.Get(src), 8888);
	source->Bind (srcSocketAddr);
	source->SetAllowBroadcast (true);
	source->Connect (dstSocketAddr);



	//データパケット生成間隔設定
	Time interPacketInterval = Seconds (interval);
	// Give Proactive algorithms some time to converge -- 20 seconds perhaps

	//データパケット生成時間設定
	Simulator::Schedule (Seconds (1.0), &NetSim::GenerateTraffic, this, source, interPacketInterval);
}

void NetSim::RunSimulation(){
//	NS_LOG_FUNCTION(this);

	ConfigureSetDefault();
	MakeNetworkTopology();
	ConfigureDataLinkLayer();
	//AttachEnergyModelToDevices(1.5, 0.0857, 0.0528);
	//	if (traceEng == "remained")
	//		TraceRemainingEnergy(e);
	//	else
	//		TraceTotalEnergy(e);
	ConfigureNetworkLayer ();
	if(numConstantNodes>0) ConfigureL4withDUPApp(constantNodes, sinkNode, sourceNode);
	if(numRandomwayNodes>0) ConfigureL4withDUPApp(randomwayNodes, sinkNode, sourceNode);




	Simulator::Stop (Seconds (simulationTime));
	Simulator::Run ();
	Simulator::Destroy ();
}

void
NetSim::Configure (int argc, char *argv[])
{
	CommandLine cmd;
	cmd.AddValue ("phyMode"   , "Wifi Physical layer mode", phyMode);
	cmd.AddValue ("distance"  , "distance (m)", distance);
	cmd.AddValue ("simulationTime"  , "Simulation Time (s)", simulationTime);
	cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
	cmd.AddValue ("numPackets", "total number of packets generated", numPackets);
	cmd.AddValue ("interval"  , "interval (seconds) between send packets", interval);
//	cmd.AddValue ("num_node"  , "total number of nodes on the Grid", (uint32_t)sim_environment.GetNumNode());
	cmd.AddValue ("numRandomwayNodes", "total number of random way nodes", numRandomwayNodes);
	cmd.AddValue ("numConstantNodes", "total number of constant nodes", numConstantNodes);
	cmd.AddValue ("nodeSpeed", "nodes move speed (m/s)", nodeSpeed);
	cmd.AddValue ("nodePause", "nodes pose time (s)", nodePause);
	cmd.AddValue ("sourceNode", "Sender's node number", sourceNode);
	cmd.AddValue ("sinkNode"  , "Receiver's node number", sinkNode);
	cmd.AddValue ("traceEng"  , "trace energy flag (remained|total)", traceEng);
	cmd.AddValue ("traceNode" , "trace energy for node", traceNum);
	cmd.AddValue ("rtProto"   , "set routing protocol(Olsr,Aodv,Dsdv)", rtproto);
	cmd.AddValue ("traceFile", "Ns2 movement trace file", traceFile);
	cmd.Parse (argc, argv);
}

uint32_t
NetSim::getsinkNode()
{
	return sinkNode;
}

int main(int argc, char **argv)
{
	//LogComponentEnable("ClusterProtocol",LOG_LEVEL_ALL);
	//LogComponentEnable("TypeHeader",LOG_LEVEL_INFO);
	//LogComponentEnable("MyRoutingTable",LOG_LEVEL_INFO);
	//LogComponentEnable("ProtoRequestQueue",LOG_LEVEL_INFO);




	RngSeedManager::SetSeed(2);

	NetSim sim;
	sim.Configure(argc, argv);

	sim.Initialize_ClusterView();


	sim.RunSimulation();


	return 0;
}









