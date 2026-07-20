/*
 * test.cc
 *
 *  Created on: 2015/06/05
 *      Author: terami
 */

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
#include "ns3/waypoint-mobility-model.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <random>
#include <fstream>
#include <algorithm>
#include <map>
#include <random>

#include "json_data.h"
//#include "simple-helper.h"
//#include "simple-protocol.h"
#include "DTN-helper.h"
#include "DTN-protocol.h"
#include "write_log.h"
#include "DTN-helper.h"
#include "DTN-protocol.h"

#define SIMPLE 1
#define DTN 2

//using namespace ns3;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("test");


class NetSim {
public :
	// initialize simulation parameters
	NetSim();
	~NetSim();

	// command line options processing
	void Configure(int, char **);

	// JSON Data Input
	void JSON_Data_Input();

	// run simulation
	void RunSimulation ();
	void Initialize_ClusterView();

	// Output Logfile Initialization
	void Initialize_Logfile();
	void Initialize_Logfile_for_Recive();
	void Initislize_logfile_for_TestLog();
	void Initialize_Logfile_for_TotalDistance();
	void Initialize_Logfile_for_Block_Nodes();
	void Initialize_Logfile_for_Exit_Nodes();
	void Initialize_Logfile_for_AoiInfo();
	void Initialize_flow_Moniter();
	void Initialize_Logfile_SpeedInfo();
	void Initialize_Logfile_for_HelloRecvTimeInfo();
	void Initialize_Logfile_for_HelloSendTimeInfo();
	void Write_crossing_latlon();


	uint32_t GetSimTime (void) {return simtime;}
//	simple::JSON_Data GetJSONData(void){return jd;}

private :

	// configure default attributes
	void ConfigureSetDefault ();

	std::mt19937 create_rand_engine();
	std::vector<int> make_rand_array_select(const size_t size, int rand_min, int rand_max);
	int get_rand_range(uint32_t seed, int min_val, int max_val);
	std::vector<int> get_rand_pseudorandom(const size_t size, int min_val, int max_val, uint32_t seed);

	void MakeNetworkTopologyforJSON();

	// make network topology
	void MakeNetworkTopology();

	// configure data link layer entities
	void ConfigureDataLinkLayer();

	// configure network layer entities
	void ConfigureNetworkLayer ();

	// configure trasport layer entities and setup applications
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

	void Initialize_Logfile_for_ClusterMemberAverage();
	void Initialize_Logfile_for_SendSectionDataSize();
	void Initialize_Logfile_for_EvacuationTime();
	void Initialize_Logfile_for_BlockedAoI();
	//void Initislize_logfile_for_TestLog();

	void OutputClusterMemberSize();
	void OutputClusterHopInfo();
	void OutputUserDataInfo();
	void WriteBlockedNodeKnowledgeLog();

	void GenerateRandomNodePosition (std::map<std::string, double> &);
	void GenerateNodePosition (std::map<std::string, double> &);
	void OutputResult ();

	//2026-04-14
	void StartAddUserLoop();
	void ActivateReservedAddUsers();
	void Initialize_Logfile_for_AddUserTime();

private :

	//uint32_t numRandomwayNodes;
	//uint32_t numConstantNodes;
	u_int32_t mode;
	double distance;   // distance between two nodes in m
	double field_X;
	double field_Y;
	uint32_t run;	
	uint32_t seed;
	uint32_t simtime = 1000;
	uint32_t delay_ratio;
	uint32_t numPackets;   // number of nodes in the grid
	uint32_t packetSize;   // size of application packet sent in bytes
	uint32_t sourceNode;   // source node id
	uint32_t   sinkNode;   // destination node id
	double     interval;   // time interval between send packets
	bool        tracing;   // enable Routing Tables traces
	std::string phyMode;   // Wifi Physical layer mode
	std::string traceEng;  // enable remained energy consumption traces
	uint32_t    traceNum;  // trace energy consumption for node
	std::string rtproto;   // set default routing protocol(Aodv,Dsdv,Olsr,Dsr)

	uint32_t totalReceived;// total received packets
	uint32_t totalSent;    // total sent packets

	// uint32_t frequency; //通信回数

	Ptr<PositionAllocator> taPositionAlloc;
	ObjectFactory pos;
	std::stringstream ssSpeed;
	std::stringstream ssPause;
	std::stringstream ssField_x;
	std::stringstream ssField_y;

	std::string traceFile;
	std::string io;
	std::string useexitinfo;
	std::string usequickhello;
	std::string traceuserid;
	std::string useeraseinfo;
	uint32_t A_d;
	std::string usehop;

private :
	// node container for save all nodes
	NodeContainer userNodes;
	NodeContainer adduserNodes;
	NodeContainer ferryNodes;
	NodeContainer accessPointNodes;

	// individual nodes, pressented as n[i], for i=0,1,...,num_node-1
	Ptr<simple::MyUser> *n_user;
	Ptr<simple::MyUser> *n_ferry;
	Ptr<simple::MyUser> *add_user;
	Ptr<simple::MyAccessPoint> *n_accessPoint;

	// network device container for all nodes
	NetDeviceContainer userDevices;
	NetDeviceContainer ferryDevices;
	NetDeviceContainer accessPointDevices;

	// helper used to assign positions and mobility models to nodes
	MobilityHelper mobility;

	// helper for create and manage tans model PHY objects
	YansWifiPhyHelper wifiPhy;

	// helper for store IPv4 address information on an interface
	Ipv4InterfaceContainer ifs;

	//SimpleHelper simple;
	DTNHelper dtn;

	FlowMonitorHelper  *flowmon;
	Ptr<FlowMonitor>   monitor;

	double _remainingEnergy;// save current remained energy for the node
	bool   _energyDrained;  // set it to true if energy is drained.

};

void NetSim::JSON_Data_Input(){
	simple::JSON_Data::json_input();
}

void NetSim::Initialize_ClusterView(){
	char filename[1000];
	std::ostringstream oss_delay, oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}

	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/"  + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterViewer/ClusterViewer_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename,std::ios::out);
	if(!fout){
		std::cout<<"Could not open file."<< filename << std::endl;
		exit(1);
	}
//	fout<<simple::MyBuilding::GetNumUsers()+simple::MyBuilding::GetNumSections()<<","<<"1000"<<","<<"1000"<<std::endl;
	fout<<simple::MyBuilding::GetNumUsers()+simple::MyBuilding::GetNumSections()<<","<<(int)simple::JSON_Data::json_x(simple::JSON_Data::get_max_lo())<<","<<(int)simple::JSON_Data::json_y_size(simple::JSON_Data::get_max_la())<<std::endl;

	fout << "NI, 0, 10000,   0,   0, 0, READY, 0, 0" << std::endl;
	// std::cout<<std::to_string(simple::JSON_Data::get_min_la())<<","<<std::to_string(simple::JSON_Data::get_min_lo())<<std::endl;
	fout << "NI, 0, 10001, "<<simple::JSON_Data::json_x(simple::JSON_Data::get_max_lo())<<",  0, 0, READY, 0, 0" << std::endl;
	// std::cout<<std::to_string(simple::JSON_Data::get_max_la())<<","<<std::to_string(simple::JSON_Data::get_max_lo())<<std::endl;
	fout << "NI, 0, 10002, "<<simple::JSON_Data::json_x(simple::JSON_Data::get_max_lo())<<", "<<simple::JSON_Data::json_y_size(simple::JSON_Data::get_max_la())<<", 0, READY, 0, 0" << std::endl;
	// std::cout<<std::to_string(simple::JSON_Data::get_max_la())<<","<<std::to_string(simple::JSON_Data::get_min_lo())<<std::endl;
	fout << "NI, 0, 10003,   0, "<<simple::JSON_Data::json_y_size(simple::JSON_Data::get_max_la())<<", 0, READY, 0, 0" << std::endl;
	// std::cout<<std::to_string(simple::JSON_Data::get_min_la())<<","<<std::to_string(simple::JSON_Data::get_max_lo())<<std::endl;

//	std::list<json> nodes=simple::JSON_Data::get_nodes();
//	std::list<simple::way_node> no =simple::JSON_Data::get_waynodes();

	std::map<std::pair<double,double>,simple::way_node> no =simple::JSON_Data::get_waynodes();
	std::map<String,simple::way_info> wi =simple::JSON_Data::get_linestrings();
	std::map<std::pair<double,double>,simple::way_node> gr = simple::JSON_Data::get_graph();


	int i = 10004;


	std::list<String> nodeids;
	std::list<std::pair<double,double>> hinan =simple::JSON_Data::get_hinanjo();
	std::map<std::pair<double,double>, uint32_t> m_capacity = simple::JSON_Data::get_capacity();
	uint32_t capacity_num;
	std::map<std::pair<double,double>, uint32_t> capacity_map;
	std::set<std::pair<double,double>> enternode;

///////////////////////////////避難所近くのノードを取得////////////////////////////////////
	if(simple::MyBuilding::GetExitAutoflag()){
		for(auto itr = hinan.begin();itr != hinan.end();++itr){
			std::pair<double,double> hinanjo = *itr;
			capacity_num = m_capacity.at(hinanjo);
			std::pair<double,double> enter;
			double nearly = (double)INF;
			for(auto itr2 = gr.begin();itr2 != gr.end();++itr2){
				std::pair<double,double> node = itr2->first; //緯度から簡易的に近くのノードを探す？
				// std::cout<<node.first<<", "<<node.second<<std::endl;
				double dist = simple::JSON_Data::calculate_distance(node,hinanjo);
				if(dist < nearly){
					nearly = dist;
					enter = node;
				}
			}
			if(nearly<simple::MyBuilding::Getneary()) {
				enternode.insert(enter);
				capacity_map.insert(std::make_pair(enter,capacity_num));
			}
			// if(nearly<200) enternode.insert(enter);
		}
	}

	// for(auto a : enternode){
	// 	std::cout<<a.first<<", "<<a.second<<std::endl;
	// 	// simple::JSON_Data::GetIDfromCoordinate(a);
	// 	String id = simple::JSON_Data::GetIDfromCoordinate(a);
	// 	std::cout<<id<<std::endl;
	// 	simple::MyBuilding::SetExitNodesAuto(id, 1000); //避難可能人数は1000人としている
	// }
