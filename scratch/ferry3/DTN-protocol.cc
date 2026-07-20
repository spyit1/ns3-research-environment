/*
 * simple-protocol.cc
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/mobility-module.h"
// #include "ns3/ipv4-address.h"

#include <algorithm>
#include <limits>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <sstream>
#include <set>

//#include "simple-protocol.h"
#include "DTN-protocol.h"
//#include "DTN-helper.h"
#include "create-environment.h"
#include "section-data-trade.h"

NS_LOG_COMPONENT_DEFINE ("ClusterProtocol");

// uint32_t frequency = 0;

namespace ns3
{

namespace simple
{

// int frequency = 0;


NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);
Environment m_environment=Environment();

//UDP port for MYRTP control traffic
const uint32_t simple::RoutingProtocol::MY_PORT=5555;

SenderInfoList m_sil;
double diffusion=0.0;
double current_data_send=0.0;

unsigned long int packetOnP2P=0;
int flg = false;

int cml_size_num = 0;
int CN_num = 0;

unsigned long int controlPacketOnWifi=0;

std::unordered_map<DataKey,double,DataKey::Hash> dissemination_cache;
std::unordered_map< DataKey,std::unordered_map<int,double>,DataKey::Hash > dissemination_time;


std::map<Ipv4Address,int> num_cluster_cache;
std::map< int,std::map<Ipv4Address,int> > num_cluster;

double dissemination_P2P=0.0;
double dissemination_wifi=0.0;

uint32_t user_space_buf = 0;

std::map<int,int> hop_info;

std::list<std::pair<uint32_t,uint32_t>> section_info;

uint8_t getFlg = true;

std::list<StoreSectionData> SectionDataContainer;
ClusterAllMemberList allcluster;

int s_datasize = 0;
int section_datasize = 0;

u_int32_t addressIndex = 1;

bool RoutingProtocol::m_flag = true;
bool RoutingProtocol::quickhelloflag = false;
bool RoutingProtocol::eraseinfoflag = false;
uint32_t testnum = 0;

String buf;
//std::set<Ipv4Address> addressBuf; //setは重複を許さない


class DeferredRouteOutputTag : public Tag
{
public:
	DeferredRouteOutputTag (int32_t o = -1) : Tag (), m_oif (o) {}

	static TypeId GetTypeId ()
	{
		static TypeId tid = TypeId ("ns3::proto_simple::DeferredRouteOutputTag").SetParent<Tag> ()
						    						  .SetParent<Tag> ()
						    						  .AddConstructor<DeferredRouteOutputTag> ()
						    						  ;
		return tid;
	}

	TypeId  GetInstanceTypeId () const
	{
		return GetTypeId ();
	}

	int32_t GetInterface() const
	{
		return m_oif;
	}

	void SetInterface(int32_t oif)
	{
		m_oif = oif;
	}

	uint32_t GetSerializedSize () const
	{
		return sizeof(int32_t);
	}

	void  Serialize (TagBuffer i) const
	{
		i.WriteU32 (m_oif);
	}

	void  Deserialize (TagBuffer i)
	{
		m_oif = i.ReadU32 ();
	}

	void  Print (std::ostream &os) const
	{
		os << "DeferredRouteOutputTag: output interface = " << m_oif;
	}

private:
	/// Positive if output device is fixed in RouteOutput
	int32_t m_oif;
};


TypeId RoutingProtocol::GetTypeId(void)
{
	static TypeId tid=TypeId("ns3::simple::RoutingProtocol")
									.SetParent<Ipv4RoutingProtocol>()
									.AddConstructor<RoutingProtocol>()
									.AddAttribute("UpdateInterval",
											"Time interval for update routing table.default is 30s.",
											TimeValue(Seconds(30)),
											MakeTimeAccessor(&RoutingProtocol::UpdateInterval),
											MakeTimeChecker())
											.AddAttribute("ActiveRouteTimeout",
													"Period of time during which the route is considered to be valid",
													TimeValue(Seconds(3)),
													MakeTimeAccessor(&RoutingProtocol::ActiveRouteTimeout),
													MakeTimeChecker())
													.AddAttribute ("UniformRv",
															"Access to the underlying UniformRandomVariable",
															StringValue ("ns3::UniformRandomVariable"),
															MakePointerAccessor (&RoutingProtocol::m_uniformRandomVariable),
															MakePointerChecker<UniformRandomVariable> ())
															;
	return tid;
}

RoutingProtocol::RoutingProtocol():
								ActiveRouteTimeout(Seconds(3)),
								m_IdCache(Seconds(1)),
								m_hop(0),
								m_size(1),
								now(ON),
								cluster_cellular(0),
								during_merge(false),
								during_split(false),
								num_children(0),
								is_server(false),
								is_P2Pserver(false),
								do_clustering(false),
								m_broadcast_id(0),
								my_id(0),
								nodeId (0),
								m_nodeState(0),
								m_dataseqno(0),
								m_ackseqno(0),
								m_mepIdCache(Seconds(1)),
								m_mqapIdCache(Seconds(1)),
								m_cncpIdCache(Seconds(1)),
								m_dataIdCache(Seconds(1)),
								m_rreqIdCache(Seconds(1)),
								m_mepId(0),
								m_mqapId(0),
								m_cncpId(0),
								m_rreqId(0),
								m_stateId(0),
								recv_data_times(0),
								send_data_times(0),
								do_epidemic(true),//10/20 変更
								do_P2P(false),
								do_optimize(false),
								m_epidemicLimit(0),
								m_maxrange(240)
{
	NS_LOG_FUNCTION(this);
}

RoutingProtocol::~RoutingProtocol()
{
	NS_LOG_FUNCTION(this);

}

void RoutingProtocol::SetMode(String s)
{
	if(s=="ON"){
		m_flag=true;
	}
	if(s=="OFF"){
		m_flag=false;
	}
}

void RoutingProtocol::SetUseQuickHelloflag(String s){
	if(s=="ON"){
		// std::cout<<s<<std::endl;
		quickhelloflag=true;
	}
	if(s=="OFF"){
		quickhelloflag=false;
	}
}

void RoutingProtocol::SetUseEraseInfoflag(String s){
	if(s=="ON"){
		// std::cout<<s<<std::endl;
		eraseinfoflag=true;
	}
	if(s=="OFF"){
		eraseinfoflag=false;
	}

}

void RoutingProtocol::DoDispose()
{
	m_ipv4=0;
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::iterator iter=m_socketAddresses.begin(); iter!=m_socketAddresses.end(); iter++) {
		iter->first->Close();
	}
	m_socketAddresses.clear();
	Ipv4RoutingProtocol::DoDispose();
}

void RoutingProtocol::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
	//ルーティングテーブルのエントリーを出力する処理を行う
	//実際にはルーティングテーブルクラスのPrintメソッドにより処理する

	*stream->GetStream() << "Node: " << m_ipv4->GetObject<Node>()->GetId()
					     << " Time: " << Simulator::Now().GetSeconds() << "s ";

	m_routingTable.Print(stream);
	*stream->GetStream () << std::endl;
}

int64_t RoutingProtocol::AssignStreams(int64_t stream)
{
	NS_LOG_FUNCTION(this << stream);
	//乱数系列の割り当て処理
	m_uniformRandomVariable->SetStream(stream);
	return 1;
}

Ptr<Ipv4Route> RoutingProtocol::RouteOutput(Ptr<Packet> p,const Ipv4Header &header,Ptr<NetDevice> oif,Socket::SocketErrno &sockerr)
{
	NS_LOG_FUNCTION(this << header << " " << (oif ? oif->GetIfIndex():0));

	//パケットの転送経路を決定するための処理を行う
	sockerr = Socket::ERROR_NOTERROR;
	Ptr<Ipv4Route> route;
	Ipv4Address dst = header.GetDestination ();
	RoutingTableEntry rt;
	if (m_routingTable.LookupValidRoute (dst, rt)) //テーブルに終点への有効な経路があるかを検索
	{                                                         //あればその経路を選択
		route = rt.GetRoute ();
		NS_ASSERT (route != 0);
		NS_LOG_DEBUG ("Exist route to " << route->GetDestination () << " from interface " << route->GetSource ());
		if (oif != 0 && route->GetOutputDevice () != oif)
		{
			//std::cout<<"Output device doesn't match. Dropped."<<std::endl;
			sockerr = Socket::ERROR_NOROUTETOHOST;
			return Ptr<Ipv4Route> ();
		}

		return route;
	}

	uint32_t iif = (oif ? m_ipv4->GetInterfaceForDevice (oif) : -1);
	DeferredRouteOutputTag tag (iif);
	//std::cout<< "Valid Route not found"<<std::endl;
	if (!p->PeekPacketTag (tag))
	{
		p->AddPacketTag (tag);
	}
	return LoopbackRoute (header, oif);
}

bool RoutingProtocol::RouteInput(Ptr<const Packet> p,const Ipv4Header &header,Ptr<const NetDevice> idev,
		UnicastForwardCallback ucb,MulticastForwardCallback mcb,LocalDeliverCallback lcb,ErrorCallback ecb)
{
	NS_LOG_FUNCTION(this);

	NS_ASSERT (m_ipv4 != 0);
	NS_ASSERT (p != 0);
	// Check if input device supports IP
	NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
	int32_t iif = m_ipv4->GetInterfaceForDevice (idev);

	Ipv4Address dst = header.GetDestination ();
	Ipv4Address origin = header.GetSource ();

	// Broadcast local delivery/forwarding
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
			m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
	{
		Ipv4InterfaceAddress iface = j->second;
		if (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()) == iif)
			if (dst == iface.GetBroadcast () || dst.IsBroadcast ())
			{
				Ptr<Packet> packet = p->Copy ();
				if (lcb.IsNull () == false)
				{
					NS_LOG_LOGIC ("Broadcast local delivery to " << iface.GetLocal ());
					lcb (p, header, iif);
					// Fall through to additional processing
				}
				else
				{
					NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << origin);
					ecb (p, header, Socket::ERROR_NOROUTETOHOST);
				}

				if (header.GetTtl () > 1)
				{
					NS_LOG_LOGIC ("Forward broadcast. TTL " << (uint16_t) header.GetTtl ());
					RoutingTableEntry toBroadcast;
					if (m_routingTable.LookupRoute (dst, toBroadcast))
					{
						Ptr<Ipv4Route> route = toBroadcast.GetRoute ();
						ucb (route, packet, header);
					}
					else
					{
						NS_LOG_DEBUG ("No route to forward broadcast. Drop packet " << p->GetUid ());
					}
				}
				return true;
			}
	}

	// Unicast local delivery
	if (m_ipv4->IsDestinationAddress (dst, iif))
	{
		RoutingTableEntry toOrigin;

		if (lcb.IsNull () == false)
		{
			NS_LOG_LOGIC ("Unicast local delivery to " << dst);
			lcb (p, header, iif);
		}
		else
		{
			std::cout<<"Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << origin<<std::endl;
			ecb (p, header, Socket::ERROR_NOROUTETOHOST);
		}
		return true;
	}
	// Forwarding

	return Forwarding (p, header, ucb, ecb);
}

bool RoutingProtocol::Forwarding(Ptr<const Packet> p,const Ipv4Header &header,UnicastForwardCallback ucb,ErrorCallback ecb)
{
	NS_LOG_FUNCTION(this);

	Ipv4Address dst=header.GetDestination();
	Ipv4Address origin=header.GetSource();
	RoutingTableEntry toDst;

	if (m_routingTable.LookupRoute (dst, toDst))
	{
		if (toDst.GetFlag () == VALID)
		{
			Ptr<Ipv4Route> route = toDst.GetRoute ();
			RoutingTableEntry toOrigin;
			m_routingTable.LookupRoute (origin, toOrigin);
			ucb (route, p, header);
			return true;
		}
	}
	return false;
}

void RoutingProtocol::SetIpv4(Ptr<Ipv4> ipv4)
{
	NS_LOG_FUNCTION(this);

	//フォワーディングテーブルのインスタンスを取得する
	//プロトコルを開始させる
	m_ipv4 = ipv4;
	m_lo = m_ipv4->GetNetDevice (0);
	Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}

//application layer
void RoutingProtocol::Start()
{
	
	NS_LOG_FUNCTION (this);

	WriteLog();

	if(m_environment.GetProtocol().find("CLUSTER")!=std::string::npos){
		do_clustering=true;
	}

	// int i = 0;

	switch (nodeType) {
	case EVC_USER: {
		Ptr<MyUser> node = m_ipv4->GetObject<MyUser>();
		Ptr<Socket> socket;
		for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j) {
			socket = j->first;
		}
		node->SetSocket(socket);
		node->HandleUserMovementTimer(Seconds(1.0));
		CheckMovingUserNumTimer(MyBuilding::GetMovingUserNumInterval());
		WriteAoiLog();
		WriteBlockAoiLog();
		if(m_flag){
			ActivateHelloTimer(60);		
		}
		break;
	}
	case EVC_AP:{
		Ptr<MyUser> hinanjo = m_ipv4->GetObject<MyUser>();
		Ptr<Socket> socket;
		for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j) {
			socket = j->first;
		}
		hinanjo->SetSocket(socket);

		if(m_flag){
			HandleShelterData();
		}

		now = EXIT;
		break;
	}	
	case ADD_EVC_USER:{
		Ptr<MyUser> addnode = m_ipv4->GetObject<MyUser>();
		Ptr<Socket> socket;
		for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j) {
			socket = j->first;
		}
		addnode->SetSocket(socket);
		std::vector<uint32_t> randomnum = MyBuilding::GetRandomNum();
		addnode->AddUserMovementTimer(Seconds(0), randomnum);
		WriteAoiLog();
		WriteBlockAoiLog();
		if(m_flag){
			AdduserHelloTimer(0,randomnum);
		}
		
		CheckMovingAddUserNumTimer(randomnum);
		break;
	}
	default:
		break;
	}
}

void RoutingProtocol::SetSocket(){
	Ptr<MyUser> addnode = m_ipv4->GetObject<MyUser>();
	Ptr<Socket> socket;
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j) {
		socket = j->first;
	}
	addnode->SetSocket(socket);
}

void RoutingProtocol::SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination, int packetType)
{
	RoutingTable rtable;
	socket->SendTo(packet, 0, InetSocketAddress(destination, MY_PORT));
	rtable.DeleteRoute(destination);
	OutputText_RW(Simulator::Now(), packetType, destination);

}

void RoutingProtocol::NotifyInterfaceUp(uint32_t i)
{
	// std::cout << "NotifyInterfaceUp: " << i << std::endl;

	NS_LOG_FUNCTION(this << m_ipv4->GetAddress(i, 0).GetLocal ());
	Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
	if (l3->GetNAddresses (i) > 1)
    {
      NS_LOG_WARN ("AODV does not work with more then one address per each interface.");
    }
	Ipv4InterfaceAddress iface = l3->GetAddress(i, 0);
	if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
	{
		return;
	}

	// 該当するI/Fの専用ソケットを生成する
	// Create a socket to listen only on this interface
	Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
	NS_ASSERT (socket != 0);
	socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvProto, this));
	socket->Bind (InetSocketAddress (iface.GetLocal (), MY_PORT));
	// socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), MY_PORT));
	socket->BindToNetDevice (l3->GetNetDevice (i));
	socket->SetAllowBroadcast (true);
	socket->SetIpRecvTtl (true);
	socket->SetAttribute ("IpTtl", UintegerValue (1));
	m_socketAddresses.insert (std::make_pair (socket, iface));


	socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
	NS_ASSERT (socket != 0);
	socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvProto, this));
	//	socket->Bind (InetSocketAddress (iface.GetLocal (), MY_PORT));
	socket->Bind (InetSocketAddress (iface.GetBroadcast(), MY_PORT));
	socket->BindToNetDevice (l3->GetNetDevice (i));
	socket->SetAllowBroadcast (true);
	socket->SetIpRecvTtl (true);
	socket->SetAttribute ("IpTtl", UintegerValue (1));
	m_socketSubnetBroadcastAddresses.insert (std::make_pair (socket, iface));

	// ルーティングテーブルのエントリーを設定する
	// Add local blodcast record to the routing table
	Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
	Ipv4Mask mask=iface.GetMask();
	Ipv4Address netAddress=iface.GetLocal().CombineMask(mask);
	std::string iftype=(dev->IsPointToPoint()) ? "p2p" : "csma";

	RoutingTableEntry rt(
		/*device=*/	dev,
		/*dst=*/	netAddress,
		/*iface=*/	iface,
		/*nexthop=*/ Ipv4Address ("0.0.0.0"),
		/*matrics=*/ 1,
		/*rtype=*/	"C",
		/*iftype=*/	iftype,
		/*lifetime=*/	Simulator::GetMaximumSimulationTime()
	);
	m_routingTable.AddRoute(rt);

	m_hop=0;
	m_size=1;
	now=ON;
	m_id=iface.GetLocal();  //10.0.0.1 ~ 10.0.0.100
	c_id=iface.GetLocal();  //10.0.0.1 ~ 10.0.0.100
	m_parent=iface.GetLocal(); //10.0.0.1 ~ 10.0.0.100
	b_cid=iface.GetLocal();  //10.0.0.1 ~ 10.0.0.100
	nodeId = m_ipv4->GetObject<Node>()->GetId();
	my_id = m_ipv4->GetObject<Node>()->GetId()+1;
	if (nodeId < MyBuilding::GetNumUsers()){
		nodeType = EVC_USER;
	}else if(nodeId >= MyBuilding::GetNumUsers() && nodeId < MyBuilding::GetNumUsers()+MyBuilding::GetAddUsersNum()){
		// std::cout<<"ADD_EVC_USER"<<std::endl;
		nodeType = ADD_EVC_USER;
	}else if(nodeId >= MyBuilding::GetNumUsers()+MyBuilding::GetAddUsersNum()){
		nodeType = EVC_AP;
	}

}

