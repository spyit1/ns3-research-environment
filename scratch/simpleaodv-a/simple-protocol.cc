/*
* simple-protocol.cc
*
*  Created on: 2016/03/03
*      Author: terami
*/

#include "simple-protocol.h"
//#include "simple-application.h"
//#include "simple-network.h"

#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include "ns3/mobility-module.h"

#include <algorithm>
#include <limits>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <stdio.h>



NS_LOG_COMPONENT_DEFINE ("ClusterProtocol");

namespace ns3
{

namespace simple
{

NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

//UDP port for MYRTP control traffic
const uint32_t simple::RoutingProtocol::MY_PORT=5555;

SenderInfoList m_sil;
double diffusion=0.0;
double current_data_send=0.0;

unsigned long int controlPacketOnWifi=0;

std::unordered_map<DataKey,double,DataKey::Hash> dissemination_cache;
std::unordered_map< DataKey,std::unordered_map<int,double>,DataKey::Hash > dissemination_time;

long double dissemination_wifi=0.0;

std::map<int,int> hop_info;

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

//--------------------------------------------------------------
TypeId RoutingProtocol::GetTypeId(void)
{
	static TypeId tid = TypeId ("ns3::simple::RoutingProtocol")
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
													 MakePointerChecker<UniformRandomVariable> ());
	return tid;
}

RoutingProtocol::RoutingProtocol():
								ActiveRouteTimeout(Seconds(3)),
								m_IdCache(Seconds(1)),
								is_server(false),
								m_broadcast_id(0),
								my_id(0),
								m_nodeState(0),
								m_dataseqno{0},
								m_ackseqno{0},
								m_dataIdCache(Seconds(1)),
								m_rreqIdCache(Seconds(1)),
								m_rreqId(0),
								recv_data_times(0),
								send_data_times(0)
{
	NS_LOG_FUNCTION(this);
}

RoutingProtocol::~RoutingProtocol()
{
	NS_LOG_FUNCTION(this);

}

void RoutingProtocol::DoDispose()
{
	m_ipv4=0;

	//終了時のオブジェクトの廃棄などの後処理

	Ipv4RoutingProtocol::DoDispose();
}

void RoutingProtocol::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
	//ルーティングテーブルのエントリーを出力する処理を行う
	//実際にはルーティングテーブルクラスのPrintメソッドにより処理する

	*stream->GetStream() << "Node: " << m_ipv4->GetObject<Node>()->GetId()
					     << " Time: " << Simulator::Now().GetSeconds() << "s ";

	*stream->GetStream () << "Node: " << m_ipv4->GetObject<Node> ()->GetId ()
	                      << "; Time: " //<< Now ().As (unit)
						  << ", Local time: " //<< GetObject<Node> ()->GetLocalTime ().As (unit)
						  << ", AODV Routing table" << std::endl;

	m_routingTablemagr.Print(stream);
	*stream->GetStream () << std::endl;
}

int64_t RoutingProtocol::AssignStreams(int64_t stream)
{
	NS_LOG_FUNCTION(this << stream);
	//乱数系列の割り当て処理
	m_uniformRandomVariable->SetStream(stream);
	return 1;
}

//transport layer
Ptr<Ipv4Route> RoutingProtocol::RouteOutput(Ptr<Packet> p,const Ipv4Header &header,Ptr<NetDevice> oif,Socket::SocketErrno &sockerr)
{
	NS_LOG_FUNCTION(this << header << " " << (oif ? oif->GetIfIndex():0));

	//パケットの転送経路を決定するための処理を行う
	sockerr = Socket::ERROR_NOTERROR;
	Ptr<Ipv4Route> route;
	Ipv4Address dst = header.GetDestination ();
	RoutingTableEntry rt;
	if (m_routingTablemagr.LookupValidRoute (dst, rt)) //テーブルに終点への有効な経路があるかを検索
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

//network layer
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
					if (m_routingTablemagr.LookupRoute (dst, toBroadcast))
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

//network layer
bool RoutingProtocol::Forwarding(Ptr<const Packet> p,const Ipv4Header &header,UnicastForwardCallback ucb,ErrorCallback ecb)
{
	NS_LOG_FUNCTION(this);

	Ipv4Address dst=header.GetDestination();
	Ipv4Address origin=header.GetSource();
	RoutingTableEntry toDst;

	if (m_routingTablemagr.LookupRoute (dst, toDst))
	{
		if (toDst.GetFlag () == VALID)
		{
			Ptr<Ipv4Route> route = toDst.GetRoute ();
			RoutingTableEntry toOrigin;
			m_routingTablemagr.LookupRoute (origin, toOrigin);
			ucb (route, p, header);
			return true;
		}
	}
	return false;
}

//network layer
void RoutingProtocol::SetIpv4(Ptr<Ipv4> ipv4)
{
	NS_LOG_FUNCTION(this);

	//フォワーディングテーブルのインスタンスを取得する
	//プロトコルを開始させる
	m_ipv4=ipv4;
	m_lo = m_ipv4->GetNetDevice (0);
	Simulator::ScheduleNow(&RoutingProtocol::Start,this);
}

//application layer
void RoutingProtocol::Start()
{
	NS_LOG_FUNCTION (this);

	if(my_id==1) {
		StartHandleDataTimer(ADHOC_CLUSTER);
	}

	my_id=m_ipv4->GetObject<Node>()->GetId()+1;
	addr=m_ipv4->GetAddress(m_ipv4->GetInterfaceForDevice(m_ipv4->GetNetDevice(1)),0).GetLocal();

	OutputText_NI(Simulator::Now(),m_ipv4,addr,6);
}

/*
void RoutingProtocol::UpdateTimerExpire()
{
	NS_LOG_FUNCTION (this << " " << UpdateInterval);

	//更新タイマーが切れた場合、自分のルーティングテーブルまたは経路情報を送出する
	SendProtoRoutingTable();

	m_updatetimer.Cancel();//実行中のイベントをキャンセルする
	m_updatetimer.Schedule(UpdateInterval);//タイマーをリセットする
}
*/

//network layer
void RoutingProtocol::SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination)
{
	socket->SendTo (packet, 0, InetSocketAddress (destination, MY_PORT));
}