///////////////////////////////////////////////////////////////////////////////////////////////////////////

    for(auto itr = no.begin();itr != no.end();++itr){
    	std::pair<double,double> node = itr->first;
    	simple::way_node wayn = itr->second;
    	String flg="FALSE";
	   	String id = "node/"+std::to_string(i);
		if(simple::MyBuilding::GetExitAutoflag()){
			decltype(enternode)::iterator itr3 = enternode.find(node);
			if(itr3!=enternode.end()){
				flg="EXIT";
				std::cout<<id<<std::endl;
				uint32_t cap_num = capacity_map.at(node);
				//uint32_tに-1が代入されると4294967295となる = ここでは実質-１であるかどうかの判定をしている
				if(cap_num == 4294967295) cap_num = 100; //元データが-1の場合避難可能人数を100に設定する
				simple::MyBuilding::SetExitNodesAuto(id,cap_num);
			}
		}

		std::map<String, uint32_t> exits = simple::MyBuilding::GetExitNodes();
		decltype(exits)::iterator itr2 = exits.find(id);
		if(itr2 != exits.end()){
			flg="EXIT";
		}

    	if(wayn.GetCrossingflag()) {
    		flg="TRUE";
			String id = "node/"+std::to_string(i);
			// if(simple::MyBuilding::GetExitAutoflag()){
			// 	decltype(enternode)::iterator itr3 = enternode.find(node);
			// 	if(itr3!=enternode.end()){
			// 		std::cout<<id<<std::endl;
			// 		simple::MyBuilding::SetExitNodesAuto(id,1000);
			// 	}
			// }
//    		for(auto itr3 = hinan.begin();itr3 != hinan.end();++itr3){
//    			std::pair<double,double> hinanjo = *itr3;
//    			double dist = simple::JSON_Data::calculate_distance(node,hinanjo);
//    			if(dist<150){
//    				simple::MyBuilding::SetExitNodes(id,100);
//    			}
//    		}
    	}

//    	nodeid = "node/" + std::to_string(i);
//    	nodeids.push_back(nodeid);

    	fout << "NI, 0, "<<i<<", "<<simple::JSON_Data::json_x(node.first)<<", "<<simple::JSON_Data::json_y(node.second)<<", 0, "<< flg<<", 0, 0"<< std::endl; //ノードの表示
    	simple::JSON_Data::SetNodeID(i,node);
    	i++;
    }

    std::cout<<"escape"<<std::endl;
    std::map<String, uint32_t> escape = simple::MyBuilding::GetExitNodes();
    std::cout<<escape.size()<<std::endl;
	uint32_t canexitnum = 0;
    for(auto itr = escape.begin();itr != escape.end();++itr){
    	String it = itr->first;
    	std::cout<<it<<":("<<simple::JSON_Data::GetCoordinatefromID(it).first<<", "<<simple::JSON_Data::GetCoordinatefromID(it).second<<")"<<std::endl;
		std::cout<<"収容可能人数："<<itr->second<<std::endl;
		canexitnum = canexitnum + itr->second;
    }
	std::cout<<"合計収容可能人数："<<canexitnum<<std::endl;
	if(canexitnum < simple::MyBuilding::GetNumUsers()+simple::MyBuilding::GetAddUsersNum()){
		std::cout<<"収容可能人数以上のユーザがいるため終了します"<<std::endl;
		exit(1);
	}
    std::cout<<"blocked"<<std::endl;


    std::list<String> blocked = simple::MyBuilding::GetBlockedNodes();
    // for(auto itr = blocked.begin();itr != blocked.end();++itr){
    // 	String it = *itr;
    // 	std::cout<<it<<":("<<simple::JSON_Data::GetCoordinatefromID(it).first<<", "<<simple::JSON_Data::GetCoordinatefromID(it).second<<")"<<std::endl;
    // }

//    for(auto itr = nodeids.begin();itr != nodeids.end();++itr){
//    	String it = *itr;
//    }

    for(auto itr = hinan.begin();itr != hinan.end();++itr){
    	std::pair<double,double> node = *itr;
    	fout << "NI, 0, "<<i<<", "<<simple::JSON_Data::json_x(node.first)<<", "<<simple::JSON_Data::json_y(node.second)<<", 0, AP, 0, 0 "<< std::endl;
    	i++;
    }

    simple::JSON_Data::SetNodenum(i);
//    simple::JSON_Data::SetNodeID();

    no =simple::JSON_Data::get_waynodes();
    for(auto itr=wi.begin();itr!=wi.end();++itr){
    	std::list<simple::way_node> it = itr->second.GetNodes();
    	int n = 0;
    	for(auto itr2=it.begin();itr2!=it.end();++itr2){
    		std::pair<double,double> it2 = itr2->GetCoordinate();
    		if(n==0){
    			n = simple::JSON_Data::NodeIDstoi(no.at(it2).GetNodeID());
    		}else{
    			fout << "NL, 0, "<<n<<", "<< simple::JSON_Data::NodeIDstoi(no.at(it2).GetNodeID())<<", 1" << std::endl;
    			n = simple::JSON_Data::NodeIDstoi(no.at(it2).GetNodeID());
    		}
    	}
    }


	// output text - NI
//	fout << "NI, 0, 1000,   0,   0, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1001, 100,   0, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1002, 100, 100, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1003,   0, 100, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1004,  60,   0, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1005,  60,  20, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1006,  20,  20, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1007,  40,  20, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1008,  20,  40, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1009,  80,  40, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1010,  80,  20, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1011,   0,  60, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1012,  20,  60, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1013,  80,  60, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1014, 100,  60, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1015,  40,  60, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1016,  40,  80, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1017,  20,  80, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1018,  60,  80, 0, READY, 0, 0" << std::endl;
//	fout << "NI, 0, 1019,  60, 100, 0, READY, 0, 0" << std::endl;
	// output text - NL
	fout << "NL, 0, 10000, 10001, 1" << std::endl;
	fout << "NL, 0, 10001, 10002, 1" << std::endl;
	fout << "NL, 0, 10002, 10003, 1" << std::endl;
	fout << "NL, 0, 10003, 10000, 1" << std::endl;
//	fout << "NL, 0, 1004, 1005, 1" << std::endl;
//	fout << "NL, 0, 1006, 1007, 1" << std::endl;
//	fout << "NL, 0, 1008, 1009, 1" << std::endl;
//	fout << "NL, 0, 1009, 1010, 1" << std::endl;
//	fout << "NL, 0, 1011, 1012, 1" << std::endl;
//	fout << "NL, 0, 1013, 1014, 1" << std::endl;
//	fout << "NL, 0, 1015, 1016, 1" << std::endl;
//	fout << "NL, 0, 1016, 1017, 1" << std::endl;
//	fout << "NL, 0, 1018, 1019, 1" << std::endl;
	fout.close();
//	fout.close();
}


void NetSim::Write_crossing_latlon(){
	char filename[1000];
	std::ostringstream oss_run;
	std::string str = "obayashiIOFiles/Log/obayashi/crossing_latlon.txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	if(!fout){
		std::cout<<"Could not open file(Write_crossing_latlon)."<<std::endl;
		exit(1);
	}

	std::map<std::pair<double,double>,simple::way_node> no =simple::JSON_Data::get_waynodes();
	
	for(auto itr = no.begin();itr != no.end();++itr){
		std::pair<double,double> node = itr->first;
    	simple::way_node wayn = itr->second;
		
		if(wayn.GetCrossingflag()) {
			fout << std::setprecision(16) << node.first<<", "<< std::setprecision(16) << node.second << std::endl; //ノードの表示
		}
	}
	fout.close();
	std::cout<<"OK!"<<std::endl;
}


void NetSim::Initialize_flow_Moniter(){
	String mode="ON";
	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
		return;
	}

	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag() == true){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	// std::string str = "obayashiIOFiles/Log/Block_Nodes.txt";
	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/"+eraseblock+"/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Info/Flow_moniter/flowMoniter_" +std::to_string(simple::MyBuilding::GetRun()) +".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	fout<<"flow_moniter"<<std::endl;
}

void NetSim::Initialize_Logfile_for_AoiInfo(){
	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";
	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}
	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" +eraseblock+"/"+ std::to_string(simple::MyBuilding::GetNumUsers()) + "/AoI_Info/AoI_Info_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename,std::ios::out);
	fout<<"time,userID,x,y,AoI,exitnode"<<std::endl;
	fout.close();


	char filename2[1000];
	std::string str2 = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/"+ eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/AoI_Info/moving_user_nun_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str2.copy(filename2,str2.size());
	filename2[str2.size()] = '\0';

	std::ofstream fout2;
	fout2.open(filename2,std::ios::out);
	fout2<<"time,shelterID,numusers"<<std::endl;
	fout2.close();	

	char filename3[1000];
	std::string str3 = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/AoI_Info/all_moving_user_nun_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str3.copy(filename3,str3.size());
	filename3[str3.size()] = '\0';

	std::ofstream fout3;
	fout3.open(filename3,std::ios::out);
	fout3<<"time,numusers"<<std::endl;
	fout3.close();	

	char filename4[1000];
	std::string str4 = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello +"/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/AoI_Info/ALL_AoI_Info_"  +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str4.copy(filename4,str4.size());
	filename4[str4.size()] = '\0';
	std::ofstream fout4;
	fout4.open(filename4,std::ios::out);
	fout4<<"time,userID,x,y,AoI,exitnode"<<std::endl;
	fout4.close();

	char filename5[1000];
	std::string str5 = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello +"/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/AoI_Info/Block_AoI_Info_"  +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str5.copy(filename5,str5.size());
	filename5[str5.size()] = '\0';
	std::ofstream fout5;
	fout5.open(filename5,std::ios::out);
	fout5<<"time,userID,x,y,AoI,blocknode"<<std::endl;
	fout5.close();

	char filename6[1000];
	std::string str6 = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello +"/" +eraseblock+ "/"  + std::to_string(simple::MyBuilding::GetNumUsers()) + "/AoI_Info/ALL_Blocked_AoI_Info_"  +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str6.copy(filename6,str6.size());
	filename6[str6.size()] = '\0';
	std::ofstream fout6;
	fout6.open(filename6,std::ios::out);
	fout6<<"time,userID,x,y,AoI,blocknode"<<std::endl;
	fout6.close();

}

void NetSim::Initialize_Logfile_for_AddUserTime()
{
	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();

	String mode = "ON";
	if(simple::RoutingProtocol::GetMode()){
		mode = "ON";
	}else{
		mode = "OFF";
	}

	String quickhello = "QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello = "QuickHello_ON";
	}else{
		quickhello = "QuickHello_OFF";
	}

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock = "EraseBlock_ON";
	}else{
		eraseblock = "EraseBlock_OFF";
	}

	std::string str =
		"obayashiIOFiles/Log/obayashi/" + std::string(mode) + "/" +
		std::string(quickhello) + "/" + std::string(eraseblock) + "/" +
		std::to_string(simple::MyBuilding::GetNumUsers()) +
		"/Time/AddUserTime_" + std::to_string(simple::MyBuilding::GetRun()) + ".txt";

	str.copy(filename, str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename, std::ios::out);
	fout << "time,userID,index" << std::endl;
	fout.close();
}

void NetSim::Initialize_Logfile_SpeedInfo(){
	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";
	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}

	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}
	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello +"/" +eraseblock+ "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Info/Speed/UserSpeed_"  +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	fout<<"UserId,Speed"<<std::endl;
	fout.close();	
}

void NetSim::Initialize_Logfile_for_Recive(){
	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}
	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Recive/recive_data" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	fout << "sender,reciver,time" << std::endl;
	fout.close();
}

void NetSim::Initialize_Logfile_for_HelloRecvTimeInfo(){
	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}
	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/HelloTimeInfo/HelloRecv/RecvHelloTime" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	fout << "sender,reciver,time" << std::endl;
	fout.close();
}

void NetSim::Initialize_Logfile_for_HelloSendTimeInfo(){
	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}
	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/HelloTimeInfo/HelloSend/SendHelloTime" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	fout << "sender,time,quicksendflag" << std::endl;
	fout.close();
}