void RoutingProtocol::NotifyInterfaceDown(uint32_t i)
{
	NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ());

	// Close socket
	// now=EXIT;
	Ptr<Socket> socket = FindSocketWithInterfaceAddress (m_ipv4->GetAddress (i, 0));
	NS_ASSERT (socket);
	socket->Close ();
	m_socketAddresses.erase (socket);
}

void RoutingProtocol::NotifyAddAddress(uint32_t i,Ipv4InterfaceAddress address)
{
	NS_LOG_FUNCTION(this << " interface " << i << " address " << address);
	Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
	if (!l3->IsUp (i))
	{
		return;
	}
	if (l3->GetNAddresses (i) == 1)
	{
		//...
	}
	else
	{
		NS_LOG_LOGIC ("AODV does not work with more then one address per each interface. Ignore added address");
	}
}

void RoutingProtocol::NotifyRemoveAddress(uint32_t i,Ipv4InterfaceAddress address)
{
	NS_LOG_FUNCTION (this);
	Ptr<Socket> socket = FindSocketWithInterfaceAddress (address);
	if (socket)
	{
		//...
	}
	else
	{
		NS_LOG_LOGIC ("Remove address not participating in AODV operation");
	}
}

//network layer
Ptr<Ipv4Route> RoutingProtocol::LoopbackRoute(const Ipv4Header &hdr,Ptr<NetDevice> oif) const
{
	NS_LOG_FUNCTION(this << hdr);
	NS_ASSERT (m_lo != 0);
	Ptr<Ipv4Route> rt=Create<Ipv4Route>();
	//自分へ配信する際の処理
	rt->SetDestination (hdr.GetDestination ());

	std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
	if (oif)
	{
		// Iterate to find an address on the oif device
		for (j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
		{
			Ipv4Address addr = j->second.GetLocal ();
			int32_t interface = m_ipv4->GetInterfaceForAddress (addr);
			if (oif == m_ipv4->GetNetDevice (static_cast<uint32_t> (interface)))
			{
				rt->SetSource (addr);
				break;
			}
		}
	}
	else
	{
		rt->SetSource (j->second.GetLocal ());
	}
	// NS_ASSERT_MSG (rt->GetSource () != Ipv4Address (), "Valid AODV source address not found");
	rt->SetGateway (Ipv4Address ("127.0.0.1"));
	rt->SetOutputDevice (m_lo);
	return rt;
}

//network
void RoutingProtocol::RecvProto (Ptr<Socket> socket)
{
	// std::cout<<"RecvProto"<<std::endl;
	Ptr<MyUser> node = m_ipv4->GetObject<MyUser>();
	NS_LOG_FUNCTION(this << socket);
	Address sourceAddress;
	Ptr<Packet> packet = socket->RecvFrom(sourceAddress);
	InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom(sourceAddress);
	Ipv4Address sender = inetSourceAddr.GetIpv4();
	Ipv4Address receiver;
	if (m_socketAddresses.find(socket) != m_socketAddresses.end()){
		receiver = m_socketAddresses[socket].GetLocal();
	}else if(m_socketSubnetBroadcastAddresses.find(socket) != m_socketSubnetBroadcastAddresses.end()){
		receiver = m_socketSubnetBroadcastAddresses[socket].GetLocal();
	}

	TypeHeader tHeader(TYPE_USERDATA);
	packet->RemoveHeader(tHeader);
//	std::cout<<packet->RemoveHeader(tHeader)<<std::endl;
	// std::cout << "header>> " << tHeader.Get () << std::endl;

	if (!tHeader.IsValid ())
	{
		NS_LOG_DEBUG ("AODV message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
		std::cout<<"ヘッダーがついていません"<<std::endl;
		return; // drop
	}

	switch (tHeader.Get()) {
	case TYPE_USERDATA:
		RecvUserDataAsUnicast(packet, receiver, sender);
		break;
	case TYPE_HELLO:
		if(node->GetWaitStatus() == true){ //ユーザが動いていないならHelloを受け取らない
			return;
		}
		if(node->GetStatus() == EXIT){ //避難済みユーザはHelloを受け取らない
			return;
		}
		RecvHello(packet, receiver, sender);
		break;
	case TYPE_SHELTERDATA:
		if(node->GetWaitStatus() == true){ //ユーザが動いていないなら避難所の情報を受け取らない
			return;
		}
		if(node->GetStatus() == EXIT){ //避難済みユーザは避難所の情報を受け取らない
			return;
		}
		RecvShelterDataAsBroadcast(packet);
		break;
	default :
		std::cout<<"default####################################################" <<std::endl;
		break;
	}
}

Ptr<Socket> RoutingProtocol::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const
{
	NS_LOG_FUNCTION (this << addr);
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j=m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
	{
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;
		if (iface == addr)
			return socket;
	}
	Ptr<Socket> socket;
	return socket;
}


void RoutingProtocol::SetRoute(Ipv4Address next,Ipv4Address dst){
	RoutingTableEntry rt;

	Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (m_id));
	RoutingTableEntry newEntry(
			/*device=*/ dev,
			/*dst=*/ dst,
			/*iface=*/ m_ipv4->GetAddress (m_ipv4->GetInterfaceForAddress (m_id), 0),
			/*nexthop=*/ next,
			/*matrics=*/ 1,
			/*rtype=*/ "C",
			/*iftype=*/ "csma",
			/*lifetime=*/ ActiveRouteTimeout
	);

	if(!m_routingTable.LookupRoute(dst,rt)){
		if(my_id==1) {
//			std::cout<<"dst:"<<dst<<std::endl;
		}
		m_routingTable.AddRoute(newEntry);
	}else{
		m_routingTable.Update(newEntry);
		//rt.Print();
	}

}


void RoutingProtocol::HandleShelterData(){
	Ptr<MyUser> node = m_ipv4->GetObject<MyUser>();
	//ここに避難所と収容人数のデータを入れる
	std::pair<double,double>CurrentCoordinate = node->GetCurrentCoordinate(); //現在の位置（避難所の座標を取得）
	String ShelterNodeId = JSON_Data::GetIDfromCoordinate(CurrentCoordinate); //避難所のノードIDを取得
	uint32_t ShelterNodeIdtoInt = JSON_Data::NodeIDstoi(ShelterNodeId);
	std::map<String,uint32_t> exit_info= MyBuilding::GetExitNodes();
	std::map<String,uint32_t> first_exit_info= MyBuilding::Get_first_exit_info();
	uint32_t first_user_space;
	first_user_space = first_exit_info.at(ShelterNodeId);
	uint32_t user_space;
	user_space=exit_info.at(ShelterNodeId); //残りの避難可能人数を取得

	if(first_user_space == user_space /*|| user_space_buf == user_space*/){ //誰も避難しておらず、収容可能人数が減っていなければ情報を送信しない
		std::cout<<"避難者数に変化がありません。"<<std::endl;
		std::cout<<"user_space_buf = "<<user_space_buf<<std::endl;
		HandleShelterDataTimer(MyBuilding::GetShelterDataInterval());
		return;
	}

	uint32_t time = Simulator::Now().GetInteger()/1000000000; //現在時刻を秒で取得
	ShelterInfo data;
	data = ShelterInfo(ShelterNodeIdtoInt, user_space, time);
	user_space_buf = user_space;
	SendShelterInfoAsBroadcast(data);
}

void RoutingProtocol::WriteBlockAoiLog(){
	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();
	std::map<String, float> blockdata = user->GetBlockedNodeID_and_time();
	uint32_t userID = user->GetUserId() + 1;
	bool flag = user->GetExitflag();

	if(flag == true){ //避難完了していれば何もしない
		return;
	}

	char filename[100];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";
	if(GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}

	String quickhello="QuickHello_ON";
	if(GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}

	String eraseblock = "EraseBlock_ON";
	if(GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}
	std::string str = "Log/obayashi/" + mode + "/" + quickhello +"/" +eraseblock+ "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/AoI_Info/Block_AoI_Info_"  +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::app);

	for(auto a : blockdata){
		double Aoi = (Simulator::Now().GetDouble()/1000000000) - (a.second);
		fout << userID <<", "<<a.first<<", "<<Simulator::Now().GetDouble()/1000000000<<", "<<Aoi<<std::endl;
	}
	fout.close();
	BlockAoiTimer(1.0);
}