/*
bool RoutingProtocol::IsMyOwnAddress (Ipv4Address src)
{
	NS_LOG_FUNCTION (this << src);
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
			m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
	{
		Ipv4InterfaceAddress iface = j->second;
		if (src == iface.GetLocal ())
		{
			return true;
		}
	}
	return false;
}
*/

//network layer
void RoutingProtocol::NotifyInterfaceUp(uint32_t i)
{
	NS_LOG_FUNCTION(this << m_ipv4->GetAddress(i,0).GetLocal());

	Ptr<Ipv4L3Protocol> l3=m_ipv4->GetObject<Ipv4L3Protocol>();
	Ipv4InterfaceAddress iface=l3->GetAddress(i,0);
	if(iface.GetLocal()==Ipv4Address ("127.0.0.1"))
	{
		return;
	}

	// Create a socket to listen only on this interface
	Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
	NS_ASSERT (socket != 0);
	socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvProto, this));
	socket->BindToNetDevice (l3->GetNetDevice (i));
//	socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), MY_PORT));
	socket->Bind (InetSocketAddress (iface.GetLocal (), MY_PORT));
	socket->SetAllowBroadcast (true);
	socket->SetIpRecvTtl (true);
	socket->SetAttribute ("IpTtl", UintegerValue (1));
	m_socketAddresses.insert (std::make_pair (socket, iface));

	// Create also a  subnet broadcast socket
	socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
	NS_ASSERT (socket != 0);
	socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvProto, this));
	socket->BindToNetDevice (l3->GetNetDevice (i));
	socket->Bind (InetSocketAddress (iface.GetBroadcast (), MY_PORT));
	socket->SetAllowBroadcast (true);
	socket->SetIpRecvTtl (true);
	socket->SetAttribute ("IpTtl", UintegerValue (1));
	m_socketSubnetBroadcastAddresses.insert (std::make_pair (socket, iface));

	//ルーティングテーブルのエントリーを設定する
	Ptr<NetDevice> dev=m_ipv4->GetNetDevice(m_ipv4->GetInterfaceForAddress(iface.GetLocal()));
	Ipv4Mask mask=iface.GetMask();
	Ipv4Address netAddress=iface.GetLocal().CombineMask(mask);
	std::string iftype=(dev->IsPointToPoint()) ? "p2p" : "csma";

	//経路エントリーを作成する
	RoutingTableEntry rt(
			/*device=*/ dev,
			/*dst=*/ netAddress,
			/*iface=*/  iface,
			/*nexthop=*/ Ipv4Address("0.0.0.0"),
			/*matrics=*/ 1,
			/*rtype=*/ "C",
			/*iftype=*/ iftype,
			/*lifetime=*/ Simulator::GetMaximumSimulationTime());
	m_routingTablemagr.AddRoute(rt);

	//std::cout << "\nNode: " << m_ipv4->GetObject<Node>()->GetId() << " Time: " << Simulator::Now().GetSeconds() << "s ";
	//rt.Print();

	m_id=iface.GetLocal();  //10.0.0.1 ~ 10.0.0.100
	m_parent=iface.GetLocal(); //10.0.0.1 ~ 10.0.0.100
	my_id=m_ipv4->GetObject<Node>()->GetId()+1;  //1 ~ 100
	//std::cout<<m_id<<" , "<<m_parent<<" , "<<my_id<<std::endl;
}

Ipv4Address RoutingProtocol::GetIpv4Address(){
	return m_id;
}
	

//network layer
void RoutingProtocol::NotifyInterfaceDown(uint32_t i)
{
	NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ());

	// Close socket
	Ptr<Socket> socket = FindSocketWithInterfaceAddress (m_ipv4->GetAddress (i, 0));
	NS_ASSERT (socket);
	socket->Close ();
	m_socketAddresses.erase (socket);
}

//network layer
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

//network layer
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
	NS_ASSERT_MSG (rt->GetSource () != Ipv4Address (), "Valid AODV source address not found");
	rt->SetGateway (Ipv4Address ("127.0.0.1"));
	rt->SetOutputDevice (m_lo);
	return rt;
}