void NetSim::Initislize_logfile_for_TestLog(){
	char filename[1000];
	std::ostringstream oss_delay, oss_run;
//	oss_delay << simple::MyBuilding::GetFloodingInterval();
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}
	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/testLog/Recv_data_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	if(!fout){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	fout << "初期化" << std::endl;
	fout.close();

	char filename2[1000];
	std::string str2 = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/"+ eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/testLog/testlog_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	// std::string str2 = "obayashiIOFiles/Log/obayashi/ON/testLog/testlog.txt";
	str2.copy(filename2,str2.size());
	filename2[str2.size()] = '\0';
	std::ofstream fout2;
	fout2.open(filename2,std::ios::out);
	if(!fout2){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	fout2 << "初期化" << std::endl;
	fout2.close();
}

void NetSim::Initialize_Logfile_for_TotalDistance(){
	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}
	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/UserInfo/Total_move_distance" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	fout << "UserID, total_distance" << std::endl;
	fout.close();
}

void NetSim::Initialize_Logfile_for_Block_Nodes(){
	String mode="ON";
	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
		return;
	}

	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	


	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	// std::string str = "obayashiIOFiles/Log/Block_Nodes.txt";
	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Info/Block/BlockNode_" +std::to_string(simple::MyBuilding::GetRun()) +".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	fout << "Block_Nodes" << std::endl;
	fout.close();

	char filename2[1000];
	std::string str2 = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Info/Block/Block_Nodes_xy" +std::to_string(simple::MyBuilding::GetRun()) +".txt";
	str2.copy(filename2,str2.size());
	filename2[str2.size()] = '\0';
	fout.open(filename2,std::ios::out);
	fout << "blocknodeid,x,y" << std::endl;
	for(auto a : simple::MyBuilding::GetBlockedNodes2()){
		fout << a <<","<< simple::JSON_Data::json_x(simple::JSON_Data::GetCoordinatefromID(a).first)<<","<<simple::JSON_Data::json_y(simple::JSON_Data::GetCoordinatefromID(a).second)<<std::endl;
	}
	fout.close();
}

void NetSim::Initialize_Logfile_for_Exit_Nodes(){
	String mode="ON";
	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
		return;
	}

	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	// std::string str = "obayashiIOFiles/Log/Exit_Nodes.txt";
	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Info/Exit/Exit_Nodes" +std::to_string(simple::MyBuilding::GetRun()) +".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	fout << "Exit_Nodes" << std::endl;
	std::map<String, uint32_t> exit_nodes = simple::MyBuilding::GetExitNodes();
	for(auto a : exit_nodes){
		fout << a.first <<", "<< a.second << std::endl;
	}
	fout.close();

	char filename2[1000];
	std::string str2 = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Info/Exit/Exit_Nodes_xy" +std::to_string(simple::MyBuilding::GetRun()) +".txt";
	str2.copy(filename2,str2.size());
	filename2[str2.size()] = '\0';
	fout.open(filename2,std::ios::out);
	fout << "exitnodeid,x,y" << std::endl;
	for(auto a : exit_nodes){
		fout << a.first <<","<< simple::JSON_Data::json_x(simple::JSON_Data::GetCoordinatefromID(a.first).first)<<","<<simple::JSON_Data::json_y(simple::JSON_Data::GetCoordinatefromID(a.first).second)<<std::endl;
	}
	fout.close();
}


void NetSim::Initialize_Logfile(){
//	WriteLog::Initialize_Logfile_for_ClusterMemberAverage();
	WriteLog::Initialize_Logfile_for_SendSectionDataSize();
	WriteLog::Initialize_Logfile_for_EvacuationTime();
	WriteLog::Initialize_Logfile_for_EvacuationTime2();
	WriteLog::Initialize_Logfile_for_EscapeInfo();
	WriteLog::Initialize_Logfile_for_ClusterMemberSize();
	WriteLog::Initialize_Logfile_for_ClusterHopInfo();
	WriteLog::Initialize_Logfile_for_UserTrackingLog();
	//WriteLog::Initislize_logfile_for_TestLog();
	//WriteLog::Initislize_logfile_for_Recive();
}

void NetSim::OutputClusterMemberSize(){
	int64_t time = Simulator::Now().GetInteger();
	int64_t roughTime = time / 1000000000;
	int64_t splitSecond = time % 1000000000;
	char filename[1000];
	std::ostringstream oss_delay,oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}

	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterInfo//ClusterMemberSize/ClusterMemberSize_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::map<Ipv4Address,int> m1;
	std::map<int,int> m2;
	for(uint32_t i=0; i<simple::MyBuilding::GetNumUsers(); ++i){
		Ptr<simple::RoutingProtocol> obj = n_user[i]->GetObject<simple::RoutingProtocol>();
		Ptr<simple::MyUser> user = n_user[i]->GetObject<simple::MyUser>();
		Ipv4Address add = obj->GetClusterID();
		decltype(m1)::iterator itr = m1.find(add);
		if(itr!=m1.end()){
			itr->second++;
		}else if(user->GetStatus()!=3){
			m1.insert(std::make_pair(add,1));
		}
	}
	for(auto itr = m1.begin();itr!=m1.end();++itr){
		std::pair<Ipv4Address,int> it = *itr;
		decltype(m2)::iterator itr2 = m2.find(it.second);
		if(itr2!=m2.end()){
			itr2->second++;
		}else {
			m2.insert(std::make_pair(it.second,1));
		}
	}

	String output_text = std::to_string(roughTime) + "." + std::to_string(splitSecond);
	if(!m2.empty()){
		int c_max = m2.rbegin()->first;
		for(int i = 1;i<=c_max;i++){
			decltype(m2)::iterator itr = m2.find(i);
			if(itr!=m2.end()){
				std::pair<int,int> it = *itr;
				output_text=output_text+", "+std::to_string(it.first) + ", " +std::to_string(it.second);
			}else{
				output_text=output_text+", "+std::to_string(i)+", "+std::to_string(0);
			}
		}
	}else{

	}

	std::ofstream fout;
	fout.open(filename,std::ios::app);
	fout << output_text <<std::endl;
	fout.close();

	Simulator::Schedule(Time(Seconds(1.0)),&NetSim::OutputClusterMemberSize,this);
}

void NetSim::OutputClusterHopInfo(){
	int64_t time = Simulator::Now().GetInteger();
	int64_t roughTime = time / 1000000000;
	int64_t splitSecond = time % 1000000000;
	char filename[1000];
	std::ostringstream oss_delay,oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}
	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterInfo/ClusterHopInfo/ClusterHopInfo_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::map<uint8_t,int> m1;

	for(uint32_t i=0; i<simple::MyBuilding::GetNumUsers(); ++i){
		Ptr<simple::RoutingProtocol> obj = n_user[i]->GetObject<simple::RoutingProtocol>();
		Ptr<simple::MyUser> user = n_user[i]->GetObject<simple::MyUser>();
		uint8_t hop = obj->Gethop();

		if(user->GetStatus()!=3){
			decltype(m1)::iterator itr = m1.find(hop);
			if(itr!=m1.end()){
					itr->second++;
			}else {
				m1.insert(std::make_pair(hop,1));
			}
		}
	}

	String output_text = std::to_string(roughTime) + "." + std::to_string(splitSecond);
	if(!m1.empty()){
		int hop_max = m1.rbegin()->first;
		for(uint8_t i = 0;i<=hop_max;i++){
			decltype(m1)::iterator itr = m1.find(i);
			if(itr!=m1.end()){
				std::pair<uint8_t,int> it = *itr;
				output_text=output_text+", "+std::to_string(it.first) + ", " +std::to_string(it.second);
			}else{
				output_text=output_text+", "+std::to_string(i)+", "+std::to_string(0);
			}
		}
	}else{

	}

	std::ofstream fout;
	fout.open(filename,std::ios::app);
	fout << output_text <<std::endl;
	fout.close();

	Simulator::Schedule(Time(Seconds(1.0)),&NetSim::OutputClusterHopInfo,this);
}

void NetSim::OutputUserDataInfo(){
	char filename[1000];
	std::ostringstream oss_delay,oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "obayashiIOFiles/Log/obayashi/ON/"  +quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) +"/Recive/All_User_Data_DTN_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::map<uint8_t,int> m1;
	std::ofstream fout;
	fout.open(filename,std::ios::out);

	for(uint32_t i=0;
		i<simple::MyBuilding::GetNumUsers()
		 + simple::MyBuilding::GetAddUsersNum(); ++i){
		Ptr<simple::RoutingProtocol> obj = n_user[i]->GetObject<simple::RoutingProtocol>();
		Ptr<simple::MyUser> user = n_user[i]->GetObject<simple::MyUser>();
		//uint8_t hop = obj->Gethop();
		std::set<String> block = user->GetBlockedNodeIDSet();
		uint32_t usernum = user->GetUserId();
		fout << " userID: "<< usernum <<", ";

		for(auto a : block){
			if(!a.empty()){
				fout << a <<", ";
			}
		}

		fout << std::endl;
	}
	fout.close();
}

void NetSim::WriteBlockedNodeKnowledgeLog()
{
        String mode = "ON";
        if (!simple::RoutingProtocol::GetMode()) {
                mode = "OFF";
        }

        String quickhello = "QuickHello_ON";
        if (!simple::RoutingProtocol::GetUseQuickHelloflag()) {
                quickhello = "QuickHello_OFF";
        }

        String eraseblock = "EraseBlock_ON";
        if (!simple::RoutingProtocol::GetUseEraseInfoflag()) {
                eraseblock = "EraseBlock_OFF";
        }

        std::string path =
                "obayashiIOFiles/Log/obayashi/" + std::string(mode) + "/" +
                std::string(quickhello) + "/" +
                std::string(eraseblock) + "/" +
                std::to_string(simple::MyBuilding::GetNumUsers()) +
                "/Info/BlockedNodeKnowledge_" +
                std::to_string(simple::MyBuilding::GetRun()) + ".txt";

        std::ofstream fout;
        fout.open(path, std::ios::app);

        if (!fout.is_open()) {
                std::cerr
                        << "BlockedNodeKnowledge log file could not be opened: "
                        << path << std::endl;
                return;
        }

        if (fout.tellp() == 0) {
                fout << "time,blocked_node_id,known_user_count,"
                     << "active_user_count,knowledge_rate"
                     << std::endl;
        }

        double now = Simulator::Now().GetSeconds();

        std::set<uint32_t> activeUsers =
                simple::MyBuilding::GetActiveUsers();

        std::list<String> allBlockedNodes =
                simple::MyBuilding::GetBlockedNodes2();

        std::map<String, uint32_t> knownUserCounts;

        // 全通行止めを0人で初期化
        for (const auto& blockedNodeId : allBlockedNodes) {
                if (!blockedNodeId.empty()) {
                        knownUserCounts[blockedNodeId] = 0;
                }
        }

        uint32_t maxUserCount =
                simple::MyBuilding::GetNumUsers() +
                simple::MyBuilding::GetAddUsersNum();

        // ActiveUserだけを確認
        for (uint32_t userId : activeUsers) {
                if (userId == 0) {
                        continue;
                }

                uint32_t userIndex = userId - 1;

                // n_user配列の範囲外を参照しないための確認
                if (userIndex >= maxUserCount) {
                        continue;
                }

                Ptr<simple::MyUser> user = n_user[userIndex];

                if (user == 0) {
                        continue;
                }

                std::set<String> userBlockedNodes =
                        user->GetBlockedNodeIDSet();

                for (const auto& blockedNodeId : userBlockedNodes) {
                        auto it = knownUserCounts.find(blockedNodeId);

                        // blocked_nodes_ALL.txtに存在する通行止めだけを数える
                        if (it != knownUserCounts.end()) {
                                it->second++;
                        }
                }
        }

        uint32_t activeUserCount = activeUsers.size();

        for (const auto& item : knownUserCounts) {
                const String& blockedNodeId = item.first;
                uint32_t knownUserCount = item.second;

                double knowledgeRate = 0.0;

                if (activeUserCount > 0) {
                        knowledgeRate =
                                static_cast<double>(knownUserCount) /
                                static_cast<double>(activeUserCount) *
                                100.0;
                }

                fout << now << ","
                     << blockedNodeId << ","
                     << knownUserCount << ","
                     << activeUserCount << ","
                     << knowledgeRate
                     << std::endl;
        }

        fout.close();

        Simulator::Schedule(
                Seconds(10.0),
                &NetSim::WriteBlockedNodeKnowledgeLog,
                this);
}