void RoutingProtocol::WriteAoiLog(){
	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();
	std::map<String, std::pair<uint32_t, uint32_t>> data; //ID,収容人数,時刻
	uint32_t userID = user->GetUserId() + 1;
	data = user -> GetAoiExitinfo();
	String exitnode = user -> GetExitnode();
	bool flag = user->GetExitflag();
	std::pair<uint32_t, uint32_t> data_pair;
	try{
		data_pair = data.at(exitnode);
	}catch(const std::out_of_range& e){
		AoiTimer(1.0);
		return;
	}

	if(flag == true){ //避難完了していれば何もしない
		return;
	}

	char filename[100];
	char filename2[100];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";
	if(GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}

	String quickhello="QuickHello_ON";
	if(GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}

	String eraseblock = "EraseBlock_ON";
	if(GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}
	std::string str = "Log/obayashi/" + mode + "/" + quickhello +"/"  +eraseblock+ "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/AoI_Info/AoI_Info_"  +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	/////////////////目指している避難所のみのAoIを出力//////////////////////////////////////////
	std::ofstream fout;
	fout.open(filename,std::ios::app);
	double Aoi = (Simulator::Now().GetDouble()/1000000000) - (data_pair.second);
	fout << userID <<", "<<exitnode<<", "<<Simulator::Now().GetDouble()/1000000000<<", "<<Aoi<<std::endl;
	fout.close();
	////////////////////全避難所のAoIを取得///////////////////////

	std::string str2 = "Log/obayashi/" + mode + "/" + quickhello +"/" +eraseblock+ "/"  + std::to_string(simple::MyBuilding::GetNumUsers()) + "/AoI_Info/ALL_AoI_Info_"  +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str2.copy(filename2,str2.size());
	filename2[str2.size()] = '\0';
	std::ofstream fout2;
	fout2.open(filename2,std::ios::app);
	for(auto a : data){
		double Aoi = (Simulator::Now().GetDouble()/1000000000) - (a.second.second);

		// fout << "UserID:"<< userID <<", EXIT_ID:"<< a.first <<", Time:"<<a.second.second<<", AoI:"<< Aoi <<std::endl;
		fout2 << userID <<", "<<a.first<<", "<<Simulator::Now().GetDouble()/1000000000<<", "<<Aoi<<std::endl;
	}
	// fout << std::endl;
	fout2.close();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AoiTimer(1.0);
}