//network
void RoutingProtocol::RecvProto(Ptr<Socket> socket)
{
	NS_LOG_FUNCTION(this << socket);

	Address sourceAddress;
	//Read a single packet from the socket and retrieve the sender address.
	Ptr<Packet> packet=socket->RecvFrom(sourceAddress);
	InetSocketAddress inetSourceAddr=InetSocketAddress::ConvertFrom(sourceAddress);
	Ipv4Address sender=inetSourceAddr.GetIpv4();
	//Ipv4Address receiver=m_socketAddresses[socket].GetLocal();
	Ipv4Address receiver;
	if (m_socketAddresses.find(socket) != m_socketAddresses.end())
	{
		receiver = m_socketAddresses[socket].GetLocal ();
	}
	else if (m_socketSubnetBroadcastAddresses.find(socket) != m_socketSubnetBroadcastAddresses.end())
	{
		receiver = m_socketSubnetBroadcastAddresses[socket].GetLocal ();
	}

	//UpdateRouteToNeighbor (sender, receiver);
	TypeHeader tHeader(TYPE_ROUTEREQUEST);
	packet->RemoveHeader(tHeader);
	if (!tHeader.IsValid ())
	{
		NS_LOG_DEBUG ("AODV message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
		return; // drop
	}
	switch (tHeader.Get ())
	{

//	case TYPE_HELLO:
//		RecvHello(packet, receiver, sender);
//		break;

	case TYPE_ACK:
//		std::cout<<"TypeAck"<<std::endl;
		RecvAckHello(packet, receiver, sender);
		break;

	case TYPE_DATA:
		RecvDataAsAdhoc(packet, receiver, sender);
		break;

	case TYPE_ROUTEREQUEST:
		RecvRouteRequest(packet, receiver, sender);
		break;

	case TYPE_ROUTEREPLY:
		RecvRouteReply(packet, receiver, sender);
		break;

	case TYPE_DATAACK:
		RecvAck(packet, receiver, sender);
		break;

	default :
		break;
	}
}

/*********************/
/*      Timer系      */
/*********************/
void RoutingProtocol::StartHelloTimer(){
	HelloTimer.SetFunction(&RoutingProtocol::SendHello,this);
	HelloTimer.Remove();
	HelloTimer.Schedule(MilliSeconds (m_uniformRandomVariable->GetInteger (0,500)));
}

void RoutingProtocol::ActivateHelloTimer(){
	HelloTimer.SetFunction(&RoutingProtocol::SendHello,this);
	HelloTimer.Remove();
	HelloTimer.Schedule(Seconds(2.0));
}

void RoutingProtocol::StartHandleDataTimer(uint8_t flag){
	HandleDataTimer.SetFunction(&RoutingProtocol::HandleData,this);
	HandleDataTimer.SetArguments(flag);
	HandleDataTimer.Remove();
//	HandleDataTimer.Schedule(Seconds(m_uniformRandomVariable->GetInteger (10, 100)));
	HandleDataTimer.Schedule(Seconds(1.0));
}

void RoutingProtocol::ActivateHandleDataTimer(uint8_t flag){
	HandleDataTimer.SetFunction(&RoutingProtocol::HandleData,this);
	HandleDataTimer.SetArguments(flag);
	HandleDataTimer.Remove();
	HandleDataTimer.Schedule(MilliSeconds (m_uniformRandomVariable->GetInteger (0,500)));
}

void RoutingProtocol::ActivateUnicastHandleDataTimer(Data data,Ipv4Address dst,double timeCount){
	DataKey dk(data.GetDst(),data.GetSeqNo());
	DataSendUnicastTimer[dk].SetFunction(&RoutingProtocol::SendDataAsUnicast,this);
	DataSendUnicastTimer[dk].SetArguments(data,dst);
	DataSendUnicastTimer[dk].Remove();
	DataSendUnicastTimer[dk].Schedule(Seconds(timeCount));
}

void RoutingProtocol::ActivateRouteRequestTimer(Ipv4Address source,Ipv4Address dst,uint32_t seqNo,uint8_t ttl,double timeCount){
	RouteRequestTimer.SetFunction(&RoutingProtocol::SendRouteRequest,this);
	RouteRequestTimer.SetArguments(source,dst,seqNo,ttl,timeCount);
	RouteRequestTimer.Remove();
	RouteRequestTimer.Schedule(Seconds(timeCount));
}

void RoutingProtocol::ActivateSendRouteRequestTimer(Ipv4Address source,Ipv4Address dst,uint32_t seqNo,uint8_t ttl,double timeCount){
	//DataKey dk(dst,seqNo);
	std::cout<<"Activate:"<<timeCount<<" seqNo:"<<seqNo<<std::endl;
	SendRouteRequestTimer[dst].SetFunction(&RoutingProtocol::SendRouteRequest,this);
	SendRouteRequestTimer[dst].SetArguments(source,dst,seqNo,ttl,timeCount);
	SendRouteRequestTimer[dst].Remove();
	SendRouteRequestTimer[dst].Schedule(Seconds(timeCount));
}

void RoutingProtocol::ActivateUnicastTimeoutTimer(DataKey dk){
	UnicastTimeoutTimer[dk].SetFunction(&RoutingProtocol::UnicastTimeout,this);
	UnicastTimeoutTimer[dk].SetArguments(dk);
	UnicastTimeoutTimer[dk].Remove();
	UnicastTimeoutTimer[dk].Schedule(Seconds(current_data_send));
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

	if(!m_routingTablemagr.LookupRoute(dst,rt)){
		if(my_id==1) {
			std::cout<<"dst:"<<dst<<std::endl;
		}
		m_routingTablemagr.AddRoute(newEntry);
	}else{
		m_routingTablemagr.Update(newEntry);
		//rt.Print();
	}
}

void RoutingProtocol::RecvRoutingTbl(Ptr<Packet> p,Ipv4Address receiver,Ipv4Address sender)
{
	//ルーティングテーブルの更新処理を行う

	NS_LOG_FUNCTION(this);

	//ルーティングプロトコルパケットのデカプセル化処理
	//ルーティングプロトコルのパケットのPayload部の取り出し
	//...

	//ルーティングテーブルの更新処理
	//...
}

void RoutingProtocol::SendProtoRoutingTable()//(*@\label{01-18}@*)
{
	NS_LOG_FUNCTION(this);

	//ルーティングプロトコルのパケットのカプセル化処理
	//自分のルーティング情報を隣接ノードへ送出する
}

bool RoutingProtocol::UpdateRouteLifeTime(Ipv4Address addr,Time lifetime)
{
	NS_LOG_FUNCTION(this << addr << lifetime);

	//経路の維持時間を修正する

	return true;
}

Ptr<Socket> RoutingProtocol::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const
{
	NS_LOG_FUNCTION (this << addr);
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
			m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
	{
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;
		if (iface == addr)
			return socket;
	}
	Ptr<Socket> socket;
	return socket;
}

/***********************/
/*      Data処理系      */
/***********************/
//application layer
void RoutingProtocol::HandleData(uint8_t flag){
	if((int)m_dataseqno[my_id-1]<1){

		uint32_t id[] = {100};
		//if(my_id == 25)
		//	id[0] = {50};
		//RoutingTableEntry toDst;

		for(int i=0;i<(int)(sizeof(id)/sizeof(*id));i++){
			Data data=MakeData(m_id,GenerateDst(id[i]),Ipv4Address(),Ipv4Address(),flag);
			m_db.AddData(data);
			m_dkl.AddKey(data.GetKey());
			std::cout<<"HandleData no TryUnicastRouting\n";
			TryUnicastRouting(data);
		}

//		m_dkl.Print();
	}
}

//application layer
void RoutingProtocol::TryUnicastRouting(Data data){
	SendRouteRequest(data.GetSource(),data.GetDst(),data.GetSeqNo(),18,0);
	m_udc.PushDataKey(data.GetKey());
	ActivateUnicastTimeoutTimer(data.GetKey());
}

//application layer
void RoutingProtocol::UnicastTimeout(DataKey dk){
	SendRouteRequestTimer[dk.GetId()].Cancel();
	SendRouteRequestTimer.erase(dk.GetId());

	UnicastTimeoutTimer[dk].Cancel();
	UnicastTimeoutTimer.erase(dk);
}

Data RoutingProtocol::MakeData(Ipv4Address source,Ipv4Address dst,Ipv4Address optionalSource,Ipv4Address optionalDst,uint8_t flag){
	Data data(source,dst,optionalSource,optionalDst,m_dataseqno[dst.Get()-source.Get()],flag,0,128);

	Ptr<Node> node=m_ipv4->GetObject<Node>();
	Ptr<MobilityModel> mob=node->GetObject<MobilityModel>();
	Vector3D pos = mob->GetPosition ();

	m_sil[data.GetKey()]=SenderInfo(pos.x,pos.y,Simulator::Now().GetSeconds());

	return data;
}

Ipv4Address RoutingProtocol::GenerateDst(uint32_t n_id){
	/*if(m_environment.GetDstType()=="BROADCAST"){
		return "10.255.255.255";
	}else{*/
	Ipv4Address dst;
//	dst.Set(167772160+m_uniformRandomVariable->GetInteger(1,m_environment.GetNumNode()));
	dst.Set(167772160+n_id);
//	dst.Set(167772160+m_uniformRandomVariable->GetInteger(1,100));
//	std::cout<<dst<std;
	return dst;
	//}
}

void RoutingProtocol::SendDataAsFlooding(Data data){
	//std::cout<<"SendDataAsFlooding"<<std::endl;
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
			m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
	{
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;

		m_dataIdCache.IsDuplicate(iface.GetLocal(), m_dataseqno[data.GetDst().Get()-data.GetSource().Get()]);

		Ptr<Packet> packet = Create<Packet> ();
		packet->AddHeader (data);
		TypeHeader tHeader (TYPE_DATA);
		packet->AddHeader (tHeader);
		// Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
		Ipv4Address destination;
		if (iface.GetMask () == Ipv4Mask::GetOnes ())
		{
//			std::cout<<"true"<<std::endl;
			destination = Ipv4Address ("255.255.255.255");
		}
		else
		{
//			std::cout<<"false"<<std::endl;
			destination = iface.GetBroadcast ();
		}
		Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (500, 1000)))/*(0.0+(my_id-1)*26)))*/,
				&RoutingProtocol::SendTo, this, socket, packet, destination);
