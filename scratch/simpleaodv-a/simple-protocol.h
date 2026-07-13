/*
 * simple-protocol.h
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#ifndef SIMPLE_PROTOCOL_H_
#define SIMPLE_PROTOCOL_H_

#include "simple-rtable.h"
#include "simple-id-cache.h"
#include "simple-packet.h"

#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/rng-seed-manager.h"

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

#define L 10
//#define U 50

#define MEP_INTERVAL 1
#define MEP_TTL 2
#define MAP_DELAY 0.05
#define MAP_RANDOM 0.005
#define GATEWAY_LIMIT 2
#define RECV_MEP_LIMIT 2
#define RECV_MAP_LIMIT 2
#define MERGE_LIMIT 1
#define SPLIT_LIMIT 1

#define ON 0
#define NSN 1
#define CN 2
#define BN 3
#define BCN 4

#define LOG_TIMER 1000000000

namespace ns3 {
namespace simple {

const int HELLO_PACKET=1;
const int ACK_PACKET=2;
const int DATA_PACKET=10;
const int ROUTEREQUEST_PACKET=13;
const int ROUTEREPLY_PACKET=14;
const int DATAACK_PACKET=20;

class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
	RoutingProtocol();
	virtual ~RoutingProtocol();
	virtual void DoDispose();

	static TypeId GetTypeId(void);
	static const uint32_t MY_PORT;

	//パケットの転送経路を決定する処理関数
	Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p,const Ipv4Header &header,Ptr<NetDevice> oif,Socket::SocketErrno &sockerr);
	//受信パケットの配送処理関数
	bool RouteInput(Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
					UnicastForwardCallback ucb, MulticastForwardCallback mcb,
					LocalDeliverCallback lcb,ErrorCallback ecb);
	virtual void NotifyInterfaceUp (uint32_t interface);
	//I/Fが使用不可能なったときの処理関数
	virtual void NotifyInterfaceDown(uint32_t interface);
	//I/FにIPアドレスを指定したときの処理関数
	virtual void NotifyAddAddress(uint32_t i,Ipv4InterfaceAddress address);
	//I/FにIPアドレスを削除したときの処理関数
	virtual void NotifyRemoveAddress(uint32_t i,Ipv4InterfaceAddress address);
	//Ipv4のインスタンスを取得する関数
	virtual void SetIpv4(Ptr<Ipv4> ipv4);
	//ルーティングテーブルを出力する関数
	virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;

	//乱数系列のストリーム番号を割り当てる関数
	int64_t AssignStreams(int64_t stream);

	Ipv4Address GetIpv4Address();



private:
	//経路更新の時間間隔
	Time UpdateInterval;
	//経路のホールドダウンタイム
	Time ActiveRouteTimeout;
	//IPプロトコル
	Ptr<Ipv4> m_ipv4;
	

	//1つのIPインターフェースに1つのソケットを生成する
	//(socket,I/F address(IP+mask)のマップを定義する
	std::map<Ptr<Socket>,Ipv4InterfaceAddress> m_socketAddresses;
	std::map<Ptr<Socket>,Ipv4InterfaceAddress> m_socketSubnetBroadcastAddresses;

	//ルーティングテーブルマネージャー
	RoutingTable m_routingTablemagr;
	/// Handle duplicated
	IdCache m_IdCache;
	///ループバック
	Ptr<NetDevice> m_lo;

	Ipv4Address m_id;
	Ipv4Address m_parent;

	bool is_server;

	uint32_t m_broadcast_id;

	int my_id;
	Ipv4Address addr;
	uint8_t m_nodeState;

	//データパケットを作成する場合のシーケンスナンバー
	uint32_t m_dataseqno[100];


	uint32_t m_ackseqno[100];

	//受信したデータパケットを格納するバッファ
	DataBuffer m_db;

	//保持しているデータパケットのリスト
	DataKeyList m_dkl;

	UnicastDataContainer m_udc;

	IdCache m_dataIdCache;
	IdCache m_rreqIdCache;

	uint32_t m_rreqId;

	//ルーティングテーブルマネージャー
//	RoutingTable m_routingTable;

	//送受信されたデータパケットの観測に使用
	uint32_t recv_data_times;
	uint32_t send_data_times;
	ReceiverInfoList m_ril;

private:
	//プロトコルの開始処理
	void Start();
	//使用可能な経路があればパケットを転送する
	bool Forwarding(Ptr<const Packet> p,const Ipv4Header &header,UnicastForwardCallback uvb,ErrorCallback ecb);
	//ホールドダウンタイマーの処理
	bool UpdateRouteLifeTime(Ipv4Address addr,Time lt);
	void UpdateRouteToNeighbor (Ipv4Address sender, Ipv4Address receiver);
	// Check that packet is send from own interface
	bool IsMyOwnAddress (Ipv4Address src);
	//ローカルI/Fアドレスifaceに対応するソケットを参照する
	Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr) const;
	//渡されたヘッダ情報によりループバック経路を作成する
	Ptr<Ipv4Route> LoopbackRoute(const Ipv4Header &header,Ptr<NetDevice> oif) const;

	//ルーティングプロトコルのパケットを受信する
	void RecvProto(Ptr<Socket>);
	void RecvRouteRequest(Ptr<Packet> p,Ipv4Address receiver,Ipv4Address sender);
	void RecvRouteReply(Ptr<Packet> p,Ipv4Address receiver,Ipv4Address sender);
	void RecvHello(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);
	void RecvAck(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);
	//ルーティングプロトコルのパケットを処理する
	void RecvRoutingTbl(Ptr<Packet> packet,Ipv4Address receiver,Ipv4Address sender);
	//データパケット受信(adhoc)
	void RecvDataAsAdhoc(Ptr<Packet> p,Ipv4Address receiver,Ipv4Address sender);

	void SendHello();
	void SendAck(Data data);
	void SendAckHello(Hello hello);
	void RecvAckHello(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);
	void SendRequest(DataKeyList dkl,Ipv4Address dst);
	void SendRouteRequest(Ipv4Address source,Ipv4Address dst,uint32_t seqNo,uint8_t ttl,double timeCount);
	void SendRouteReply(RouteRequest rreq);
	//フラッディングでデータパケットを送信
	void SendDataAsFlooding(Data data);
	void SendDataAsUnicast(Data data,Ipv4Address dst);
	//ルーティングテーブルを送信する
	void SendProtoRoutingTable();

	void SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination);

	void ForwardRouteRequest(RouteRequest rreq); //REQUEST送信
	void ForwardRouteReply(RouteReply rrep); //REPLY送信

	void DetectReceiveData(Data);
	void ForwardDataAsUnicast(Data data,Ipv4Address dst);
	void ForwardAck(DataAck ack);

	//HELLO送信を制御するタイマーを起動
	void StartHelloTimer();
	void ActivateHelloTimer();
	void ActivateRouteRequestTimer(Ipv4Address source,Ipv4Address dst,uint32_t seqNo,uint8_t ttl,double timeCount);
	void ActivateSendRouteRequestTimer(Ipv4Address source,Ipv4Address dst,uint32_t seqNo,uint8_t ttl,double timeCount);
	//adhocでデータパケットを送信する場合のタイマーの開始
	void StartHandleDataTimer(uint8_t flag);
	void ActivateHandleDataTimer(uint8_t flag);
	void ActivateUnicastHandleDataTimer(Data data,Ipv4Address dst,double timeCount);
	void ActivateUnicastTimeoutTimer(DataKey dk);
	void HandlClusterViewTimer();
	void UpdateTimerExpire(); //更新タイマーの処理
	void UnicastTimeout(DataKey dk);

	//データパケットを送信する場合の制御
	void HandleData(uint8_t flag);
	//データパケット作成
	Data MakeData(Ipv4Address source,Ipv4Address dst,Ipv4Address optionalSource,Ipv4Address optionalDst,uint8_t flag);
	bool StoreData(Data data);
	Ipv4Address GenerateDst(uint32_t n_id);
	//経路構築
	void SetRoute(Ipv4Address next,Ipv4Address dst);
	void TryUnicastRouting(Data data);
	//データパケットの転送
	void ForwardDataAsFlooding(Data data);

	//cluster viewer
	void OutputText_NI(Time now,Ptr<Ipv4> m_ipv4,Ipv4Address parent,int color);
	void WriteForFile_RW(std::string packetName,Ipv4Address dst,double time,uint64_t splitSecond,uint32_t seqno);
	void WriteForFile_ACK(std::string packetName,Ipv4Address dst,double time,uint64_t splitSecond);
	void WriteForFile_DATA(std::string packetName,Ipv4Address dst,double time,uint64_t splitSecond);
	void OutputText_RW(Time now,int pktType,Ipv4Address dst,uint32_t seqno);
	void WriteLog();

	//経路更新用タイマー
	Timer m_updatetimer;

	//乱数ジェネレータ
	Ptr<UniformRandomVariable> m_uniformRandomVariable;

	Timer HelloTimer;
	Timer AckTimer;
	Timer rreqTimer;
	Timer ClusterViewTimer;
	Timer HandleDataTimer;
	Timer RouteRequestTimer;
	Timer RouteRequestTimer1;

	//std::unordered_map<DataKey,Timer,DataKey::Hash> SendRouteRequestTimer;
	std::map<Ipv4Address,Timer> SendRouteRequestTimer;
	std::unordered_map<DataKey,Timer,DataKey::Hash> DataSendUnicastTimer;
	std::unordered_map<DataKey,Timer,DataKey::Hash> UnicastTimeoutTimer;

};

} //namespace simple
} //namespace ns3

#endif /* SIMPLE_PROTOCOL_H_ */