void RoutingProtocol::CheckMovingUserNum(){
	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();
	bool checkflag = user->GetCheckMoveflag();
	bool Exit_flag = user->GetExitflag();
	bool alreadycountflag = user->GetAlreadyCountflag();
	uint32_t user_num;
	std::map<String, uint32_t> moving_user_num = MyBuilding::GetMovingUserNum();
	String ExitNodeID = user->GetExitnode();

	try{
		user_num = moving_user_num.at(ExitNodeID);
	}catch(const std::out_of_range& e){
		// std::cout<<"目指す避難所が決まっていません！！（これ出たらエラー）"<<std::endl;
		CheckMovingUserNumTimer(MyBuilding::GetMovingUserNumInterval());
		return;
	}

	if(checkflag == true && Exit_flag == false){ //すでにカウント済みかチェックする
		//すでにカウントされている移動中のユーザの処理
		CheckMovingUserNumTimer(MyBuilding::GetMovingUserNumInterval());
		// return;
	}

	if(checkflag == true && Exit_flag == true && alreadycountflag == false){ //避難済みならば減らす
		//すでに一度カウントされており、そのユーザが避難したときに行う処理（ユーザ数を減らす処理）
		user_num--;
		moving_user_num.erase(ExitNodeID); //mapは上書きされないため一旦消去
		moving_user_num.insert(std::make_pair(ExitNodeID, user_num));
		MyBuilding::SetMovingUserNum(moving_user_num);
		user->SetAlreadyCountflag();

		//ここでcheckflagをfalseにしたら避難所変更にも対応できるかも？

		// MovingUserNumTimer.Cancel();
		// return;
	}

	if(checkflag == true && Exit_flag == true && alreadycountflag == true) {
		//減算済みの場合何もしない
		//ユーザが一度カウントされていてかつ避難済みでかつ、一度避難したことを確認されている場合
		MovingUserNumTimer.Remove();
		return;
	}

	// uint32_t user_num = moving_user_num.at(ExitNodeID);
	if(Exit_flag == false && checkflag == false && alreadycountflag == false){
		//ユーザが避難していないかつ、一度もユーザ数として加算されていないかつ、避難したことも確認されていない場合
		user_num++;
		moving_user_num.erase(ExitNodeID); //mapは上書きされないため一旦消去
		moving_user_num.insert(std::make_pair(ExitNodeID, user_num));
		MyBuilding::SetMovingUserNum(moving_user_num);
		user->SetCheckMoveflag(); //一度カウントされたことを記録する
	}

	char filename[100];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";
	if(GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}

	String quickhello="QuickHello_ON";
	if(GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}

	String eraseblock = "EraseBlock_ON";
	if(GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "Log/obayashi/" + mode + "/"  + quickhello +"/" +eraseblock+ "/"  + std::to_string(simple::MyBuilding::GetNumUsers()) + "/AoI_Info/moving_user_nun_"  +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::app);
	for(auto a : moving_user_num){
		fout <<Simulator::Now().GetDouble()/1000000000<<", "<<a.first<<", "<<a.second<<std::endl;
	}
	fout.close();

	char filename2[100];
	std::string str2 = "Log/obayashi/" + mode + "/" + quickhello +"/" +eraseblock+ "/" + std::to_string(simple::MyBuilding::GetNumUsers()) +  "/AoI_Info/all_moving_user_nun_"  +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str2.copy(filename2,str2.size());
	filename2[str2.size()] = '\0';
	std::ofstream fout2;
	fout2.open(filename2,std::ios::app);
	uint32_t sum = 0;
	for(auto a : moving_user_num){
		sum = sum + a.second;
	}
	fout2 <<Simulator::Now().GetDouble()/1000000000<<", "<<sum<<std::endl;
	fout2.close();

	//////////ここですべてのAoIを記録する関数を呼ぶ////////////////

	////////////////////////////////////////////////////////////////
	CheckMovingUserNumTimer(MyBuilding::GetMovingUserNumInterval());
}

// void RoutingProtocol::HandleUserData(Ipv4Address sender, uint32_t sender_userID)
// {	
// 	Ptr<MyUser> node = m_ipv4->GetObject<MyUser>();
// 	uint32_t userId = node->GetUserId() + 1;
// 	uint32_t num = MyBuilding::GetNumUsers()+MyBuilding::GetAddUsersNum();

// 	if(sender_userID > num){
// 		std::cout<<"避難所です"<<std::endl;
// 		return;
// 	}

// 	if(userId > num){
// 		std::cout<<"避難所です"<<std::endl;
// 		return;
// 	}
	
// 	String findblockednodeid = node->GetFindBlockedNodeID();
// 	node->SetBlockedNodeID(findblockednodeid);
// 	std::set<String> blockednodeid = node->GetBlockedNodeIDSet();
// 	std::set<String> blockednodeid_buf = node->GetBlockedNodeIDBuf();
// 	std::set<Ipv4Address> addressBuf = node->GetAlreadySentAddressSet();
// 	String blockedid;
// 	String kara; //null文字
// 	std::set<String> fullexitnodeid = node->GetFullexitNodeIDSet();

// 	////////////////////////////////避難所情報///////////////////////////////
// 	std::map <String, std::pair<uint32_t, uint32_t>> AoIexitInfo; //nodeID, 残りの収容可能人数, 時刻情報
// 	std::map <String, std::pair<uint32_t, uint32_t>> AoIexitInfo_Buf; //一つ前のnodeID, 残りの収容可能人数, 時刻情報
// 	AoIexitInfo = node->GetAoiExitinfo();
// 	AoIexitInfo_Buf = node->GetAoiExitinfo_buf();
// 	/////////////////////////////////////////////////////////////////////////

// 	// uint32_t status = node->GetStatus(); 

// 	blockednodeid_buf = node->GetBlockedNodeIDBuf();
// 	blockednodeid = node->GetBlockedNodeIDSet(); //最新情報を取得
// 	blockednodeid_buf.erase(kara);
// 	blockednodeid.erase(kara);

// 	UserData data = MakeUserData(userId, c_id, blockednodeid, fullexitnodeid, AoIexitInfo);
// 	// m_db.AddData(data);
// 	// m_dkl.AddKey(data.GetKey());

// 	if(blockednodeid.empty() && fullexitnodeid.empty()){
// 		bool flag = false;
// 		for(auto a : AoIexitInfo){
// 			if(a.second.second == 0){
// 				// std::cout<<"初期の避難所情報です"<<std::endl;
// 			}else{
// 				// std::cout<<"避難所情報だけ初期情報より更新があります"<<std::endl;
// 				flag = true;
// 				break;
// 			}
// 		}
// 		// std::cout<<"まだ情報を１つも所持していません"<<std::endl;
// 		if(flag == false){
// 			return;
// 		}
// 		// std::cout<<"避難所情報だけ初期情報より更新があります"<<std::endl;
// 	}

// 	if(addressBuf.count(sender) == 1){ // 以前に送ったことのあるアドレスの場合実行される
// 		// std::cout<<"一度送信したことのある宛先アドレスです&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<std::endl;
// 		std::ofstream writing_file;
// 		std::string filename = "Log/obayashi/ON/testLog/testlog.txt";
// 		writing_file.open(filename, std::ios::app);
// 		writing_file << userId+1  <<" buf:";

// 		for(auto x : blockednodeid_buf){
// 			writing_file <<x<<",";
// 		}

// 		writing_file << " time = " <<Simulator::Now().GetDouble()/1000000000 << std::endl;
// 		writing_file << userId+1  << " blockedid:";

// 		for(auto y : blockednodeid){
// 			writing_file <<y<<",";
// 		}
// 		writing_file << " time = " << Simulator::Now().GetDouble()/1000000000 << std::endl;

// 		std::set<String> fullexitnodeid_buf = node->GetFullexitNodeIDSet_Buf();
// 		if(blockednodeid_buf.size() == blockednodeid.size() && fullexitnodeid.size() == fullexitnodeid_buf.size() && AoIexitInfo == AoIexitInfo_Buf){
// 			writing_file << "新しい情報をもっていません" << std::endl;
// 			// std::cout<<"新しい情報をもっていません"<<std::endl;
// 			for(auto ab : blockednodeid){
// 				node->SetBlockedNodeIDBuf(ab);
// 			}
// 			return;
// 		}
// 		node->ClearAlreadySentAddress(); //情報が更新された場合送信済みアドレスsetをクリアする
// 		//return;
// 	}

// 	if(blockednodeid_buf.size() != blockednodeid.size()){
// 		node->ClearAlreadySentAddress();
// 	}

// 	for(auto z : blockednodeid){
// 		node->SetBlockedNodeIDBuf(z);
// 	}

// 	node->SetFullexitNodeID_Buf(fullexitnodeid);
// 	node->SetAoiExitinfo_buf(AoIexitInfo);
// 	SendUserDataAsUnicast(data, sender);
// }

// TestData RoutingProtocol::MakeTestData(uint32_t testnum, Ipv4Address dst){
// 	Ptr<MyUser> node = m_ipv4->GetObject<MyUser>();
	// uint32_t dataseqno = node->GetSeqNo();
	// dataseqno++;
	// node->SetSeqNo(dataseqno);
	// dataseqno += nodeId*pow(10,7); // userid | seqno
	//TestData data(testnum, dst);
	// uint32_t num = node->GetNumTransfers();
	// uint32_t vol = node->GetVolTransfers();
	// num++;
	// vol += 4+1+4+4+4+4+4+4+4+4*MyBuilding::GetExitNodes().size();
	// node->SetNumTransfers(num);
	// node->SetVolTransfers(vol);

	// Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
	// Vector3D pos = mob->GetPosition ();

	// m_sil[data.GetKey()] = SenderInfo(pos.x, pos.y, Simulator::Now().GetSeconds());
	//return data;
// }

// User and LF
// UserData RoutingProtocol::MakeUserData(uint32_t userId, Ipv4Address dst, std::set<String>b_set, std::set<String>fullexit_set, std::map <String, std::pair<uint32_t, uint32_t>>aoi_exitinfo)
// {
// 	Ptr<MyUser> node = m_ipv4->GetObject<MyUser>();
// 	uint32_t dataseqno = node->GetSeqNo();
// 	dataseqno++;
// 	node->SetSeqNo(dataseqno);
// 	dataseqno += nodeId*pow(10,7); // userid | seqno
// 	MyBuilding::SetBlockedNodesbuf_set(b_set);
// 	UserData data(userId, dst, b_set, fullexit_set, aoi_exitinfo);
// 	std::set<String> block = data.GetBlockedNodeIDSet();
// 	uint32_t num = node->GetNumTransfers();
// 	num++;
// 	node->SetNumTransfers(num);
// 	// node->SetVolTransfers(vol);
// 	// Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
// 	// Vector3D pos = mob->GetPosition ();
// 	// m_sil[data.GetKey()] = SenderInfo(pos.x, pos.y, Simulator::Now().GetSeconds());
// 	return data;
// }