//		Simulator::Schedule (Time (MilliSeconds (/*m_uniformRandomVariable->GetInteger (0, 20)*/(0.0+(my_id-1)*26))),
//						&RoutingProtocol::RecvDataAsAdhoc, this, packet, destination, iface.GetLocal());

		OutputText_RW(Simulator::Now(),DATA_PACKET,destination,data.GetSeqNo());
//		std::cout<<"SendDataAsFlooding"<<std::endl;

		current_data_send=Simulator::Now().GetSeconds();
	}
}

void RoutingProtocol::SendHello(){
	m_broadcast_id++;

	Hello hello;
	hello.SetId(m_id);
	hello.SetBroadcastId(m_broadcast_id);

	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
				m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
		{
			Ptr<Socket> socket = j->first;
			Ipv4InterfaceAddress iface = j->second;

			m_IdCache.IsDuplicate (iface.GetLocal (), m_broadcast_id);

			Ptr<Packet> packet = Create<Packet> ();
			packet->AddHeader (hello);
			TypeHeader tHeader (TYPE_HELLO);
			packet->AddHeader (tHeader);
			// Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
			Ipv4Address destination;
			if (iface.GetMask () == Ipv4Mask::GetOnes ())
			{
				destination = Ipv4Address ("255.255.255.255");
			}
			else
			{
				destination = iface.GetBroadcast ();
			}
			Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
					&RoutingProtocol::SendTo, this, socket, packet, destination);
			OutputText_RW(Simulator::Now(),HELLO_PACKET,destination,0);
		}

	ActivateHelloTimer();
}