NetSim::NetSim (): totalReceived(0), totalSent(0)
{
	//numRandomwayNodes=0;
	//numConstantNodes=121;
	seed = 2;
	distance   = 1;
	field_X    = 100;
	field_Y    = 100;
	packetSize = 128;
	numPackets = 0;
	sourceNode = 0;
	sinkNode   = 0;
	interval   = 0.05;
	tracing    = true;      // turn on routing table traces
	traceNum   = 12;        // trace energy consumption for center node
	phyMode    = std::string("DsssRate11Mbps");
	traceEng   = std::string("remained"); // trace remained energy consumption
	rtproto    = std::string("ers");     // set default routing protocol
	io			= std::string("OFF");
	useexitinfo= std::string("OFF");
}

NetSim::~NetSim ()
{
	for (uint32_t i=0; i<simple::MyBuilding::GetNumUsers(); ++i)
		n_user[i] = 0;
	n_user = 0;

	// for(uint32_t i=0; i<simple::MyBuilding::GetAddUsersNum(); ++i)
	// 	add_user[i] = 0;
	// add_user = 0;
}

//2026-04-14

void NetSim::StartAddUserLoop()
{
	Simulator::Schedule(Seconds(simple::MyBuilding::GetAddUserStartTime()),
	                    &NetSim::ActivateReservedAddUsers, this);
}

void NetSim::ActivateReservedAddUsers()
{
	double now = Simulator::Now().GetSeconds();

	if (now > simple::MyBuilding::GetAddUserEndTime()) {
		return;
	}

	char filename[1000];
	String mode = "ON";
	if(simple::RoutingProtocol::GetMode()){
		mode = "ON";
	}else{
		mode = "OFF";
	}

	String quickhello = "QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello = "QuickHello_ON";
	}else{
		quickhello = "QuickHello_OFF";
	}

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock = "EraseBlock_ON";
	}else{
		eraseblock = "EraseBlock_OFF";
	}

	std::string str =
		"obayashiIOFiles/Log/obayashi/" + std::string(mode) + "/" +
		std::string(quickhello) + "/" + std::string(eraseblock) + "/" +
		std::to_string(simple::MyBuilding::GetNumUsers()) +
		"/Time/AddUserTime_" + std::to_string(simple::MyBuilding::GetRun()) + ".txt";

	str.copy(filename, str.size());
	filename[str.size()] = '\0';

	for (uint32_t k = 0; k < simple::MyBuilding::GetAddUsersPerInterval(); ++k) {
		uint32_t idx = simple::MyBuilding::GetNextAddUserIndex();

		if (idx >= simple::MyBuilding::GetAddUsersNum()) {
			return;
		}

		uint32_t nodeIndex = simple::MyBuilding::GetNumUsers() + idx;
		Ptr<simple::MyUser> addUser = n_user[nodeIndex];

		addUser->SetWaitStatus(false);
		addUser->SetMyUser();

		simple::MyBuilding::AddActiveUser(nodeIndex + 1);

		Ptr<simple::RoutingProtocol> obj =
			addUser->GetObject<simple::RoutingProtocol>();

		if (obj != 0) {
			obj->HandlClusterViewTimer();
			obj->ActivateHelloTimer(0);
		}

		addUser->AddUserMovementTimer(Seconds(0));

		if (now >= simple::MyBuilding::GetMeasureStartTime() &&
		    now <= simple::MyBuilding::GetMeasureEndTime()) {
			std::ofstream fout;
			fout.open(filename, std::ios::app);
			fout << now << "," << (nodeIndex + 1) << "," << idx << std::endl;
			fout.close();
		}

		// ��ʂł��m�F�������Ȃ炱����c��
		std::cout << "[AddUser] time=" << now
		          << " userID=" << (nodeIndex + 1)
		          << " idx=" << idx
		          << " interval=" << simple::MyBuilding::GetAddUserInterval()
		          << std::endl;

		simple::MyBuilding::PlusNextAddUserIndex();
	}

	Simulator::Schedule(Seconds(simple::MyBuilding::GetAddUserInterval()),
	                    &NetSim::ActivateReservedAddUsers, this);
}

void NetSim::ConfigureSetDefault ()
{
	Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
	Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));
	Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue(100.0));
	//Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));
}

//////////////////////////////////random number generateer////////////////////////////////////////
std::mt19937 NetSim::create_rand_engine(){
	std::random_device rnd;
	std::vector<std::uint_least32_t> v(10);
	std::generate(v.begin(), v.end(), std::ref(rnd));
	std::seed_seq seed(v.begin(), v.end());
	return std::mt19937(seed);
}

std::vector<int> NetSim::make_rand_array_select(const size_t size, int rand_min, int rand_max){
    if(rand_min > rand_max) std::swap(rand_min, rand_max);
    const size_t max_min_diff = static_cast<size_t>(rand_max - rand_min + 1);
    if(max_min_diff < size) throw std::runtime_error("The argument is wrong");

    std::vector<int> tmp;
    tmp.reserve(max_min_diff);

    for(int i = rand_min; i <= rand_max; ++i)tmp.push_back(i);

    auto engine = create_rand_engine();
    std::uniform_int_distribution<int> distribution(rand_min, rand_max);

    for(size_t cnt = 0; cnt < size; ++cnt){
        size_t pos =std::uniform_int_distribution<size_t>(cnt, tmp.size()-1)(engine);

        if(cnt != pos) std::swap(tmp[cnt], tmp[pos]);
    }
    tmp.erase(std::next(tmp.begin(), size), tmp.end());

    return tmp;
}

int NetSim::get_rand_range(uint32_t seed, int min_val, int max_val) {
    std::mt19937 mt64(seed);

    std::uniform_int_distribution<int> get_rand_uni_int( min_val, max_val );

    return get_rand_uni_int(mt64);
}