void RoutingProtocol::CheckHelloData(UserData data, Ipv4Address sender){
	bool sendflag = false;
	////////////////////////自分のデータ取得/////////////////////////////
	Ptr<MyUser> mydata = m_ipv4->GetObject<MyUser>();
	std::set<String> m_blockednodeid = mydata->GetBlockedNodeIDSet();
	std::set<String> m_fullexitnodeid = mydata->GetFullexitNodeIDSet();
	std::map<String, std::pair<uint32_t, uint32_t>> m_aoi_data = mydata->GetAoiExitinfo(); //nodeID, 残りの収容可能人数, 時刻情報
	std::map<String, float> m_aoi_blocked_data = mydata->GetBlockedNodeID_and_time();
	// std::map<String, std::pair<uint32_t, uint32_t>> m_aoi_data_buf = mydata->GetAoiExitinfo_buf();
	std::string kara = "";
	//////////////////////////////////受信データから取得////////////////////////////////////////
	std::set<String> r_blockednodeid = data.GetBlockedNodeIDSet();
	std::set<String> r_fullexitnodeid = data.GetFullExitNodeIDSet();
	std::map<String, std::pair<uint32_t, uint32_t>> r_exit_aoi_data = data.GetAoIExitinfo(); //nodeID, 残りの収容可能人数, 時刻情報
	std::map<String, float> r_aoi_blocked_data = data.GetAoIBlockedInfo();
	m_blockednodeid.erase(kara);
	m_fullexitnodeid.erase(kara);
	r_blockednodeid.erase(kara);
	r_fullexitnodeid.erase(kara);
	/////////////////////////////送り返すデータのもと////////////////////////////////////
	uint32_t userID = mydata->GetUserId();
	Ipv4Address dst = sender; //Hello送信元が送り返す宛先になるから
	std::set<String> s_blockednodeid;
	std::set<String> s_fullexitnodeid;
	std::map<String, std::pair<uint32_t, uint32_t>> s_exit_aoi_data;
	std::map<String, float> s_aoi_blocked_data;


	bool newdataflag = false;

	//とりあえずブロック,避難所は無条件で情報更新する
	for(auto a : r_blockednodeid){
		for(auto b : m_blockednodeid){
			if(a == b){
				s_blockednodeid.erase(b);
				break;
			}else{
				s_blockednodeid.insert(b);
			}
		}
		mydata->SetBlockedNodeID(a); //ブロック情報更新
		// mydata->UpdateGraphInfoReceivedPacket(a);//グラフを更新することで経路を更新？
	}

	for(auto a : r_fullexitnodeid){
		for(auto b : m_fullexitnodeid){
			if(a == b){
				s_fullexitnodeid.erase(b);
				break;
			}else{
				s_fullexitnodeid.insert(b);
			}
		}
		mydata->SetFullexitNodeID(a);
	}

	//////////////////////////////////避難所データ比較//////////////////////////////////////////

	for(auto newexitdata : r_exit_aoi_data){
		uint32_t recv_data_aoi;
		uint32_t old_data_aoi;
		uint32_t recv_data_can_exitnum;
		uint32_t old_data_can_exitnum;
		String shelterID = newexitdata.first;
		recv_data_aoi = newexitdata.second.second; //受信したデータの生成時間を取得
		old_data_aoi = m_aoi_data.at(shelterID).second; //すでに所持しているデータの時間を取得

		recv_data_can_exitnum = newexitdata.second.first;
		old_data_can_exitnum = m_aoi_data.at(shelterID).first;

		if(old_data_aoi > recv_data_aoi){ //データの生成時間で比較
			//相手から古い情報を受け取った場合は、自分から新しい情報を返す。
			//送るデータを作成
			// std::cout<<"old_data_aoi = "<<old_data_aoi<<", recv_data_aoi = "<<recv_data_aoi<<std::endl; //debag
			s_exit_aoi_data.insert(std::make_pair(shelterID, std::make_pair(old_data_can_exitnum, old_data_aoi)));
			sendflag = true;
		}
		if(old_data_aoi == recv_data_aoi){
			//同じAoIの情報
		}
		if(old_data_aoi < recv_data_aoi){
			m_aoi_data.erase(shelterID);
			m_aoi_data.insert(std::make_pair(shelterID, std::make_pair(recv_data_can_exitnum, recv_data_aoi)));
			mydata->SetAoiExitinfo(m_aoi_data); //更新されたデータをセット
			newdataflag = true;

		}
	}

	///////////////////////////////////////通行止めAoI比較///////////////////////////////////////////////////

	for(auto a : m_aoi_blocked_data){
		String blockID = a.first;
		try{
			r_aoi_blocked_data.at(blockID); //受信データに自分が持っているデータがあるかないか確認の処理
		}
		catch(std::out_of_range& oor){
			// std::cout<<"Hello送信元が所持していないデータがあります"<<std::endl; //debag
			double aoi = (Simulator::Now().GetDouble()/1000000000) - a.second;

			if(eraseinfoflag == true){
				if(aoi < MyBuilding::GetEraseAoI()){ //aoiが設定したしきい値より小さければ実行
					s_aoi_blocked_data.erase(blockID);
					// std::cout<<"a.second = "<<a.second<<std::endl; //debag
					s_aoi_blocked_data.insert(std::make_pair(blockID, a.second)); //送り返すデータに挿入
					sendflag = true;
				}else{
					// std::cout<<"しきい値より大きいのでデータを返しません"<<std::endl;
				}
			}else{
				s_aoi_blocked_data.erase(blockID);
				// std::cout<<"a.second = "<<a.second<<std::endl; //debag
				s_aoi_blocked_data.insert(std::make_pair(blockID, a.second)); //送り返すデータに挿入
				sendflag = true;
			}
		}
	}

	for(auto b : r_aoi_blocked_data){
		float recv_data_aoi;
		float old_data_aoi;
		String blockID = b.first;
		recv_data_aoi = b.second; //受信したデータの生成時間を取得

		try{
			old_data_aoi = m_aoi_blocked_data.at(blockID); //すでに所持しているデータの時間を取得
		}
		catch(std::out_of_range& oor){
			//新規データ
			// std::cout<<"新しい通行止め情報です"<<std::endl; //debag
			// std::cout<<"blockID = "<<blockID<<std::endl;//debag
			double aoi = (Simulator::Now().GetDouble()/1000000000) - recv_data_aoi;
			// std::cout<<recv_data_aoi<<std::endl;//debag
			// std::cout<<"AoI = "<<aoi<<std::endl;//debag
			if(eraseinfoflag == true){
				if(aoi < MyBuilding::GetEraseAoI()){ //しきい値より小さいならば新たな情報として設定する
					// std::cout<<"通行止め箇所として設定しました"<<std::endl; //debag
					mydata->SetBlockedNodeID_and_time(blockID, recv_data_aoi); //持っていないデータを追加
					mydata->UpdateGraphInfoReceivedPacket(blockID);
					newdataflag = true;
				}else{
					// std::cout<<"しきい値よりAoIが大きいので通行止め箇所として設定しませんでした"<<std::endl;
				}
			}else{
					// std::cout<<"通行止め箇所として設定しました"<<std::endl; //debag
					mydata->SetBlockedNodeID_and_time(blockID, recv_data_aoi); //持っていないデータを追加
					mydata->UpdateGraphInfoReceivedPacket(blockID);
					newdataflag = true;
			}
		}

		if(old_data_aoi > recv_data_aoi){ //データの生成時間で比較
			//送信元が古いデータを所持している場合
			s_aoi_blocked_data.erase(blockID);
			s_aoi_blocked_data.insert(std::make_pair(blockID, old_data_aoi));
			sendflag = true;
		}
		if(old_data_aoi == recv_data_aoi){
			//同じAoIの情報
		}
		if(old_data_aoi < recv_data_aoi){
			//情報鮮度が高いデータ
			double aoi = (Simulator::Now().GetDouble()/1000000000) - recv_data_aoi;
			if(eraseinfoflag == true){
				if(aoi < MyBuilding::GetEraseAoI()){ //しきい値より小さいならばブロックとして設定する
					// std::cout<<"Helloから通行止め箇所として設定しました"<<std::endl; //debag
					m_aoi_blocked_data.erase(blockID);
					m_aoi_blocked_data.insert(std::make_pair(blockID, recv_data_aoi));
					mydata->SetBlockedNodeID_and_time2(m_aoi_blocked_data); //更新されたデータをセット
					mydata->UpdateGraphInfoReceivedPacket(blockID);//グラフを更新することで経路を更新？
				}else{
					// std::cout<<"Helloから通行止め箇所として設定しませんでした"<<std::endl; //debag
				}
			}else{
				m_aoi_blocked_data.erase(blockID);
				m_aoi_blocked_data.insert(std::make_pair(blockID, recv_data_aoi));
				mydata->SetBlockedNodeID_and_time2(m_aoi_blocked_data); //更新されたデータをセット
				mydata->UpdateGraphInfoReceivedPacket(blockID);//グラフを更新することで経路を更新？
			}
			newdataflag = true;
		}

	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(GetUseQuickHelloflag() == true && newdataflag == true){
		SendHello(); //Quicksendの場合は新しい情報を受け取るとすぐHelloを送信する
	}

	s_blockednodeid.erase(kara);
	s_fullexitnodeid.erase(kara);
	// if(s_blockednodeid.size() != 0 || s_fullexitnodeid.size() != 0){
	// 	// std::cout<<"実行されてる？"<<std::endl; //debag
	// 	// for(auto a : s_blockednodeid){
	// 	// 	std::cout<<a<<std::endl;
	// 	// }
	// 	// for(auto a : s_fullexitnodeid){
	// 	// 	std::cout<<a<<std::endl;
	// 	// }
	// 	sendflag = true;
	// }

	if(sendflag == true){
		UserData senddata(userID, dst, s_blockednodeid, s_fullexitnodeid, s_exit_aoi_data, s_aoi_blocked_data);
		SendUserDataAsUnicast(senddata, dst);
	}

}

UserData RoutingProtocol::MakeSendData(Ipv4Address dst){
	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();
	uint32_t userId = user->GetUserId() + 1;
	std::set<String> b_set = user->GetBlockedNodeIDSet();
	std::set<String> fullexit_set = user->GetFullexitNodeIDSet();
	std::map <String, std::pair<uint32_t, uint32_t>> aoi_exitinfo = user->GetAoiExitinfo();
	std::map <String, float> aoi_blockinfo = user->GetBlockedNodeID_and_time();
	String kara = "";
	b_set.erase(kara);
	fullexit_set.erase(kara);
	UserData data(userId, dst, b_set, fullexit_set, aoi_exitinfo, aoi_blockinfo);
	return data;
}


void RoutingProtocol::SendHello(){
	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();
	if(user->GetStatus() == EXIT){
		return;
	}
	////////////////////////////////////データ作成//////		//user->SetBlockedNodeIDBuf(blockedid); //ブロックノードIDをnode/12345というString型でバッファする
		// std::cout << blockedid << ", ";//////////////////////////////
	Ipv4Address dst = Ipv4Address ("255.255.255.255");
	uint32_t userId = user->GetUserId() + 1;
	std::set<String> b_set = user->GetBlockedNodeIDSet();
	std::set<String> fullexit_set = user->GetFullexitNodeIDSet();
	std::map <String, std::pair<uint32_t, uint32_t>> aoi_exitinfo = user->GetAoiExitinfo();
	std::map <String, float> aoi_blockinfo = user->GetBlockedNodeID_and_time();
	String kara = "";
	b_set.erase(kara);
	fullexit_set.erase(kara);
	UserData hello(userId, dst, b_set, fullexit_set, aoi_exitinfo, aoi_blockinfo);
	//////////////////////////////////////////////////////////////////////////////////

	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
	{
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;

		m_IdCache.IsDuplicate (iface.GetLocal (), m_broadcast_id);

		Ptr<Packet> packet = Create<Packet> ();
		packet->AddHeader (hello);
		TypeHeader tHeader (TYPE_HELLO);
		packet->AddHeader (tHeader);

		Ipv4Address destination;
		if (iface.GetMask () == Ipv4Mask::GetOnes ())
		{
			destination = Ipv4Address ("255.255.255.255");
		}
		else
		{
			destination = iface.GetBroadcast ();
		}

		MyBuilding::PlusSendHelloNum();
		MyBuilding::PlusSendHelloBytes(packet->GetSize());
		helloid = Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
				&RoutingProtocol::SendTo, this, socket, packet, destination, HELLO_PACKET);

	}

	CheckAoI();

	if(GetUseQuickHelloflag() == true && user->GetQuickSendflag() == true /*&& user->GetQuickHelloTimes() < MyBuilding::GetQuickHelloTimes()*/){
		// HelloTimer.Cancel();
		ActivateHelloTimer(MyBuilding::GetQuickHelloInterval());
		user->SetAlreadyQuickSendFlag(true);
		user->AddQuickHelloTimes(); //QuickHelloをした回数を1増やす
		return;
	}else{
		//QuickSendを実行しないときの処理
		// HelloTimer.Cancel();
		ActivateHelloTimer(MyBuilding::GetHelloInterval());
		user->SetAlreadyQuickSendFlag(false);
		user->InitQuickHelloTimes(); //QuickHelloの回数を初期化
		return;
	}
}