void RoutingProtocol::RecvHello (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender)
{
	Hello hello;
	p->RemoveHeader (hello);

	uint32_t id=hello.GetBroadcastId();

	if (m_IdCache.IsDuplicate (hello.GetId (), id))
	{
		return;

	}

	SetRoute(hello.GetId(),hello.GetId());
	m_parent=hello.GetId();

	SendAckHello(hello);
}

void RoutingProtocol::SendAckHello(Hello hello){
	Ack ack;

	ack.SetId(m_id);
	ack.SetSeqNo(hello.GetBroadcastId());

	RoutingTableEntry toNext;
	if(!m_routingTablemagr.LookupRoute(hello.GetId(),toNext)){
		return;
	}

	Ptr<Packet> packet = Create<Packet> ();
	packet->AddHeader (ack);
	TypeHeader tHeader (TYPE_ACK);
	packet->AddHeader (tHeader);
	Ptr<Socket> socket=FindSocketWithInterfaceAddress (toNext.GetInterface ());
	NS_ASSERT (socket);
	Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
			&RoutingProtocol::SendTo, this, socket, packet, toNext.GetNextHop());
	OutputText_RW(Simulator::Now(),ACK_PACKET,toNext.GetNextHop(),0);
}

void RoutingProtocol::RecvAckHello(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender){

	Ack ack;
	p->RemoveHeader(ack);

	SetRoute(ack.GetId(),ack.GetId());

}

void RoutingProtocol::SendDataAsUnicast(Data data,Ipv4Address dst){
	std::cout<<"SendDataAsUnicast"<<std::endl;

	RoutingTableEntry toDst;

	if(m_dataseqno[data.GetDst().Get()-data.GetSource().Get()] != m_ackseqno[data.GetDst().Get()-data.GetSource().Get()]){
		m_dataseqno[data.GetDst().Get()-data.GetSource().Get()]--;

//		data.SetSeqNo(m_dataseqno[data.GetDst().Get()-data.GetSource().Get()]--);
//		data.SetSource(data.GetSource());
//		data.SetDst(data.GetDst());
		Data data1=MakeData(m_id,GenerateDst(data.GetDst().Get()-167772160),Ipv4Address(),Ipv4Address(),ADHOC_CLUSTER);
//		ActivateUnicastHandleDataTimer(data,dst,2.0);
		m_db.AddData(data1);
		m_dkl.AddKey(data1.GetKey());
		std::cout<<"SendDataAsUnicast no TryUnicastRouting\n";
		TryUnicastRouting(data1);
//		ActivateRouteRequestTimer(data1.GetSource(),data1.GetDst(),data1.GetSeqNo(),18,1.0);
//		SendRouteRequest(data.GetSource(),data.GetDst(),data.GetSeqNo(),18,1.0);
//		m_udc.PushDataKey(data1.GetKey());
		return;
	}

	if(!m_routingTablemagr.LookupRoute(dst,toDst)){
		return;
	}

	Ptr<Packet> packet = Create<Packet> ();
	packet->AddHeader (data);
	TypeHeader tHeader (TYPE_DATA);
	packet->AddHeader (tHeader);
	Ptr<Socket> socket=FindSocketWithInterfaceAddress (toDst.GetInterface ());
	NS_ASSERT (socket);
	Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 30))),
			&RoutingProtocol::SendTo, this, socket, packet, toDst.GetNextHop());
	OutputText_RW(Simulator::Now(),DATA_PACKET,toDst.GetNextHop(),data.GetSeqNo());

	ActivateUnicastHandleDataTimer(data,dst,2.0);
	m_dataseqno[data.GetDst().Get()-data.GetSource().Get()]++;
}

void RoutingProtocol::SendAck(Data data){
	std::cout<<"SendAck"<<std::endl;
	Ipv4Address dst=data.GetSource();

	DataAck ack(m_id,dst,m_ackseqno[data.GetDst().Get()-data.GetSource().Get()]);

	RoutingTableEntry toDst;
	if(!m_routingTablemagr.LookupRoute(dst,toDst)){
		return;
	}


	Ptr<Packet> packet = Create<Packet> ();
	packet->AddHeader (ack);
	TypeHeader tHeader (TYPE_DATAACK);
	packet->AddHeader (tHeader);
	Ptr<Socket> socket=FindSocketWithInterfaceAddress (toDst.GetInterface ());
	NS_ASSERT (socket);
	//network layer
	Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
			&RoutingProtocol::SendTo, this, socket, packet, toDst.GetNextHop());

	controlPacketOnWifi=controlPacketOnWifi+ack.GetSerializedSize();
	OutputText_RW(Simulator::Now(),DATAACK_PACKET,toDst.GetNextHop(),ack.GetSeqNo());
	m_dataseqno[data.GetDst().Get()-data.GetSource().Get()]++;

}

uint32_t n_id = 0;

void RoutingProtocol::RecvAck(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender){
	std::cout<<"RecvAck"<<std::endl;
	DataAck ack;
	p->RemoveHeader(ack);

	SetRoute(sender,sender);
	SetRoute(sender,ack.GetSource());

	if(ack.GetDst()==m_id){
		std::cout<<"received packet"<<std::endl;
		m_ackseqno[ack.GetSource().Get()-ack.GetDst().Get()]++;

		return;
	}

	ForwardAck(ack);
}