std::vector<int> NetSim::get_rand_pseudorandom(const size_t size, int min_val, int max_val, uint32_t seed){
	std::vector<int> vec;
	std::set<int> se;
	int num;
	size_t i=0;

    std::mt19937 mt64(seed);
    std::uniform_int_distribution<int> get_rand_uni_int( min_val, max_val );

	for(i=0;i<size;i++){
		num = get_rand_uni_int(mt64);
			decltype(se)::iterator it = se.find(num);
			if(it!=se.end()){
				i--;
			}else{
				vec.push_back(num);
				se.insert(num);
			}
	}

//	while(1){
//		num = get_rand_range(seed, min_val, max_val);
//		decltype(se)::iterator it = se.find(num);
//		if(it!=se.end()){
//
//		}else{
//			vec.push_back(num);
//			se.insert(num);
//			i++;
//		}
//		if(i==size) break;
//	}
	return vec;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void NetSim::MakeNetworkTopologyforJSON()
{
	uint32_t numferry;

	if(simple::MyBuilding::GetAutoFerryNumFlag()){
		numferry = simple::MyBuilding::GetExitNodes().size();
	}else{
		numferry = simple::MyBuilding::GetNumFerrys();
	}
	//set user nodes
	uint32_t APnum = simple::MyBuilding::GetExitNodes().size();
	
	n_user = new Ptr<simple::MyUser> [simple::MyBuilding::GetNumUsers()+simple::MyBuilding::GetAddUsersNum()+APnum];
	n_ferry = new Ptr<simple::MyUser> [numferry];

	for (uint32_t i=0; i<(simple::MyBuilding::GetNumUsers()+simple::MyBuilding::GetAddUsersNum()+APnum); ++i)
	{
		n_user[i] = CreateObject<simple::MyUser> ();
		userNodes.Add(n_user[i]);
	}

	for (uint32_t i = 0; i < simple::MyBuilding::GetNumUsers(); ++i) {
		simple::MyBuilding::AddActiveUser(i + 1);
	}

	for (uint32_t i=0; i<numferry; ++i)
	{
		n_ferry[i] = CreateObject<simple::MyUser> ();
		ferryNodes.Add(n_ferry[i]);
		// n_ferry[i]->SetPatrolRoute(simple::MyBuilding::GetPatrolRoute());
	}

	std::map<std::pair<double,double>,simple::way_node> graph = simple::JSON_Data::get_graph();


	std::vector<int> rand =get_rand_pseudorandom(simple::MyBuilding::GetNumUsers(),0,(int)simple::JSON_Data::get_waynodes().size()-1,seed);
	std::sort(rand.begin(),rand.end());

	std::vector<int> rand2 =get_rand_pseudorandom(simple::MyBuilding::GetAddUsersNum(),0,(int)simple::JSON_Data::get_waynodes().size()-1,seed+1);
	std::sort(rand2.begin(),rand2.end());

	std::vector<int> rand3 =get_rand_pseudorandom(APnum,0,(int)simple::JSON_Data::get_waynodes().size()-1,seed+2);
	std::sort(rand3.begin(),rand3.end());
	// for(int x : rand){
	// 	std::cout<<"rand:"<<x<<std::endl;
	// }

	std::map<String,simple::way_info> way = simple::JSON_Data::get_linestrings();
	std::map<std::pair<double,double>,simple::way_node> node = simple::JSON_Data::get_waynodes();

	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

	int i=0;
	int j=0;

	std::map<String, uint32_t> hinan = simple::MyBuilding::GetExitNodes();
	for (auto itr = hinan.begin(); itr != hinan.end(); ++itr)
	{
		std::map<std::string, double> pos;
		std::pair<double, double> zahyou = simple::JSON_Data::GetCoordinatefromID(itr->first);
		simple::way_node wn = node.at(zahyou);
		wn.SetExitflag(true);
	}

	for(auto itr = node.begin();itr != node.end();++itr){
		if(i==rand.front()){
			std::pair<double,double> coord = itr->first; //自分の初期位置？
			simple::way_node n_info = itr->second;
			String nodeid = n_info.GetNodeID();
			std::cout<<nodeid<<std::endl;
			simple::way_info w_info = way.at(n_info.GetWayIDs().front());
			std::list<simple::way_node> w_nodes = w_info.GetNodes();
			std::list<simple::way_node> cw_nodes;
			std::list<simple::way_node> cw_nodes2;
			uint32_t f_distance_to_crossing = 0;
			uint32_t b_distance_to_crossing = 0;

			positionAlloc->Add (Vector (simple::JSON_Data::json_x(coord.first), simple::JSON_Data::json_y(coord.second), 0.0));
			bool f_flag = false;
			bool b_flag = false;
			for(auto itr2 = w_nodes.begin();itr2 != w_nodes.end();++itr2){
				simple::way_node n = *itr2;
				std::pair<double,double> c = n.GetCoordinate();

				if(c==coord){
					// auto itr3 = itr2; //自分の位置のイテレータを保持
					for(auto itr3 = itr2; itr3 != w_nodes.end(); ++itr3){
						simple::way_node n1 = *itr3;
						if(n1.GetCrossingflag() || n1.GetExitflag()){
							// if(n_user[j]->GetUserId() + 1 == 13) std::cout<<"実行されました1"<<std::endl; //debag
							cw_nodes.push_back(n1);
							f_flag = true;
							break;
						}
						cw_nodes.push_back(n1);
						f_distance_to_crossing++;
					}
					for(auto itr3 = itr2; itr3 != std::prev(w_nodes.begin()); --itr3){
						simple::way_node n2 = *itr3;
						if(n2.GetCrossingflag() || n2.GetExitflag()){
							// if(n_user[j]->GetUserId() + 1 == 13) std::cout<<"実行されました2"<<std::endl; //debag
							cw_nodes2.push_back(n2);
							b_flag = true;
							break;
						}
						cw_nodes2.push_back(n2);
						b_distance_to_crossing++;					
					}
					break;
				}
			}

			
			if(f_flag == true && b_flag == true){
				if(f_distance_to_crossing <= b_distance_to_crossing){
					w_info.SetNodeList(cw_nodes);
				}else{
					w_info.SetNodeList(cw_nodes2);
				}
			}else if(f_flag == true && b_flag == false){
				w_info.SetNodeList(cw_nodes);
			}else if(f_flag == false && b_flag == true){
				w_info.SetNodeList(cw_nodes2);
			}else{
				w_info.SetNodeList(cw_nodes2);
				std::cout<<f_flag<<", "<<b_flag<<std::endl;
				std::cout<<n_user[j]->GetUserId() + 1<<"：ここが実行されるということはフラフラしてます"<<std::endl;
			}
			n_user[j]->SetCurrentWay(w_info);
			n_user[j]->SetGraph(graph);
			rand.erase(rand.begin());
			j++;
		}
		i++;
	}

	// std::cout<<"j = "<<j<<std::endl;
	int x = 0;
	for(auto itr = node.begin();itr != node.end();++itr){
		if(x==rand2.front()){
			std::pair<double,double> coord = itr->first;
			simple::way_node n_info = itr->second;
			simple::way_info w_info = way.at(n_info.GetWayIDs().front());
			std::list<simple::way_node> w_nodes = w_info.GetNodes();
			std::list<simple::way_node> cw_nodes;
			std::list<simple::way_node> cw_nodes2;
			uint32_t f_distance_to_crossing = 0;
			uint32_t b_distance_to_crossing = 0;
			// std::list<simple::way_node> cw_nodes3;

//			std::cout<<"("<<coord.first<<","<<coord.second<<")"<<std::endl;

			positionAlloc->Add (Vector (simple::JSON_Data::json_x(coord.first), simple::JSON_Data::json_y(coord.second), 0.0));
			bool f_flag = false;
			bool b_flag = false;
			for(auto itr2 = w_nodes.begin();itr2 != (w_nodes.end());++itr2){
				simple::way_node n = *itr2;
				std::pair<double,double> c = n.GetCoordinate();
				if(c==coord){
					// auto itr3 = itr2; //自分の位置のイテレータを保持
					for(auto itr3 = itr2; itr3 != w_nodes.end(); ++itr3){
						simple::way_node n1 = *itr3;
						if(n1.GetCrossingflag() || n1.GetExitflag()){
							cw_nodes.push_back(n1);
							f_flag = true;
							break;
						}
						cw_nodes.push_back(n1);
						f_distance_to_crossing++;
					}
					for(auto itr3 = itr2; itr3 !=std::prev(w_nodes.begin()); itr3--){
						simple::way_node n2 = *itr3;
						if(n2.GetCrossingflag() || n2.GetExitflag()){
							cw_nodes2.push_back(n2);
							b_flag = true;
							break;
						}
						cw_nodes2.push_back(n2);
						b_distance_to_crossing++;					
					}
					break;
				}
			}

			// if(n_user[j]->GetUserId() + 1 == 3){
			// 	int i = 1;
			// 	for(auto a : cw_nodes){
			// 		std::pair<double,double> coordinate = std::make_pair(a.GetCoordinate().first, a.GetCoordinate().second);
			// 		String nodeid = simple::JSON_Data::GetIDfromCoordinate(coordinate);
			// 		std::cout<<i<<"："<<nodeid<<std::endl;
			// 		i++;
			// 	}
			// 	i = 1;
			// 	for(auto a : cw_nodes2){
			// 		std::pair<double,double> coordinate = std::make_pair(a.GetCoordinate().first, a.GetCoordinate().second);
			// 		String nodeid = simple::JSON_Data::GetIDfromCoordinate(coordinate);
			// 		std::cout<<i<<"："<<nodeid<<std::endl;
			// 		i++;
			// 	}
			// }
			
			if(f_flag == true && b_flag == true){
				if(f_distance_to_crossing <= b_distance_to_crossing){
					w_info.SetNodeList(cw_nodes);
				}else{
					w_info.SetNodeList(cw_nodes2);
				}
			}else if(f_flag == true && b_flag == false){
				w_info.SetNodeList(cw_nodes);
			}else if(f_flag == false && b_flag == true){
				w_info.SetNodeList(cw_nodes2);
			}else{
				w_info.SetNodeList(cw_nodes2);
				std::cout<<f_flag<<", "<<b_flag<<std::endl;
				std::cout<<n_user[j]->GetUserId() + 1<<"：ここが実行されることある？"<<std::endl;
			}

			n_user[j]->SetCurrentWay(w_info);
			n_user[j]->SetGraph(graph);
			rand2.erase(rand2.begin());
			j++;
		}
		x++;
	}

	int y = 0;
	for(auto itr = node.begin();itr != node.end();++itr){
		if(y==rand3.front()){
			std::pair<double,double> coord = itr->first;
			simple::way_node n_info = itr->second;
			simple::way_info w_info = way.at(n_info.GetWayIDs().front());
			std::list<simple::way_node> w_nodes = w_info.GetNodes();
			std::list<simple::way_node> cw_nodes;

//			std::cout<<"("<<coord.first<<","<<coord.second<<")"<<std::endl;

			positionAlloc->Add (Vector (simple::JSON_Data::json_x(coord.first), simple::JSON_Data::json_y(coord.second), 0.0));

			bool p_flag = false;
			for(auto itr2 = w_nodes.begin();itr2 != w_nodes.end();++itr2){
				simple::way_node n = *itr2;
				std::pair<double,double> c = n.GetCoordinate();
				if(c==coord){
					p_flag = true;
				}
				if(p_flag){
					cw_nodes.push_back(n);
				}
			}
			w_info.SetNodeList(cw_nodes);
			// std::cout<<"j = "<<j<<std::endl;
			n_user[j]->SetCurrentWay(w_info);
			n_user[j]->SetGraph(graph);
			rand3.erase(rand3.begin());
			j++;
		}
		y++;
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   mobility.SetPositionAllocator (positionAlloc);

	mobility.SetMobilityModel ( // ノードの移動モデル
			"ns3::WaypointMobilityModel",
			"InitialPositionIsWaypoint", BooleanValue (false)
			);
	mobility.Install (userNodes);
	// std::cout<<"test"<<std::endl;

	// mobility.Install (adduserNodes);
	// std::cout<<"test"<<std::endl;
	u_int32_t ii = 0;
	for (; ii<simple::MyBuilding::GetNumUsers()+simple::MyBuilding::GetAddUsersNum(); ++ii)
	{
		if(useexitinfo=="OFF"){
			n_user[ii]->SetUseexitinfoflag(false);
		}
		if(useexitinfo=="ON"){
			n_user[ii]->SetUseexitinfoflag(true);
		}
		std::map<std::string, double> pos;
		std::pair<double,double> no=n_user[ii]->GetCurrentWay().GetNodes().front().GetCoordinate(); 
		double x;
		double y;
		x=simple::JSON_Data::json_x(no.first);
		y=simple::JSON_Data::json_y(no.second);

		// std::cout<<"x:"<<x<<"y:"<<y<<std::endl;

		pos.insert (std::make_pair("x", x));
		pos.insert (std::make_pair("y", y));
		pos.insert (std::make_pair("z", 0.0));
		Ptr<WaypointMobilityModel> mob = userNodes.Get(ii)->GetObject<WaypointMobilityModel> ();
		Waypoint wpt (Seconds(0.1), Vector(pos.at("x"), pos.at("y"), pos.at("z")));
		mob->AddWaypoint (wpt);
		pos.clear ();
		n_user[ii]->SetMyUser();
	}

	// std::map<String, uint32_t> hinan = simple::MyBuilding::GetExitNodes();
	for (auto itr = hinan.begin(); itr != hinan.end(); ++itr)
	{
		std::map<std::string, double> pos;
		std::pair<double, double> zahyou = simple::JSON_Data::GetCoordinatefromID(itr->first);
		double x;
		double y;
		x = simple::JSON_Data::json_x(zahyou.first);
		y = simple::JSON_Data::json_y(zahyou.second);

		pos.insert (std::make_pair("x", x));
		pos.insert (std::make_pair("y", y));
		pos.insert (std::make_pair("z", 0.0));
		Ptr<WaypointMobilityModel> mob = userNodes.Get(ii)->GetObject<WaypointMobilityModel> ();
		Waypoint wpt (Seconds(0.1), Vector(pos.at("x"), pos.at("y"), pos.at("z")));
		mob->AddWaypoint (wpt);
		pos.clear ();
		n_user[ii]->SetCurrentCoordinate(zahyou);
		n_user[ii]->SetMyUser();
		// simple::MyBuilding::SetExitList(std::make_pair(itr->first,n_user[ii]));
		ii++;
	}


	mobility.SetMobilityModel ( // ノードの移動モデル
			"ns3::WaypointMobilityModel",
			"InitialPositionIsWaypoint", BooleanValue (false)
			);
	mobility.Install (ferryNodes);

	for (uint32_t i=0; i<numferry; i++)
	{
		std::vector<std::string> patorol_route = simple::MyBuilding::GetPatrolRoute();
		n_ferry[i]->SetPatrolRoute(patorol_route);
		n_ferry[i]->SetOldPatrolRoute(patorol_route);
		std::cout<<"フェリーノード生成"<<std::endl;
		std::map<std::string, double> pos;
		// GenerateRandomNodePosition(pos);
		Ptr<WaypointMobilityModel> mob = ferryNodes.Get(i)->GetObject<WaypointMobilityModel> ();
		String start_node;
		if(simple::MyBuilding::GetAutoFerryNumFlag()){
			start_node = patorol_route[i];
			n_ferry[i]->SetCurrentPatrolIndex(i);
		}else{
			start_node= simple::MyBuilding::GetFerryStart();
		}
		std::pair<double,double> xy = simple::JSON_Data::GetCoordinatefromID(start_node);
		std::cout<<start_node<<std::endl;
		double x = simple::JSON_Data::json_x(xy.first);
		double y = simple::JSON_Data::json_y(xy.second);
		pos.insert (std::make_pair("x", x));
		pos.insert (std::make_pair("y", y));
		pos.insert (std::make_pair("z", 20.0));
		Waypoint wpt (Seconds(0.1), Vector(pos.at("x"), pos.at("y"), pos.at("z")));
		mob->AddWaypoint (wpt);
		pos.clear ();
		// std::vector<std::string> test = simple::MyBuilding::GetPatrolRoute();
		// std::cout << "Patrol Route: ";
		// for(const auto& s : test) std::cout << s << " ";
		// std::cout << std::endl;
	}	

}

void NetSim::MakeNetworkTopology()
{
	//set user nodes
	n_user = new Ptr<simple::MyUser> [simple::MyBuilding::GetNumUsers()];
	for (uint32_t i=0; i<simple::MyBuilding::GetNumUsers(); ++i)
	{
		n_user[i] = CreateObject<simple::MyUser> ();
		userNodes.Add(n_user[i]);
	}

	mobility.SetPositionAllocator ( // ノードの配置
			"ns3::RandomRectanglePositionAllocator",
			"X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"),
			"Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=100.0]")
			);
	mobility.SetMobilityModel ( // ノードの移動モデル
			"ns3::WaypointMobilityModel",
			"InitialPositionIsWaypoint", BooleanValue (false)
			);
	mobility.Install (userNodes);

	for (uint32_t i=0; i<simple::MyBuilding::GetNumUsers(); i++)
	{
		std::map<std::string, double> pos;
		GenerateRandomNodePosition(pos);
		Ptr<WaypointMobilityModel> mob = userNodes.Get(i)->GetObject<WaypointMobilityModel> ();
		Waypoint wpt (Seconds(0.1), Vector(pos.at("x"), pos.at("y"), pos.at("z")));
		mob->AddWaypoint (wpt);
		pos.clear ();
		n_user[i]->SetMyUser();
	}


	//set access point nodes
//	n_accessPoint = new Ptr<simple::MyAccessPoint>[simple::MyBuilding::GetNumSections()];
//	for (uint32_t i=0; i<simple::MyBuilding::GetNumSections(); ++i) {
//		n_accessPoint[i] = CreateObject<simple::MyAccessPoint>();
//		accessPointNodes.Add(n_accessPoint[i]);
//	}
//
//	mobility.SetPositionAllocator (
//			"ns3::GridPositionAllocator",	// 正方形Grid
//			"MinX", DoubleValue (simple::MyBuilding::GetSectionWidth()/2),
//			"MinY", DoubleValue (simple::MyBuilding::GetSectionHight()/2),
//			"DeltaX", DoubleValue (simple::MyBuilding::GetSectionWidth()),
//			"DeltaY", DoubleValue (simple::MyBuilding::GetSectionHight()),
//			"GridWidth", UintegerValue (simple::MyBuilding::GetNumSectionX()),
//			"LayoutType", StringValue ("RowFirst"));
//	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//	mobility.Install (accessPointNodes);
//
//	for (uint32_t i=0; i<simple::MyBuilding::GetNumSections(); i++)
//	{
//		n_accessPoint[i]->SetMyAccessPoint();
//	}
}