void RoutingProtocol::SendShelterInfoAsBroadcast(ShelterInfo data){
	// std::cout<<"ShelterDataAsBroadcast!!"<<std::endl;
	data.SetId(m_id);
	data.SetBroadcastId(m_broadcast_id);
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
	{
		//std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();

		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;

		m_IdCache.IsDuplicate (iface.GetLocal (), m_broadcast_id);

		Ptr<Packet> packet = Create<Packet> ();
		packet->AddHeader (data);
		TypeHeader tHeader (TYPE_SHELTERDATA);
		packet->AddHeader (tHeader);

		Ipv4Address destination;
		if (iface.GetMask () == Ipv4Mask::GetOnes ())
		{
			destination = Ipv4Address ("255.255.255.255");
		}
		else
		{
			destination = iface.GetBroadcast ();
		}
		
		// std::cout<<destination<<std::endl;
		Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
				&RoutingProtocol::SendTo, this, socket, packet, destination, SHELTERDATA_PACKET);
		//OutputText_RW(Simulator::Now(),HELLO_PACKET,destination);
	}
	 HandleShelterDataTimer(MyBuilding::GetShelterDataInterval());
}


void RoutingProtocol::SendUserDataAsUnicast(UserData data, Ipv4Address destination){
	RoutingTableEntry toDst;
	RoutingTable rtable;
	Ptr<MyUser> node = m_ipv4->GetObject<MyUser>();
	uint32_t id = node->GetUserId() + 1;
	SetRoute(destination,destination);
	if(!m_routingTable.LookupRoute(destination,toDst)){
		std::cout<<"ルートないぜ！！！！！！！！！"<<std::endl;
		return;
	}

	Ptr<Socket> socket=FindSocketWithInterfaceAddress (toDst.GetInterface());
	Ptr<Packet> packet = Create<Packet> ();
	packet->AddHeader (data);
	TypeHeader tHeader (TYPE_USERDATA);
	packet->AddHeader (tHeader);
	node->SetAlreadySentAddress(destination); //送信済みのアドレスとしてセットする
	std::ofstream writing_file;

	String quickhello="QuickHello_ON";
	if(simple::RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}

	std::string filename = "Log/obayashi/ON/" + quickhello + "/" + std::to_string(MyBuilding::GetNumUsers())+"/testLog/Send_time.txt";
	writing_file.open(filename, std::ios::app);
	writing_file << id+1 <<", "<< destination <<", "<<Simulator::Now().GetDouble()/1000000000<<std::endl;
	dataevent = Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 100))),
			&RoutingProtocol::SendTo, this, socket, packet, destination, USERDATA_PACKET);

	

}


void RoutingProtocol::RecvHello(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender){
	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();
	UserData hello;
	p->RemoveHeader (hello);
	//uint32_t sender_userID = hello.GetUserId();
	MyBuilding::PlusRecvHelloNum();
	MyBuilding::PlusRecvHelloBytes(p->GetSize());

	CheckHelloData(hello,sender); //データと送信元のアドレスを返す
	// HandleUserData(sender,sender_userID);
}



void RoutingProtocol::RecvShelterDataAsBroadcast(Ptr<Packet> p){
	// std::cout<<"RecvShelterDataAsBroadcast!!"<<std::endl;
	
	ShelterInfo data;
	p->RemoveHeader (data);
	uint32_t shelterID = data.GetShelterId();
	String s_shelterID = "node/"+std::to_string(shelterID);
	uint32_t can_exit_num = data.GetExitNum();
	uint32_t Time = data.GetTime();
	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();
	uint32_t id = user->GetUserId() + 1;
	std::map <String, std::pair<uint32_t, uint32_t>> Aoi_exitinfo;

	if (m_IdCache.IsDuplicate (data.GetId (), id))
	{
		std::cout<<"避難所情報重複"<<std::endl;
		return;
	}

	///////////////////ここに時間をもとに情報を取捨選択する処理を追加///////////////////

	std::map <String, std::pair<uint32_t, uint32_t>> old_data = user->GetAoiExitinfo(); //避難所ID、収容可能人数、時間

	std::pair<uint32_t, uint32_t> canexitnum_and_time = old_data.at(s_shelterID);
	std::map <String, std::pair<uint32_t, uint32_t>> new_Aoi_exitinfo;
	if(canexitnum_and_time.second < Time){
		// std::cout<<"所持データの生成された時間："<<canexitnum_and_time.second<<", 受信したデータの生成された時間："<<Time<<std::endl;
		std::pair<uint32_t, uint32_t> new_canexitnum_and_time = std::make_pair(can_exit_num, Time);
		// new_Aoi_exitinfo.insert(std::make_pair(s_shelterID, new_canexitnum_time));
		// user->EraseAoiExitinfo(s_shelterID);
		old_data.erase(s_shelterID);
		old_data.insert(std::make_pair(s_shelterID,new_canexitnum_and_time ));
		user->SetAoiExitinfo(old_data);
		if(GetUseQuickHelloflag() == true){
			// CheckAoI();
			SendHello();
		}else{
			// SendHello();
			
		}

	}else{
		
		std::cout<<"UserID = "<< user->GetUserId() + 1 <<", 避難所から古いデータを受信しました。"<<std::endl;
	}

	////////////////////////////////////////////////////////////////////////////////////
	
	// std::cout<<"避難所ID = "<<shelterID<<", "<<"残りの避難可能人数 = "<<can_exit_num<<std::endl;
}