void RoutingProtocol::RecvDataAsAdhoc(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender)
{


	NS_LOG_FUNCTION (this << " src " << sender);

	Data data;
	p->RemoveHeader (data);


	uint32_t id=data.GetSeqNo();

	if(id == n_id){
		std::cout<<"-----\n"<<"SeqNo: "<<id+1<<std::endl;
		Time now = Simulator::Now();
		double time = now.GetDouble()/1000000000;
		std::cout<<"time : "<<time<<std::endl;
		n_id++;
	}

	uint32_t hop=data.GetHop()+1;
	data.SetHop(hop);

	if(m_dataIdCache.IsDuplicate(data.GetSource(),id)){
//		std::cout<<"return"<<std::endl;
		return;
	}

	if(data.GetDst()==m_id){
		StoreData(data);
		DetectReceiveData(data);
	}else{
		ForwardDataAsUnicast(data,data.GetDst());
	}
}


bool RoutingProtocol::StoreData(Data data){
	DataKey dk=data.GetKey();
	if(m_db.GetData(dk,data)){
		m_dkl=m_db.MakeKeyList();
		return false;
	}
	m_db.AddData(data);
	m_dkl=m_db.MakeKeyList();
	return true;
}

void RoutingProtocol::DetectReceiveData(Data data){
	if(!is_server){
		std::cout<<"DetectReceiveData"<<std::endl;
		diffusion++;
		DataKey dk=data.GetKey();
		ReceiverInfo ri=m_ril[dk];
		SenderInfo si=m_sil[dk];
		ri.SetDisseminationTime(Simulator::Now().GetSeconds()-m_sil[dk].GetSendTime());
		ri.SetDistance(sqrt(pow(si.GetX()-ri.GetX(),2.0)+pow(si.GetY()-ri.GetY(),2.0)));
		m_ril[dk]=ri;

		double num=dissemination_cache[dk]+1;
		dissemination_cache[dk]=num;

		dissemination_wifi++;

		int hop=data.GetHop();

		std::map<int,int>::iterator mi=hop_info.find(hop);
		if(mi!=hop_info.end()){
			int count=hop_info[hop]+1;
			hop_info[hop]=count;
		}else{
			hop_info[hop]=1;
		}
	}
	SendAck(data);
}

void RoutingProtocol::ForwardDataAsFlooding(Data data){
	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j){
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;
		Ptr<Packet> packet = Create<Packet> ();
		packet->AddHeader (data);
		TypeHeader tHeader (TYPE_DATA);
		packet->AddHeader (tHeader);
		// Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
		Ipv4Address destination;
		if (iface.GetMask () == Ipv4Mask::GetOnes ())
		{
			destination = Ipv4Address ("255.255.255.255");
		}
		else
		{
			destination = iface.GetBroadcast ();
		}
		//network layer
		Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
				&RoutingProtocol::SendTo, this, socket, packet, destination);

		OutputText_RW(Simulator::Now(),DATA_PACKET,destination,data.GetSeqNo());
	}
}

void RoutingProtocol::ForwardDataAsUnicast(Data data,Ipv4Address dst){
	RoutingTableEntry toDst;

	if(!m_routingTablemagr.LookupRoute(dst,toDst)){
		return;
	}

//	data.SetSeqNo(m_dataseqno);
	m_dataseqno[data.GetDst().Get()-data.GetSource().Get()]=data.GetSeqNo();


	Ptr<Packet> packet = Create<Packet> ();
	packet->AddHeader (data);
	TypeHeader tHeader (TYPE_DATA);
	packet->AddHeader (tHeader);
	Ptr<Socket> socket=FindSocketWithInterfaceAddress (toDst.GetInterface ());
	NS_ASSERT (socket);
	//network layer
	Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
			&RoutingProtocol::SendTo, this, socket, packet, toDst.GetNextHop());

	OutputText_RW(Simulator::Now(),DATA_PACKET,toDst.GetNextHop(),data.GetSeqNo());
	std::cout<<"ForwardDataAsUnicast "<<toDst.GetNextHop()<<std::endl;
	//std::cout<<"ForwardDataAsUnicast"<<std::endl;
//	m_dataseqno++;
}

void RoutingProtocol::ForwardAck(DataAck ack){
	RoutingTableEntry toDst;
	if(!m_routingTablemagr.LookupRoute(ack.GetDst(),toDst)){
		return;
	}

	Ptr<Packet> packet = Create<Packet> ();
	packet->AddHeader (ack);
	TypeHeader tHeader (TYPE_DATAACK);
	packet->AddHeader (tHeader);
	Ptr<Socket> socket=FindSocketWithInterfaceAddress (toDst.GetInterface ());
	NS_ASSERT (socket);
	//network layer
	Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
			&RoutingProtocol::SendTo, this, socket, packet, toDst.GetNextHop());

	controlPacketOnWifi=controlPacketOnWifi+ack.GetSerializedSize();
	OutputText_RW(Simulator::Now(),DATAACK_PACKET,toDst.GetNextHop(),ack.GetSeqNo());
}

/*********/
/*Unicast*/
/*********/
//application layer
void RoutingProtocol::SendRouteRequest(Ipv4Address source,Ipv4Address dst,uint32_t seqNo,uint8_t ttl,double timeCount){
	RouteRequest rreq(source,dst,seqNo,ttl,m_rreqId);
	std::cout<<"SendRouteRequest "<<source<<" to "<<dst<<std::endl;
//	std::cout<<m_rreqId<<std::endl;


	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
				m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
		{
			Ptr<Socket> socket = j->first;
			Ipv4InterfaceAddress iface = j->second;

			m_rreqIdCache.IsDuplicate (iface.GetLocal (), m_rreqId);

			Ptr<Packet> packet = Create<Packet> ();
			packet->AddHeader (rreq);
			TypeHeader tHeader (TYPE_ROUTEREQUEST);
			packet->AddHeader (tHeader);
//			Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
			Ipv4Address destination;
			if (iface.GetMask () == Ipv4Mask::GetOnes ())
			{
				destination = Ipv4Address ("255.255.255.255");
			}
			else
			{
//				std::cout<<my_id<<std::endl;
				destination = iface.GetBroadcast ();
			}
//			socket->SendTo(packet,0, InetSocketAddress (destination, MY_PORT));
			Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 100))),
							&RoutingProtocol::SendTo, this, socket, packet, destination);

			controlPacketOnWifi=controlPacketOnWifi+rreq.GetSerializedSize();
			OutputText_RW(Simulator::Now(),ROUTEREQUEST_PACKET,destination,rreq.GetSeqNo());
		}

	m_rreqId++;


	ActivateSendRouteRequestTimer(source,dst,seqNo+1,ttl+3,timeCount+1);


}