void
NetSim::ConfigureDataLinkLayer()
{

//
	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel");

	wifiPhy =  YansWifiPhyHelper();
	// set it to zero; otherwise, gain will be added
	wifiPhy.Set ("RxGain", DoubleValue (-10) );
	// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
	wifiPhy.SetChannel (wifiChannel.Create ());


	// Add a non-QoS upper mac, and disable rate control
	WifiMacHelper wifiMac;
	wifiMac.SetType ("ns3::AdhocWifiMac");

	WifiHelper wifi;
	wifi.SetStandard (WIFI_STANDARD_80211g);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
			"DataMode",StringValue (phyMode),
			"ControlMode",StringValue (phyMode));

	// Set it to adhoc mode
	userDevices = wifi.Install (wifiPhy, wifiMac, userNodes);
	ferryDevices = wifi.Install (wifiPhy, wifiMac, ferryNodes);
	accessPointDevices = wifi.Install (wifiPhy, wifiMac, accessPointNodes);
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
	basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (initialEnergy+1000000));
	EnergySourceContainer e_user = basicSourceHelper.Install(userNodes);
	EnergySourceContainer e_ferry = basicSourceHelper.Install(ferryNodes);
	EnergySourceContainer e_accessPoint = basicSourceHelper.Install(accessPointNodes);

	// transmit at 0dBm = 17.4mA, receive mode = 19.7mA
	WifiRadioEnergyModelHelper radioEnergyHelper;
	radioEnergyHelper.Set ("TxCurrentA", DoubleValue (txcurrent));
	radioEnergyHelper.Set ("RxCurrentA", DoubleValue (rxcurrent));
	radioEnergyHelper.SetDepletionCallback(
			MakeCallback(&NetSim::NotifyDrained, this));
	DeviceEnergyModelContainer deviceModelsC = radioEnergyHelper.Install (userDevices, e_user);
	DeviceEnergyModelContainer deviceModelsF = radioEnergyHelper.Install (ferryDevices, e_ferry);
	DeviceEnergyModelContainer deviceModelsA = radioEnergyHelper.Install (accessPointDevices, e_accessPoint);

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
	// mode = 2;
	// if(mode == SIMPLE){
	// 	InternetStackHelper internet;
	// 	internet.SetRoutingHelper(simple);
	// 	internet.Install(userNodes);
	// 	internet.Install(accessPointNodes);

	// 	Ipv4AddressHelper ipv4;
	// 	ipv4.SetBase ("10.0.0.0", "255.0.0.0");
	// 	ipv4.Assign (userDevices);
	// 	ipv4.Assign (accessPointDevices);
	// }else if(mode == DTN){
	// 	InternetStackHelper internet;
	// 	internet.SetRoutingHelper(dtn);
	// 	internet.Install(userNodes);
	// 	internet.Install(accessPointNodes);

	// 	Ipv4AddressHelper ipv4;
	// 	ipv4.SetBase ("10.0.0.0", "255.0.0.0");
	// 	ipv4.Assign (userDevices);
	// 	ipv4.Assign (accessPointDevices);
	// }

	// if(num == simple::MyBuilding::GetNumUsers()){

	// }

		InternetStackHelper internet;
		internet.SetRoutingHelper(dtn);
		internet.Install(userNodes);
		internet.Install(accessPointNodes);
		internet.Install(ferryNodes);

		Ipv4AddressHelper ipv4;
		ipv4.SetBase ("10.0.0.0", "255.0.0.0");
		ipv4.Assign (userDevices);
		ipv4.Assign(ferryDevices);
		ipv4.Assign (accessPointDevices);

		// Ptr<OutputStreamWrapper> test = Create<OutputStreamWrapper>("rought/test.rought",std::ios::out);
		// dtn.PrintRoutingTableAllAt(Seconds(300),test);

		// wifiPhy.EnablePcap("obayashiIOFiles/Log/obayashi/pcap-user", userDevices);

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

	/*
	Ptr<Socket> destination = Socket::CreateSocket (n[dst], tid);
	InetSocketAddress dstSocketAddr = setSocketAddress (n[dst], 8888);
	destination->Bind (dstSocketAddr);
	destination->SetRecvCallba
	std::vector<simple::MyUser> my_user(numUsers);
	std::vector<simple::MyNode> my_node(numConstantNodes);ck (MakeCallback (&NetSim::ReceivePacket, this));

	// set sender(n0)'s socket
	Ptr<Socket> source = Socket::CreateSocket (n[src], tid);
	InetSocketAddress srcSocketAddr = setSocketAddress (n[src], 8888);
	source->Bind (srcSocketAddr);
	source->SetAllowBroadcast (true);
	source->Connect (dstSocketAddr);
	*/

//	Ptr<Socket> destination = Socket::CreateSocket (Nodes.Get(dst), tid);
//	InetSocketAddress dstSocketAddr = setSocketAddress (Nodes.Get(dst), 8888);
//	destination->Bind (dstSocketAddr);
//	destination->SetRecvCallback (MakeCallback (&NetSim::ReceivePacket, this));
//
//	// set sender(n0)'s socket
//	Ptr<Socket> source = Socket::CreateSocket (Nodes.Get(src), tid);
//	InetSocketAddress srcSocketAddr = setSocketAddress (Nodes.Get(src), 8888);
//	source->Bind (srcSocketAddr);
//	source->SetAllowBroadcast (true);
//	source->Connect (dstSocketAddr);

	//データパケット生成間隔設定
	Time interPacketInterval = Seconds (interval);
	// Give Proactive algorithms some time to converge -- 20 seconds perhaps

	//データパケット生成時間設定
//	Simulator::Schedule (Seconds (10.0), &NetSim::GenerateTraffic, this, source, interPacketInterval);
}