void RoutingProtocol::RecvUserDataAsUnicast(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender){

	//std::cout << "送信元：" << sender << "受信者"<< receiver << std::endl;
	OutputText_Recive_Data(sender,receiver);
	 //Helloを開始していないときは開始する
	NS_LOG_FUNCTION (this << " src " << sender);

	UserData data;
	p->RemoveHeader (data);
	bool newdataflag = false;

	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();

	std::set<String> blockednode = data.GetBlockedNodeIDSet();
	std::set<String> fullexitnode_set = data.GetFullExitNodeIDSet();
	std::map<String, std::pair<uint32_t, uint32_t>> AoIexitinfo = data.GetAoIExitinfo(); //受け取った避難所情報
	std::map<String, std::pair<uint32_t, uint32_t>> old_AoIexitinfo = user->GetAoiExitinfo();//すでに所持している避難所情報
	std::map<String, float> AoIblockedinfo = data.GetAoIBlockedInfo();
	std::map<String, float> old_AoIblockedinfo = user->GetBlockedNodeID_and_time();

	for(auto a : fullexitnode_set){
		// std::cout<<a<<std::endl;
		user->SetFullexitNodeID(a);
	}
///////////////////////受信データLOG出力////////////////////////////////////////////////
	std::ofstream writing_file;
	std::string filename = "Log/obayashi/ON/"+std::to_string(MyBuilding::GetNumUsers())+"/testLog/Recv_data.txt";
	writing_file.open(filename, std::ios::app);
	writing_file<<std::endl<<"time: "<<Simulator::Now().GetDouble()/1000000000<<std::endl;
	writing_file << sender << " -> "<< receiver << std::endl;
	writing_file << "data: "<< "blockedid = ";
	// uint32_t UserID = user->GetUserId() + 1;
	// std::cout << UserID <<"：";
	for(auto blockedid : blockednode){	
		writing_file << blockedid <<", " ;
		user->SetBlockedNodeID(blockedid); //ブロック情報更新
		// user->UpdateGraphInfoReceivedPacket(blockedid);//グラフを更新することで経路を更新？
	}
	writing_file << std::endl;
////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////AoI比較////////////////////////////////////////////////////////

	// bool find_new_data_flag = false;

	for(auto newdata : AoIexitinfo){
		uint32_t recv_data_aoi;
		uint32_t old_data_aoi;
		uint32_t recv_data_can_exitnum;
		String shelterID = newdata.first;

		recv_data_aoi = newdata.second.second; //受信したデータの生成時間を取得
		// std::cout<<shelterID<<std::endl;
		// std::cout<<user->GetUserId()<<std::endl;
		old_data_aoi = old_AoIexitinfo.at(shelterID).second; //すでに所持しているデータの時間を取得

		recv_data_can_exitnum = newdata.second.first;

		if(old_data_aoi > recv_data_aoi){ //データの生成時間で比較
			// std::cout<<"古い情報は送らないようにしているからここが実行されるわけがない"<<std::endl;
			// std::cout<<"old_data_aoi = "<<old_data_aoi<<", recv_data_aoi = "<<recv_data_aoi<<std::endl; //debag
		}
		if(old_data_aoi == recv_data_aoi){
			// std::cout<<"タイミングのズレで同じ情報が送られてきている？(====)"<<std::endl;
		}
		if(old_data_aoi < recv_data_aoi){
			// std::cout<<"新しいデータが返って来ました"<<std::endl; //debag
			old_AoIexitinfo.erase(shelterID);
			old_AoIexitinfo.insert(std::make_pair(shelterID, std::make_pair(recv_data_can_exitnum, recv_data_aoi)));
			user->SetAoiExitinfo(old_AoIexitinfo); //更新されたデータをセット
			newdataflag = true;
		}
	}

	for(auto newdata : AoIblockedinfo){
		float recv_data_aoi;
		float old_data_aoi;
		String shelterID = newdata.first;

		recv_data_aoi = newdata.second; //受信したデータの生成時間を取得

		try{
			old_data_aoi = old_AoIblockedinfo.at(shelterID); //すでに所持しているデータの時間を取得
		}catch(std::out_of_range& oor){
			// std::cout<<"持っていないで通行止め情報が送られてきました"<<std::endl; //debag
			double aoi = (Simulator::Now().GetDouble()/1000000000) - recv_data_aoi;
			if(eraseinfoflag == true){
				if(aoi < MyBuilding::GetEraseAoI()){
					newdataflag = true;
					old_AoIblockedinfo.erase(shelterID);
					old_AoIblockedinfo.insert(std::make_pair(shelterID, recv_data_aoi));
					user->SetBlockedNodeID_and_time2(old_AoIblockedinfo); //更新されたデータをセット
					// old_AoIblockedinfo = user->GetBlockedNodeID_and_time();
				}else{
					// std::cout<<"自分は所持してないけどしきい値より大きいので破棄します"<<std::endl; //debag
				}
			}else{
				newdataflag = true;
				old_AoIblockedinfo.erase(shelterID);
				old_AoIblockedinfo.insert(std::make_pair(shelterID, recv_data_aoi));
				user->SetBlockedNodeID_and_time2(old_AoIblockedinfo); //更新されたデータをセット
			}
		}

		if(old_data_aoi > recv_data_aoi){ //データの生成時間で比較
			// std::cout<<"タイミングのズレで古いデータを受信したので破棄します"<<std::endl; //debag
			// std::cout<<"old_data_aoi = "<<old_data_aoi<<", recv_data_aoi = "<<recv_data_aoi<<std::endl; //debag
		}
		if(old_data_aoi == recv_data_aoi){
			// std::cout<<"タイミングのズレで同じ情報が送られてきている？(====)"<<std::endl; //debag
		}
		if(old_data_aoi < recv_data_aoi){
			double aoi = (Simulator::Now().GetDouble()/1000000000) - recv_data_aoi;
			if(eraseinfoflag == true){
				if(aoi < MyBuilding::GetEraseAoI()){
					// std::cout<<"新しい通行止め箇所データが返って来ました"<<std::endl; //debag
					old_AoIblockedinfo.erase(shelterID);
					old_AoIblockedinfo.insert(std::make_pair(shelterID, recv_data_aoi));
					user->SetBlockedNodeID_and_time2(old_AoIblockedinfo); //更新されたデータをセット
					user->UpdateGraphInfoReceivedPacket(shelterID);
					newdataflag = true;
				}else{
					// std::cout<<MyBuilding::GetEraseAoI()<<"以上なので破棄します"<<std::endl; //debag
				}
			}else{
				old_AoIblockedinfo.erase(shelterID);
				old_AoIblockedinfo.insert(std::make_pair(shelterID, recv_data_aoi));
				user->SetBlockedNodeID_and_time2(old_AoIblockedinfo); //更新されたデータをセット
				user->UpdateGraphInfoReceivedPacket(shelterID);
				newdataflag = true;
			}
		}
	}

	if(GetUseQuickHelloflag() == true && newdataflag == true){
		SendHello(); //Quicksendの場合は新しい情報を受け取るとすぐHelloを送信する
	}

	std::ofstream writing_file2;
	std::string filename2 = "Log/obayashi/ON/"+std::to_string(MyBuilding::GetNumUsers())+"/testLog/Recv_time.txt";
	writing_file2.open(filename2, std::ios::app);
	writing_file2 << sender << " ,"<< receiver <<", "<<Simulator::Now().GetDouble()/1000000000<<std::endl;
	std::set<String> blockednode2 = user->GetBlockedNodeIDSet();
	String kara;
	blockednode2.erase(kara);
	now = blockednode2.size();

}


void RoutingProtocol::CheckAoI(){
	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();
	std::map<String, std::pair<uint32_t, uint32_t>> AoIexitinfo = user->GetAoiExitinfo();//所持している避難所情報
	std::map<String, float> AoIblockinfo = user->GetBlockedNodeID_and_time();

	for(auto a : AoIexitinfo){
		uint32_t AoI = (Simulator::Now().GetDouble()/1000000000) - a.second.second;
		if(AoI <= MyBuilding::GetAoIthreshold()){
			user->SetQuickSendflag(true);
			return;
		}
	}

	for(auto a : AoIblockinfo){
		double AoI = (Simulator::Now().GetDouble()/1000000000) - a.second;
		if(AoI <= MyBuilding::GetAoIthreshold()){
			user->SetQuickSendflag(true);
			return;
		}
	}

	user->SetQuickSendflag(false);

}

void RoutingProtocol::ActivateCheckAoITimer(double delay){
	CheckAoITimer.SetFunction(&RoutingProtocol::CheckAoI,this);
	CheckAoITimer.Remove();
	CheckAoITimer.Schedule(Seconds(delay));
}

// void RoutingProtocol::ActivateCheckDataTimer(double delay){
// 	CheckDataTimer.SetFunction(&RoutingProtocol::CheckData,this);
// 	CheckDataTimer.Remove();
// 	CheckDataTimer.Schedule(Seconds(delay));
// }

// int aa = 0;
// void RoutingProtocol::AddUserCheckDataTimer(std::vector<uint32_t> rand){
// 	AddCheckDataTimer.SetFunction(&RoutingProtocol::CheckData,this);
// 	AddCheckDataTimer.Remove();
// 	AddCheckDataTimer.Schedule(Seconds(rand[aa]));
// 	aa++;
// }

void RoutingProtocol::ActivateHelloTimer(double delay){
	HelloTimer.SetFunction(&RoutingProtocol::SendHello,this);
	HelloTimer.Remove();
	HelloTimer.Schedule(Seconds(delay));
}


void RoutingProtocol::HandleShelterDataTimer(double delay){
	shelterdataTimer.SetFunction(&RoutingProtocol::HandleShelterData,this);
	shelterdataTimer.Remove();
	shelterdataTimer.Schedule(Seconds(delay));
}

void RoutingProtocol::CheckMovingUserNumTimer(double delay){
	MovingUserNumTimer.SetFunction(&RoutingProtocol::CheckMovingUserNum,this);
	MovingUserNumTimer.Remove();
	MovingUserNumTimer.Schedule(Seconds(delay));
}

int abcd = 0;
void RoutingProtocol::CheckMovingAddUserNumTimer(std::vector<uint32_t> vec){
	MovingUserNumTimer.SetFunction(&RoutingProtocol::CheckMovingUserNum,this);
	MovingUserNumTimer.Remove();
	MovingUserNumTimer.Schedule(Seconds((vec[abcd])));
	abcd++;
}
 