//application layer
void RoutingProtocol::ForwardRouteRequest(RouteRequest rreq){
//	std::cout<<my_id<<std::endl;

	for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
			m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
	{
		Ptr<Socket> socket = j->first;
		Ipv4InterfaceAddress iface = j->second;
		Ptr<Packet> packet = Create<Packet> ();
		packet->AddHeader (rreq);
		TypeHeader tHeader (TYPE_ROUTEREQUEST);
		packet->AddHeader (tHeader);
		// Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
		Ipv4Address destination;
		if (iface.GetMask () == Ipv4Mask::GetOnes ())
		{
			destination = Ipv4Address ("255.255.255.255");
		}
		else
		{
			std::cout<<"ForwardRouteRequest " << my_id<<std::endl;
			destination = iface.GetBroadcast ();
		}
		//network layer
		Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
				&RoutingProtocol::SendTo, this, socket, packet, destination);

		controlPacketOnWifi=controlPacketOnWifi+rreq.GetSerializedSize();
		OutputText_RW(Simulator::Now(),ROUTEREQUEST_PACKET,destination,rreq.GetSeqNo());
	}
}

//aplication layer
void RoutingProtocol::RecvRouteRequest(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender){
	RouteRequest rreq;
	p->RemoveHeader(rreq);

	uint32_t id=rreq.GetRreqId();

	if (m_rreqIdCache.IsDuplicate (rreq.GetSource(), id)){

		return;
	}



	SetRoute(sender,sender);
	SetRoute(sender,rreq.GetSource());

	if(rreq.GetDst()==m_id){
		Data data;
		if(!m_db.GetData(DataKey(rreq.GetSource(),rreq.GetSeqNo()),data)){

			SendRouteReply(rreq);
		}
		return;
	}

	uint8_t ttl=rreq.GetTtl();

	if(ttl>0){
		rreq.SetTtl(ttl-1);
		ForwardRouteRequest(rreq);
	}
}

//application layer
void RoutingProtocol::SendRouteReply(RouteRequest rreq){
	std::cout<<"SendRouteReply"<<my_id<<std::endl;
	Ipv4Address dst=rreq.GetSource();

	RouteReply rrep(m_id,dst,rreq.GetSeqNo());

	RoutingTableEntry toDst;
	if(!m_routingTablemagr.LookupRoute(dst,toDst)){
		return;
	}

	Ptr<Packet> packet = Create<Packet> ();
	packet->AddHeader (rrep);
	TypeHeader tHeader (TYPE_ROUTEREPLY);
	packet->AddHeader (tHeader);
	Ptr<Socket> socket=FindSocketWithInterfaceAddress (toDst.GetInterface ());
	NS_ASSERT (socket);
	//network layer
	Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
			&RoutingProtocol::SendTo, this, socket, packet, toDst.GetNextHop());

	controlPacketOnWifi=controlPacketOnWifi+rrep.GetSerializedSize();
	OutputText_RW(Simulator::Now(),ROUTEREPLY_PACKET,toDst.GetNextHop(),rrep.GetSeqNo());
}

//application layer
void RoutingProtocol::ForwardRouteReply(RouteReply rrep){
	RoutingTableEntry toDst;
	if(!m_routingTablemagr.LookupRoute(rrep.GetDst(),toDst)){
		return;
	}

	Ptr<Packet> packet = Create<Packet> ();
	packet->AddHeader (rrep);
	TypeHeader tHeader (TYPE_ROUTEREPLY);
	packet->AddHeader (tHeader);
	Ptr<Socket> socket=FindSocketWithInterfaceAddress (toDst.GetInterface ());
	NS_ASSERT (socket);
	//network layer
	Simulator::Schedule (Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 20))),
			&RoutingProtocol::SendTo, this, socket, packet, toDst.GetNextHop());

	controlPacketOnWifi=controlPacketOnWifi+rrep.GetSerializedSize();
	OutputText_RW(Simulator::Now(),ROUTEREPLY_PACKET,toDst.GetNextHop(),rrep.GetSeqNo());
}

//application layer
void RoutingProtocol::RecvRouteReply(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender){
	std::cout<<"RecvRouteReply:"<<my_id<<std::endl;
	RouteReply rrep;
	p->RemoveHeader(rrep);

	SetRoute(sender,sender);
	SetRoute(sender,rrep.GetSource());

	std::cout<<sender<<std::endl;
	if(rrep.GetDst()==m_id){
		std::cout<<"m_id=" << m_id<< std::endl;
		std::cout<<receiver<<std::endl;
		std::cout<<"rrep:"<<rrep.GetDst()<<std::endl;

		Data data;
		DataKey dk(rrep.GetSource(),rrep.GetSeqNo()+1);
		DataKey dk1(rrep.GetSource(),m_dataseqno[rrep.GetSource().Get()-rrep.GetDst().Get()]);

		if(m_db.GetData(dk1,data)){
			m_routingTablemagr.Print();
			std::cout<<data.GetSeqNo()<<std::endl;
			std::cout<<data.GetDst()<<std::endl;
			SendDataAsUnicast(data,data.GetDst());
		}
		std::cout<<"rrep_source:"<<rrep.GetSource()<<std::endl;
	

		SendRouteRequestTimer[rrep.GetSource()].Cancel();
		SendRouteRequestTimer.erase(rrep.GetSource());

		UnicastTimeoutTimer[dk].Cancel();
		UnicastTimeoutTimer.erase(dk);

		return;
	}

	ForwardRouteReply(rrep);
}