void NetSim::SetFlowMonitor ()
{
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();
	uint32_t ALL_Tx_Packets = 0;
	uint32_t ALL_Rx_Packets = 0;
	uint32_t ALL_Tx_Bytes = 0;
	uint32_t ALL_Rx_Bytes = 0;
	uint32_t ALL_Loss_Packets = 0;
	Simulator::Stop (Seconds (simtime));
	Simulator::Schedule(Seconds(0.0), &simple::MyBuilding::WriteActiveUserLog);
	Simulator::Schedule(Seconds(0.0), &NetSim::WriteBlockedNodeKnowledgeLog, this);
	Simulator::Run ();

	monitor->CheckForLostPackets();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
	FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

	String mode="ON";
	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
		return;
	}

	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag() == true){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	// std::string str = "obayashiIOFiles/Log/Block_Nodes.txt";
	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Info/Flow_moniter/flowMoniter_" +std::to_string(simple::MyBuilding::GetRun()) +".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::app);

	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
	{
		// Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
		    // if (t.sourceAddress == "10.0.0.2")
		    //   {
		    //     continue;
		    //   }
		uint32_t Tx_Packets = i->second.txPackets;
		uint32_t Tx_Bytes = i->second.txBytes;
		uint32_t Rx_Packets = i->second.rxPackets;
		uint32_t Rx_Bytes = i->second.rxBytes;
		uint32_t LostPackets = i->second.lostPackets;
		// if(LostPackets != 0){
		// 	fout << "	Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
		// 	fout << "  Tx Packets: " << Tx_Packets << "\n"; //送信パケット
		// 	fout << "  Tx Bytes:   " << Tx_Bytes << "\n"; //送信バイト
		// 	fout << "  Rx Packets: " << Rx_Packets << "\n"; //受信パケット
		// 	fout << "  Rx Bytes:   " << Rx_Bytes << "\n"; //受信バイト
		// 	fout << "LostPackets: " << LostPackets<< "\n";

		// 	std::cout << "	Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
		// 	std::cout << "  Tx Packets: " << Tx_Packets << "\n"; //送信パケット
		// 	std::cout << "  Tx Bytes:   " << Tx_Bytes << "\n"; //送信バイト
		// 	std::cout << "  Rx Packets: " << Rx_Packets << "\n"; //受信パケット
		// 	std::cout << "  Rx Bytes:   " << Rx_Bytes << "\n"; //受信バイト
		// 	std::cout << "LostPackets: " << LostPackets<< "\n";
		// 	std::cout << "フローモニターの値がおかしい場合は値を初期化してないからです。"<<std::endl;
		// }
		// std::cout << "	Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
		// std::cout << "  Tx Packets: " << Tx_Packets << "\n";
		// std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
		// std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
		// std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
		// std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
		// std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
		// std::cout << "LostPackets: " << LostPackets<< "\n";
		// std::cout << "  PacketDropped:  ";
		// for(const auto& packet : i->second.packetsDropped){
		// 	std::cout << packet << ", ";
		// }
		// std::cout << std::endl;
		ALL_Tx_Packets = ALL_Tx_Packets + Tx_Packets;
		ALL_Tx_Bytes = ALL_Tx_Bytes + Tx_Bytes;
		ALL_Rx_Packets = ALL_Rx_Packets + Rx_Packets;
		ALL_Rx_Bytes = ALL_Rx_Bytes + Rx_Bytes;
		ALL_Loss_Packets = ALL_Loss_Packets + LostPackets;

	}
	// int num_frequency = simple::RoutingProtocol::Get_Frequency();
	fout << "ALL_Tx_Packets: " << ALL_Tx_Packets << "\n";
	fout << "ALL_Tx_Bytes: " << ALL_Tx_Bytes << "\n";
	fout << "ALL_Rx_Packets: " << ALL_Rx_Packets << "\n";
	fout << "ALL_Rx_Bytes: " << ALL_Rx_Bytes << "\n";
	fout << "ALL_Loss_Packets: " << ALL_Loss_Packets << "\n";
	fout << "ALL_Send_Hello_Pakets: "<< simple::MyBuilding::GetSendHelloNum() <<"\n";
	fout << "ALL_Recv_Hello_Pakets: "<< simple::MyBuilding::GetRecvHelloNum() <<"\n";
	fout << "ALL_Send_Hello_Bytes: "<<simple::MyBuilding::GetSendHelloBytes() <<"\n";
	fout << "ALL_Recv_Hello_Bytes: " <<simple::MyBuilding::GetRecvHelloBytes() <<"\n";
	fout << "ALL_Send_Ack_Hello_Pakets: "<< simple::MyBuilding::GetSendAckHelloNum() <<"\n";
	fout << "ALL_Recv_Ack_Hello_Pakets: "<< simple::MyBuilding::GetRecvAckHelloNum() <<"\n";

	fout << "ALL_Send_Request_Pakets: " << simple::MyBuilding::GetSendRequestNum() << "\n";
	fout << "ALL_Recv_Request_Pakets: " << simple::MyBuilding::GetRecvRequestNum() << "\n";
	fout << "ALL_Send_UserData_Pakets: " << simple::MyBuilding::GetSendUserDataNum() << "\n";
	fout << "ALL_Recv_UserData_Pakets: " << simple::MyBuilding::GetRecvUserDataNum() << "\n";

	fout << "NearShelter_Recv_Hello: "
		<< simple::MyBuilding::GetNearShelterRecvHelloNum() << "\n";

	fout << "NearShelter_Recv_Request: "
		<< simple::MyBuilding::GetNearShelterRecvRequestNum() << "\n";

	fout << "NearShelter_Recv_UserData: "
		<< simple::MyBuilding::GetNearShelterRecvUserDataNum() << "\n";
		

	std::cout << "ALL_Tx_Packets: " << ALL_Tx_Packets << "\n";
	std::cout << "ALL_Tx_Bytes: " << ALL_Tx_Bytes << "\n";
	std::cout << "ALL_Rx_Packets: " << ALL_Rx_Packets << "\n";
	std::cout << "ALL_Rx_Bytes: " << ALL_Rx_Bytes << "\n";
	std::cout << "ALL_Loss_Packets: " << ALL_Loss_Packets << "\n";
	std::cout << "ALL_Tx_Packets - ALL_Loss_Packets: " << ALL_Tx_Packets - ALL_Loss_Packets << "\n";

	std::cout << "ALL_Send_Request_Pakets: " << simple::MyBuilding::GetSendRequestNum() << "\n";
	std::cout << "ALL_Recv_Request_Pakets: " << simple::MyBuilding::GetRecvRequestNum() << "\n";
	std::cout << "ALL_Send_UserData_Pakets: " << simple::MyBuilding::GetSendUserDataNum() << "\n";
	std::cout << "ALL_Recv_UserData_Pakets: " << simple::MyBuilding::GetRecvUserDataNum() << "\n";

	std::cout << "NearShelter_Recv_Hello: "
			<< simple::MyBuilding::GetNearShelterRecvHelloNum() << "\n";

	std::cout << "NearShelter_Recv_Request: "
			<< simple::MyBuilding::GetNearShelterRecvRequestNum() << "\n";

	std::cout << "NearShelter_Recv_UserData: "
			<< simple::MyBuilding::GetNearShelterRecvUserDataNum() << "\n";

	// std::cout << "通信回数: " << num_frequency << "\n";
	fout.close();
	// std::cout<<simple::RoutingProtocol::GetUseQuickHelloflag()<<std::endl;
	OutputUserDataInfo();
}

void NetSim::RunSimulation ()
{
	//NS_LOG_FUNCTION(this);
	// RngSeedManager::SetRun(1);
	// RngSeedManager::SetSeed(seed);
	// simple::MyBuilding::SetExitNodes();//10/18
	ConfigureSetDefault();
//	MakeNetworkTopology();
	simple::MyBuilding::SetExitNodeDistance(); //避難所間の距離を計算
	MakeNetworkTopologyforJSON();
	Initialize_Logfile_for_Exit_Nodes();
	ConfigureDataLinkLayer();
	AttachEnergyModelToDevices(300, 0.0857, 0.0528);
	ConfigureNetworkLayer ();

	//2026-04-14
	Initialize_Logfile_for_AddUserTime();
	StartAddUserLoop();

//	std::cout<<"test"<<std::endl;
	if(simple::MyBuilding::GetNumUsers()>0) ConfigureL4withDUPApp(userNodes,sourceNode, sinkNode);
	if(simple::MyBuilding::GetNumFerrys()>0) ConfigureL4withDUPApp(ferryNodes,sourceNode, sinkNode);
	if(simple::MyBuilding::GetNumSections()>0) ConfigureL4withDUPApp(accessPointNodes,sourceNode, sinkNode);

	// OutputClusterMemberSize();
	// OutputClusterHopInfo();


	///////
//	FlowMonitorHelper flowmon;
//	Ptr<FlowMonitor> monitor = flowmon.InstallAll();
//
//	Simulator::Stop (Seconds (simtime));
//	Simulator::Run ();

	 SetFlowMonitor();

	Simulator::Destroy ();
}

void
NetSim::Configure (int argc, char *argv[])
{
	CommandLine cmd;
	//cmd.AddValue ("p_mode"   , "Mode of Protocol", mode);
	cmd.AddValue ("phyMode"   , "Wifi Physical layer mode", phyMode);
	cmd.AddValue ("distance"  , "distance (m)", distance);
	cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
	cmd.AddValue ("numPackets", "total number of packets generated", numPackets);
	cmd.AddValue ("interval"  , "interval (seconds) between send packets", interval);
	//cmd.AddValue ("num_node"  , "total number of nodes on the Grid", (uint32_t)sim_environment.GetNumNode());
	cmd.AddValue ("sourceNode", "Sender's node number", sourceNode);
	cmd.AddValue ("sinkNode"  , "Receiver's node number", sinkNode);
	cmd.AddValue ("traceEng"  , "trace energy flag (remained|total)", traceEng);
	cmd.AddValue ("traceNode" , "trace energy for node", traceNum);
	cmd.AddValue ("rtProto"   , "set routing protocol(Olsr,Aodv,Dsdv)", rtproto);
	cmd.AddValue ("traceFile" , "Ns2 movement trace file", traceFile);
	cmd.AddValue ("run"       , "run", run);
	cmd.AddValue ("seed"      , "seed", seed);
	cmd.AddValue ("simtime"   , "simtime", simtime);
	cmd.AddValue ("mode"   , "Mode ON/OFF", io);
	cmd.AddValue ("usequickhello","UseQuickHello ON/OFF", usequickhello );
	cmd.AddValue ("useexitinfo", "UseExitInfo ON/OFF", useexitinfo);
	cmd.AddValue ("useeraseinfo", "AoIに基づいて情報を消すか消さないか",useeraseinfo );
	cmd.AddValue ("userroutetrace", "user's route trace (specified as a string separated by ', ')", traceuserid);
	cmd.AddValue ("A_d", "しきい値", A_d);
	cmd.AddValue ("usehop", "Hopを用いて避難所の予想をする", usehop);
	cmd.Parse (argc, argv);

	simple::RoutingProtocol::SetMode(io);
	simple::RoutingProtocol::SetUseQuickHelloflag(usequickhello);
	simple::RoutingProtocol::SetUseEraseInfoflag(useeraseinfo);
	if(!traceuserid.empty()) WriteLog::SetTraceUser(traceuserid);
	RngSeedManager::SetRun(1);
	RngSeedManager::SetSeed(1);
	simple::MyBuilding::SetSeed(1);
	simple::MyBuilding::SetRun(run);
	simple::MyBuilding::SetEraseAoI(A_d);
	simple::MyBuilding::SetSendHopFlag(usehop);
	bool useexitinfoflag = false;
	if(useexitinfo == "ON"){
		useexitinfoflag = true;
	}
	simple::MyBuilding::SetUseExitInfoFlag(useexitinfoflag);
	simple::MyBuilding::Write_Logfile_for_simulation_environment();
}

void NetSim::GenerateRandomNodePosition (std::map<std::string, double> &pos)
{
	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());
	std::uniform_real_distribution<> dist(0.0, simple::MyBuilding::GetFieldWidth());
	double x = dist(engine);
	double y = dist(engine);
	double z = 0;

	// exit
	std::list<simple::Position> els = simple::MyBuilding::GetExitSection();
	for (const auto& sectionId : els)
	{
		double top = sectionId.y * simple::MyBuilding::GetSectionHight();
		double bottom = top + simple::MyBuilding::GetSectionHight();
		double left = sectionId.x * simple::MyBuilding::GetSectionWidth();
		double right = left + simple::MyBuilding::GetSectionWidth();
		if (x > left and x < right and y > top and y < bottom)
		{
			GenerateRandomNodePosition (pos);
			return;
		}
	}
	// blocked
	std::list<simple::Position> bls = simple::MyBuilding::GetBlockedSection();
	for (const auto& sectionId : bls)
	{
		double top = sectionId.y * simple::MyBuilding::GetSectionHight();
		double bottom = top + simple::MyBuilding::GetSectionHight();
		double left = sectionId.x * simple::MyBuilding::GetSectionWidth();
		double right = left + simple::MyBuilding::GetSectionWidth();
		if (x > left and x < right and y > top and y < bottom)
		{
			GenerateRandomNodePosition (pos);
			return;
		}
	}

	pos.insert (std::make_pair("x", x));
	pos.insert (std::make_pair("y", y));
	pos.insert (std::make_pair("z", z));
}