int hoge = 0;
void RoutingProtocol::AddUserCheckAoITimer(std::vector<uint32_t> vec){
	AddCheckAoITimer.SetFunction(&RoutingProtocol::CheckAoI,this);
	AddCheckAoITimer.Remove();
	AddCheckAoITimer.Schedule(Seconds((vec[hoge])));
	hoge++;
}

int aaaa = 0;
void RoutingProtocol::SetSocketTimer(std::vector<uint32_t> vec){
	SocketTimer.SetFunction(&RoutingProtocol::SetSocket,this);
	SocketTimer.Remove();
	SocketTimer.Schedule(Seconds((vec[aaaa])));
	aaaa++;
}

int aaa = 0;
void RoutingProtocol::AdduserHelloTimer(double delay, std::vector<uint32_t> vec){
	HelloTimer.SetFunction(&RoutingProtocol::SendHello,this);
	HelloTimer.Remove();
	HelloTimer.Schedule(Seconds((vec[aaa])+delay));
	aaa++;
}

 
// void RoutingProtocol::HandleUserDataTimer(double delay)
// {
// 	fromAPtoUserHandleDataTimer.SetFunction(&RoutingProtocol::HandleUserData, this);
// 	fromAPtoUserHandleDataTimer.Remove();
// 	fromAPtoUserHandleDataTimer.Schedule(Seconds(delay));
// }

void RoutingProtocol::AoiTimer(double delay)
{
	AoITimer.SetFunction(&RoutingProtocol::WriteAoiLog, this);
	AoITimer.Remove();
	AoITimer.Schedule(Seconds(delay));
}

void RoutingProtocol::BlockAoiTimer(double delay)
{
	BlockAoITimer.SetFunction(&RoutingProtocol::WriteBlockAoiLog, this);
	BlockAoITimer.Remove();
	BlockAoITimer.Schedule(Seconds(delay));
}

void RoutingProtocol::HandlClusterViewTimer ()
{
	Timer timer(Timer::CANCEL_ON_DESTROY);
	ClusterViewTimer = timer;
	ClusterViewTimer.SetFunction(&RoutingProtocol::WriteLog, this);
	ClusterViewTimer.Remove();
	ClusterViewTimer.Schedule(Seconds(LOG_TIMER));
}

void RoutingProtocol::DatasendTimerCancel(){
//	std::cout<<"DatasendTimerCancel"<<std::endl;
	fromAPtoUserHandleDataTimer.Cancel();
	fromAPtoAPHandleDataTimer.Cancel();
	fromUsertoAPHandleDataTimer.Cancel();
	// now=EXIT;
}

// void RoutingProtocol::HelloCancel(double delay){
// //	std::cout<<"DatasendTimerCancel"<<std::endl;
// 	HelloCancelTimer.SetFunction(&RoutingProtocol::WriteAoiLog, this);
// 	HelloCancelTimer.Remove();
// 	HelloCancelTimer.Schedule(Seconds(delay));
// 	// now=EXIT;
// }

void RoutingProtocol::WriteLog ()
{
	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();
	uint32_t now2 = user->GetStatus();
	switch (now2){
	case EXIT:
		// std::cout<<"EXIT2!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
		OutputText_NI(Simulator::Now(), m_ipv4, "EXIT",m_parent,c_id,m_hop);
		HelloTimer.Remove();
		return;
	default:
		break;
	}

	switch (now) {
	case ON:
		OutputText_NI(Simulator::Now(), m_ipv4, "ON",m_parent,c_id,m_hop);
		break;
	case EXIT:
		// std::cout<<"EXIT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
		OutputText_NI(Simulator::Now(), m_ipv4, "EXIT",m_parent,c_id,m_hop);
		HelloTimer.Remove();
		break;
	default:
		// std::cout<<"now = "<<now<<std::endl;
		OutputText_NI(Simulator::Now(), m_ipv4, std::to_string(now),m_parent,c_id,m_hop);
		break;
	}
}

void RoutingProtocol::OutputText_NI (Time now_t, Ptr<Ipv4> m_ipv4, std::string color,Ipv4Address parent,Ipv4Address clusterId,uint8_t hop)
{
	// set parameters
	int64_t time = now_t.GetInteger();
	int64_t roughTime = time / 1000000000;
	int64_t splitSecond = time % 1000000000;
	Ptr<Node> node = m_ipv4->GetObject<Node>();
	Ptr<MyUser> user = m_ipv4->GetObject<MyUser>();
	Ptr<MobilityModel> obj = node->GetObject<MobilityModel>();
	double x = obj->GetPosition().x;
	double y = obj->GetPosition().y;
	int nodeid = node->GetId()+1;

	uint8_t cml_size = 0;
	std::string status = "MOVE";

	if(!m_cml.empty()){
		cml_size = m_cml.size();

	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// std::cout<<"ここでエラー"<<std::endl;
	// bool e_flag=MyUser().GetExitflag();
	bool e_flag=false;
	try{
		std::map<String, uint32_t> exits = simple::MyBuilding::GetExitNodes();

		String id = JSON_Data::GetIDfromCoordinate(user->GetPassedroute().back());
		decltype(exits)::iterator itr = exits.find(id);
		if(itr != exits.end()){
			e_flag=true;
//			std::cout<<"e_flag : true"<<std::endl;
		}
	}
	catch(std::out_of_range& oor){

	}

	if((user->GetSectionId().y==4&&(user->GetSectionId().x==0||user->GetSectionId().x==4))||e_flag) {
		// status="EXIT";
		// std::string status_buf = "EXIT";
		//次のノードが避難所の時に実行される処理
		// std::cout<<"EXIT"<<std::endl;
		// HelloCancelTimer(30.0); //30秒後にhelloを止める
		// now=EXIT;
		// HelloTimer.Cancel();
//		fromAPtoUserHandleDataTimer.Cancel();
//		fromAPtoAPHandleDataTimer.Cancel();
//		fromUsertoAPHandleDataTimer.Cancel();
	}

	// if(status == "EXIT"){
	// 	std::cout<<"OK!"<<std::endl;
	// 	if(!MyUser().GetExitflag()){
	// 		std::cout<<"OK2!"<<std::endl;
	// 		HelloTimer.Cancel();
	// 		SendHello();
	// 	}
	// }

	if(user->GetCloseflag()) {
		DatasendTimerCancel();
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	if(/*status=="MOVE"&&*/(color=="CN"||color=="BCN")) {
		cml_size_num = cml_size_num + cml_size;
		CN_num++;
	}

	// set directory
	char filename[100];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";
	if(GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}
	String quickhello="QuickHello_ON";
	if(GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}

	String eraseblock = "EraseBlock_ON";
	if(GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "Log/obayashi/" + mode + "/"  + quickhello +"/" + eraseblock + "/"  + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterViewer/ClusterViewer_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';


	// output to file
	std::ofstream fout;
	fout.open(filename,std::ios::app);
	fout << "NI, "										// Node Information
		 << roughTime << "." << splitSecond      << ", "	// time [s]
		 << nodeid								 << ", "	// node id
		 << x 									 << ", "	// position x
		 << y 									 << ", "	// position y
		 << clusterId.Get()-167772160		     << ", "	// cluster Id
		 << color 								 << ", "	// node color
		 << parent.Get()-167772160				 << ", "	// cluster Id
		 << (int)hop						     << ", "	// hop
		 << status
		 << ", "								 << (int)cml_size
		 <<std::endl;
	fout.close();

	HandlClusterViewTimer();
}

void RoutingProtocol::OutputText_Recive_Data(Ipv4Address sender, Ipv4Address reciver){
	char filename[100];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	if(simple::RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}

	String quickhello="QuickHello_ON";
	if(GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}

	String eraseblock = "EraseBlock_ON";
	if(GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "Log/obayashi/" + mode + "/"  + quickhello + "/" +eraseblock+ "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Recive/recive_data" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::app);
	fout << sender << ", " << reciver << ", " << Simulator::Now().GetDouble()/1000000000 << std::endl;
	fout.close();
}


void RoutingProtocol::WriteForFile_RW (std::string packetName, Ipv4Address dst, double time, uint64_t splitSecond)
{
	char filename[100];
	std::ostringstream oss_delay, oss_run;
	oss_delay << MyBuilding::GetHelloInterval();
	oss_run << RngSeedManager::GetRun();
	String mode="ON";

	if(GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}
	String quickhello="QuickHello_ON";
	if(GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}

	String eraseblock = "EraseBlock_ON";
	if(GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterViewer/ClusterViewer_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//	std::string str = "Log/ClusterViewer/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename,std::ios::app);
	fout<<"RW, "
		<<time<<", "		// time
		<<nodeId+1<<", "	// node id
		<<"100, ";			// communication radius
	if(dst==Ipv4Address("10.255.255.255")){
		fout<<"-1, ";		// destination of broadcast
	}else{
		fout<<dst.Get()-167772160<<", ";
	}
	fout<<packetName<<std::endl;	// packet type
	fout.close();
}

void RoutingProtocol::OutputText_RW (Time now, int pktType, Ipv4Address dst)
{
	int64_t time = now.GetInteger();
	double ttt = now.GetDouble() / 1000000000;
	//int64_t roughTime=time/1000000000;
	int64_t splitSecond = time % 1000000000;

	switch(pktType) {
	case USERDATA_PACKET:
		frequency++;
		WriteForFile_RW("USERDATA", dst, ttt, splitSecond);
		break;
	case HELLO_PACKET:
		WriteForFile_RW("HELLO", dst, ttt, splitSecond);
		break;
	case SHELTERDATA_PACKET:
		WriteForFile_RW("SHELTERDATA", dst, ttt, splitSecond);
		break;
	default:
		break;
	}
}

}
}