//↓↓↓↓↓ ClusterViewer ↓↓↓↓↓//
void RoutingProtocol::OutputText_NI(Time now ,Ptr<Ipv4> m_ipv4,Ipv4Address parent,int color){
	int64_t time=now.GetInteger();
	int64_t roughTime=time/1000000000;
	int64_t splitSecond=time%1000000000;
	Ptr<Node> node=m_ipv4->GetObject<Node>();

	Ptr<MobilityModel> obj=node->GetObject<MobilityModel>();
	double x=obj->GetPosition().x;
	double y=obj->GetPosition().y;
	int nodeid=m_ipv4->GetObject<Node>()->GetId()+1;

	std::ofstream fout;
	fout.open("simpleaodv_broadcast_unicast_view.txt",std::ios::app);
	fout<<"NI, "<<roughTime<<"."<<splitSecond<<", "<<nodeid
			<<", "<<x<<", "<<y<<", 0, "<<color<<", "
			<<parent.Get()-167772160<<", 1"<<std::endl;
	fout.close();

	HandlClusterViewTimer();
}

void RoutingProtocol::WriteForFile_RW(std::string packetName,Ipv4Address dst,double time,uint64_t splitSecond,uint32_t seqno){
	std::ofstream fout;
	fout.open("simpleaodv_broadcast_unicast_view.txt",std::ios::app);
	fout<<"RW, "<<time<<", "<<my_id<<", 120, ";
	if(dst==Ipv4Address("10.255.255.255")){
		fout<<"-1";
	}else{
		fout<<dst.Get()-167772160;
	}
	fout<<" , "<<packetName<<" , seq:"<<seqno<<std::endl;
	fout.close();
}

void RoutingProtocol::WriteForFile_ACK(std::string packetName,Ipv4Address dst,double time,uint64_t splitSecond){
	std::ofstream fout;
	fout.open("simpleaodv_broadcast_unicast_view.txt",std::ios::app);
	fout<<"RW, "<<time<<", "<<my_id<<", 250, ";
	if(dst==Ipv4Address("10.255.255.255")){
		fout<<"-1";
	}else{
		fout<<dst.Get()-167772160;
	}
	fout<<" , "<<packetName<<" , seq:"<<m_ackseqno[dst.Get()-167772160]<<std::endl;
	fout.close();
}

void RoutingProtocol::WriteForFile_DATA(std::string packetName,Ipv4Address dst,double time,uint64_t splitSecond){
	std::ofstream fout;
	fout.open("simpleaodv_broadcast_unicast_view.txt",std::ios::app);
	fout<<"RW, "<<time<<", "<<my_id<<", 250, ";
	if(dst==Ipv4Address("10.255.255.255")){
		fout<<"-1";
	}else{
		fout<<dst.Get()-167772160;
	}
	fout<<" , "<<packetName<<" , seq:"<<m_ackseqno[dst.Get()-167772160]<<std::endl;
	fout.close();
}

void RoutingProtocol::OutputText_RW(Time now,int pktType,Ipv4Address dst,uint32_t seqno){
	int64_t time=now.GetInteger();
	double ttt =now.GetDouble()/1000000000;
	//int64_t roughTime=time/1000000000;
	int64_t splitSecond=time%1000000000;

	switch(pktType){
	case HELLO_PACKET:
		WriteForFile_RW("HELLO",dst,ttt,splitSecond,seqno);
		//std::cout<<"HELLO_PACKET"<<std::endl;
		break;
	case DATAACK_PACKET:
		WriteForFile_RW("ACK",dst,ttt,splitSecond,seqno);
			//std::cout<<"ACK_PACKET"<<std::endl;
			break;
	case ACK_PACKET:
		WriteForFile_RW("ACK",dst,ttt,splitSecond,seqno);
		//std::cout<<"ACK_PACKET"<<std::endl;
		break;
	case DATA_PACKET:
		WriteForFile_RW("DATA",dst,ttt,splitSecond,seqno);
		//std::cout<<"DATA_PACKET"<<std::endl;
		break;
	case ROUTEREQUEST_PACKET:
		WriteForFile_RW("RREQ",dst,ttt,splitSecond,seqno);
		//std::cout<<"ROUTEREQUEST"<<std::endl;
		break;
	case ROUTEREPLY_PACKET:
		WriteForFile_RW("RREP",dst,ttt,splitSecond,seqno);
		//std::cout<<"ROUTEREPLY"<<std::endl;
		break;
	}
}

void RoutingProtocol::HandlClusterViewTimer(){
	Timer timer(Timer::CANCEL_ON_DESTROY);
	ClusterViewTimer=timer;

	ClusterViewTimer.SetFunction(&RoutingProtocol::WriteLog,this);
	ClusterViewTimer.Remove();
	ClusterViewTimer.Schedule(Time(LOG_TIMER));
}

void RoutingProtocol::WriteLog(){
	//if(is_server){
	OutputText_NI(Simulator::Now(),m_ipv4,m_parent,100);
	//}
	//OutputText_NI(Simulator::Now(),m_ipv4,m_parent,7);
}

}
}