void NetSim::GenerateNodePosition (std::map<std::string, double> &pos)
{
	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());
	std::uniform_real_distribution<> dist(0.0, simple::MyBuilding::GetFieldWidth());
	double x = dist(engine);
	double y = dist(engine);
	double z = 0;

	// exit
	std::list<simple::Position> els = simple::MyBuilding::GetExitSection();
	for (const auto& sectionId : els)
	{
		double top = sectionId.y * simple::MyBuilding::GetSectionHight();
		double bottom = top + simple::MyBuilding::GetSectionHight();
		double left = sectionId.x * simple::MyBuilding::GetSectionWidth();
		double right = left + simple::MyBuilding::GetSectionWidth();
		if (x > left and x < right and y > top and y < bottom)
		{
			GenerateNodePosition (pos);
			return;
		}
	}
	// blocked
	std::list<simple::Position> bls = simple::MyBuilding::GetBlockedSection();
	for (const auto& sectionId : bls)
	{
		double top = sectionId.y * simple::MyBuilding::GetSectionHight();
		double bottom = top + simple::MyBuilding::GetSectionHight();
		double left = sectionId.x * simple::MyBuilding::GetSectionWidth();
		double right = left + simple::MyBuilding::GetSectionWidth();
		if (x > left and x < right and y > top and y < bottom)
		{
			GenerateNodePosition (pos);
			return;
		}
	}

	pos.insert (std::make_pair("x", x));
	pos.insert (std::make_pair("y", y));
	pos.insert (std::make_pair("z", z));
}

void OutputResult (uint32_t simtime)
{
	//	std::cout << "start output result." << std::endl;
	//	//simple::MySection section;
	//	simple::MyAccessPoint ap;
		simple::MyUser user;
	//
	//	int32_t sumtime;
	//	int32_t avgtime,worsttime;
		int exit1,exit2,exit3,exit4;
		std::map<uint32_t, std::list<uint32_t>> exited;
	//	uint32_t voltransfers_ap,voltransfers_user;
	//	uint32_t allvoltransfers_ap,allvoltransfers_user;
	//	uint32_t numLoops;
	//	sumtime = user.GetSumEvacuationTime();
	//	avgtime = sumtime/simple::MyBuilding::GetNumUsers();
	//	worsttime = user.GetWorstEvacuationTime();
	//	std::cout << "test000" << std::endl;
		exited = simple::MyBuilding::GetUserIdPassedExitDoor();
		exit1 = exited.at(1).size();
		exit2 = exited.at(2).size();
		exit3 = exited.at(3).size();
		exit4 = exited.at(4).size();
	//	std::cout << "test001" << std::endl;
	//	allvoltransfers_ap = 0;//ap.GetAllVolTransfers();
	//	allvoltransfers_user = 0;//user.GetAllVolTransfers();
	//	numLoops = user.GetNumLoops();
	//
	//	std::cout << "test111" << std::endl;

	//
	//	// output txt file
		char filename_txt[1000];
		std::ostringstream oss_delay,oss_run;
		oss_run << RngSeedManager::GetRun();

		String mode="ON";

		if(simple::RoutingProtocol::GetMode()){
			mode="ON";
		}else{
			mode="OFF";
		}
		String quickhello="QuickHello_ON";
		if(simple::RoutingProtocol::GetUseQuickHelloflag()){
			quickhello="QuickHello_ON";
		}else{
			quickhello="QuickHello_OFF";
		}	

		String eraseblock = "EraseBlock_ON";
		if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
			eraseblock="EraseBlock_ON";
		}else{
			eraseblock="EraseBlock_OFF";
		}

		std::string str_txt = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Result/SimulationResult_"+oss_run.str()+".txt";
		str_txt.copy(filename_txt,str_txt.size());
		filename_txt[str_txt.size()] = '\0';

		std::ofstream fout_txt;
		fout_txt.open(filename_txt);
		if(!fout_txt){
			std::cout<<"Could not open file."<<std::endl;
			exit(1);
		}
	//
		fout_txt<<"run:"<<RngSeedManager::GetRun()<<" seed:"<<RngSeedManager::GetSeed()<<std::endl;
	//
	//	fout_txt<<"volume of packet transfers for each AP------------"<<std::endl
	//		<<"  apid , voltransfers"<<std::endl;
	//	for (uint32_t apid=0; apid<simple::MyBuilding::GetNumSections(); apid++)
	//	{
	//		voltransfers_ap = 0;//ap.GetVolTransfers(apid);
	//		fout_txt<<std::setw(6)<<apid<<" , "<<std::setw(12)<<voltransfers_ap<<std::endl;
	//	}
	//	fout_txt<<"----------------------------------------------------"<<std::endl<<std::endl;
	//	fout_txt<<"volume of packet transfers for each user------------"<<std::endl
	//		<<"userid , voltransfers"<<std::endl;
	//	for (uint32_t userid=0; userid<simple::MyBuilding::GetNumUsers(); userid++)
	//	{
	//		voltransfers_user = 0;//user.GetVolTransfers(userid);
	//		fout_txt<<std::setw(6)<<userid<<" , "<<std::setw(12)<<voltransfers_user<<std::endl;
	//	}
	//	fout_txt<<"----------------------------------------------------"<<std::endl<<std::endl;
	//
		fout_txt
	//		<<"avgtime"<<avgtime<<std::endl
	//		<<"worsttime"<<worsttime<<std::endl
			<<"exit1:"<<exit1<<std::endl
			<<"exit2:"<<exit2<<std::endl
			<<"exit3:"<<exit3<<std::endl
			<<"exit4:"<<exit4<<std::endl;

		fout_txt.close();
	//
	//	std::cout << "test" << std::endl;
	//
	//	// output csv file
	//	char filename_csv[1000];
	//	std::string str_csv = "SimulationResult/"+oss_delay.str()+"/SimulationResult.csv";
	//	str_csv.copy(filename_csv,str_csv.size());
	//	filename_csv[str_csv.size()] = '\0';
	//
	//	std::ofstream fout_csv;
	//	fout_csv.open(filename_csv, std::ios::app);
	//	if(!fout_csv){
	//		std::cout<<"Could not open file."<<std::endl;
	//		exit(1);
	//	}
	//
	//	fout_csv<<RngSeedManager::GetRun()	<<","
	//			<<avgtime						<<","
	//			<<worsttime					<<","
	//			<<exit1						<<","
	//			<<exit2						<<","
	//			<<exit3						<<","
	//			<<exit4						<<","
	//			<<allvoltransfers_ap			<<","
	//			<<allvoltransfers_user		<<","
	//			<<numLoops						<<","
	//			<<std::endl;
	//
	//	fout_csv.close();
	//
	//	// output number of exit door passages per simulation time
	//	uint32_t numPassedUsers[1000];
	//	char filename_pertimes_csv[1000];
	//	std::string str_pertimes_csv = "SimulationResult/"+oss_delay.str()+"/SimulationResult_pertime.csv";
	//	str_pertimes_csv.copy(filename_pertimes_csv,str_pertimes_csv.size());
	//	filename_pertimes_csv[str_pertimes_csv.size()] = '\0';
	//
	//	std::ofstream fout_pertimes_csv;
	//	fout_pertimes_csv.open(filename_pertimes_csv, std::ios::app);
	//	if(!fout_pertimes_csv){
	//		std::cout<<"Could not open file."<<std::endl;
	//		exit(1);
	//	}
	//
	//	std::cout << "simtime: " << simtime << std::endl;
	//
	//	for (uint32_t i=0; i<simtime/5; i++)
	//	{
	//		//std::cout<<i<<std::endl;
	//		numPassedUsers[i] = 0;
	//		for (uint32_t userid=0; userid<simple::MyBuilding::GetNumUsers(); userid++)
	//		{/*
	//			if (user.GetCoordinateompletionTime(userid) >= i*5 && user.GetCoordinateompletionTime(userid) < (i+1)*5)
	//			{
	//				numPassedUsers[i]++;
	//			}*/
	//		}
	//	}
	//
	//	fout_pertimes_csv<<RngSeedManager::GetRun()<<",";
	//	for (uint32_t i=0; i<simtime/5; i++)
	//	{
	//		fout_pertimes_csv<<numPassedUsers[i]<<",";
	//	}
	//	fout_pertimes_csv<<std::endl;
	//
	//	fout_pertimes_csv.close();
	//	std::cout << "Finish output results." << std::endl;
}

int main(int argc, char **argv)
{
	// start measuring simulation time

	clock_t simstart = clock();
//
//	// set building model
//	simple::MySection::SetMySection();
//
//	// ns3 setting
	NetSim sim;
	simple::MyBuilding::SettingLoad();
	simple::MyBuilding::Write_Logfile_for_simulation_environment();
	if(simple::MyBuilding::GetExitmanualflag()){
		simple::MyBuilding::SetExitNodes();
	}
	simple::MyBuilding::SetRandomNum();
	sim.JSON_Data_Input();
	simple::MyBuilding::SetBlockedNodes2();
	sim.Configure(argc, argv);

//	sim.Initialize_ClusterView();
	sim.Initialize_Logfile_for_Recive();
	sim.Initialize_Logfile_for_HelloRecvTimeInfo();
	sim.Initialize_Logfile_for_HelloSendTimeInfo();
	sim.Initislize_logfile_for_TestLog();
	sim.Initialize_ClusterView();
	simple::JSON_Data::precalculate_shortest_paths();
	sim.Write_crossing_latlon();
	sim.Initialize_Logfile();
	sim.Initialize_Logfile_for_TotalDistance();
	sim.Initialize_Logfile_for_Block_Nodes();
	sim.Initialize_Logfile_for_AoiInfo();
	sim.Initialize_flow_Moniter();
	sim.Initialize_Logfile_SpeedInfo();
	sim.RunSimulation();  
	

// end measuring simulation time
	clock_t simend = clock();
	const double simtime = static_cast<double>(simend-simstart)/CLOCKS_PER_SEC;
	std::cout<<"simulation time: "<<simtime<<" s"<<std::endl;
	String s;
	if(simple::RoutingProtocol::GetUseQuickHelloflag() == true){
		s="ON";
	}else{
		s="OFF";
	}
	
	//////Lineへの終了通知//////////
	std::string command = "/bin/python3 /home/obayashi/ns-allinone-3.35/ns-3.35/LineNotify.py "+std::to_string(simple::MyBuilding::GetNumUsers())+' '+s+' '+std::to_string(simple::MyBuilding::GetUseExitInfoFlag())+' '+std::to_string(simtime);
    system(command.c_str());

	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag() == true){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}
	
	String eraseblock = "EraseBlock_ON";
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	String usernum = std::to_string(simple::MyBuilding::GetNumUsers());
	String addusernum = std::to_string(simple::MyBuilding::GetAddUsersNum());
	String path = "obayashiIOFiles/Log/obayashi/ON/"+quickhello + "/" + eraseblock;
	String output_file = "zip/obayashi/ON/" + quickhello + "/" + eraseblock + "/" + usernum + "+" + addusernum + ".zip";
	command = "cd " + path + " && zip -rq ../../../../../" + output_file + " " + usernum;
	std::cout<<"実行数："<<simple::MyBuilding::GetCount()<<std::endl;
	// command = "zip -r zip/"+quickhello+"/"+usernum+"+"+addusernum+".zip obayashiIOFiles/Log/obayashi/ON/"+quickhello+"/"+usernum;
    system(command.c_str());
//	OutputResult(sim.GetSimTime());

	return 0;
}
