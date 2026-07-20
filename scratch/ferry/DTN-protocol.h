/*
 * simple-protocol.h
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#ifndef SIMPLE_PROTOCOL_H_
#define SIMPLE_PROTOCOL_H_

#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/config.h"

#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <math.h>
#include <string>
#include <sstream>
#include <list>

#include "data-counter.h"
//#include "P2P-packet.h"
//#include "P2Pserver-data-container.h"
#include "simple-id-cache.h"
#include "simple-packet.h"
#include "unicast-data-container.h"
#include "json_data.h"
//#define L 10
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

#define FROMRECV 1 //データ受信から呼ばれた
#define FROMTIMER 2 //タイマー周期で呼ばれた

#define ON 0
#define NSN 1
#define CN 2
#define BN 3
#define BCN 4

#define LOG_TIMER 1

namespace ns3 {
namespace simple {

const int META_DATA_PACKET=1;
const int HELLO_PACKET = 2;
const int USERDATA_PACKET=3;
const int SHELTERDATA_PACKET = 4;

static int frequency;

class RoutingProtocol: public Ipv4RoutingProtocol
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

	int8_t GetNOW(void){return now;}

	Ipv4Address GetClusterID(void){return c_id;}
	uint8_t Gethop(void){return m_hop;}
	static void SetMode(String s);
	//static void SetBlock(String block); //2023/10/10追加
	static bool GetMode(){return m_flag;}
	//static bool GetBlock(){return b_flag;}

	static void SetUseQuickHelloflag(String s);
	static bool GetUseQuickHelloflag(){return quickhelloflag;}
	static void SetUseEraseInfoflag(String s);
	static bool GetUseEraseInfoflag(){return eraseinfoflag;}

	void DatasendTimerCancel();
	void SendHello();
	void SendShelterHello();
	void SendShelterInfoAsBroadcast(ShelterInfo data);
	void SendShelterInfoAsUnicast(ShelterInfo data,Ipv4Address dst);
	void SendFirstHello();
	void setnow(uint32_t joutai){now = joutai;}

	void CheckAoI();
	// void HelloCancelTimer(double delay);

	static int Get_Frequency(){return frequency;}



private:
	//経路更新の時間間隔
	Time UpdateInterval;
	//経路のホールドダウンタイム
	Time ActiveRouteTimeout;
	//IPプロトコル
	Ptr<Ipv4> m_ipv4;

	EventId helloid;
	EventId dataevent;


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

	//クラスタの状態
	uint8_t m_hop;
	uint8_t m_size;
	uint32_t now;
	Ipv4Address m_id;
	Ipv4Address c_id;
	Ipv4Address m_parent;
	uint32_t cluster_cellular;
	bool during_merge;
	bool during_split;
	uint8_t num_children;

	bool is_server;

	//自分がP2Pサーバーであるかの判断
	bool is_P2Pserver;

	//クラスタに参加するかの判断
	bool do_clustering;

	uint32_t m_broadcast_id;

	int my_id;
	uint32_t nodeId;

	Ipv4Address addr;
	uint8_t m_nodeState;

	//データパケットを作成する場合のシーケンスナンバー
	uint32_t m_dataseqno;

	uint32_t m_ackseqno;

	//クラスタメンバリスト
	ClusterMemberList m_cml;

	ClusterAllMemberList m_caml;

	ClusterMemberList b_cml;
	Ipv4Address b_cid;

	//隣接クラスタリスト
	NeighborClusterList m_ncl;

	//スーパピアリスト
	SuperPeerInfoList m_spil;


	//受信したデータパケットを格納するバッファ

	//保持しているデータパケットのリスト
	DataKeyList m_dkl;

	UnicastDataContainer m_udc;

	/// Handle duplicated MEP,MQAP,CNCP
	IdCache m_mepIdCache;
	IdCache m_mqapIdCache;
	IdCache m_cncpIdCache;
	IdCache m_dataIdCache;
	IdCache m_rreqIdCache;

	//Broadcast ID
	uint32_t m_mepId;
	uint32_t m_mqapId;
	uint32_t m_cncpId;
	uint32_t m_rreqId;

	//分割, 結合からの経過を表すID
	uint32_t m_stateId;

	//ルーティングテーブルマネージャー
	RoutingTable m_routingTable;

	//送受信されたデータパケットの観測に使用
	uint32_t recv_data_times;
	uint32_t send_data_times;
	ReceiverInfoList m_ril;

	//エピデミックルーティングをするかの判断
	bool do_epidemic;

	bool do_P2P;

	bool do_optimize;

	uint8_t m_epidemicLimit;

	double m_maxrange;

	uint32_t nodeType;


//	std::set<String> blockednodeid;
//	String findblockednodeid;

//	static std::set<String> fullexitnodeid;
//	static String findfullexitnodeid;


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

	//自分のデータをチェックする関数
	void CheckHelloData(UserData data, Hop hop, Ipv4Address sender);
	void CancelHello();

	//ルーティングプロトコルのパケットを受信する
	void RecvProto(Ptr<Socket>);
	void RecvHello(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);
	//ルーティングプロトコルのパケットを処理する
	//データパケット受信(adhoc)
	void RecvUserDataAsUnicast(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);
	void RecvShelterDataAsBroadcast(Ptr<Packet> p, Ipv4Address receiver);

	void SendRequest(DataKeyList dkl,Ipv4Address dst);


	void SendAckHello(Ipv4Address dst);
	void RecvAckHelloAsBroadcast(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);

	//フラッディングでデータパケットを送信
	void SendUserDataAsBroadcast (UserData data);
	void SendUserDataAsUnicast(UserData data, Hop hop, Ipv4Address dst);
	//ルーティングテーブルを送信する
	void SendProtoRoutingTable();

	//経路構築
	void SetRoute(Ipv4Address next,Ipv4Address dst);

	void SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination, int packetType);

	//HELLO送信を制御するタイマーを起動
	void ActivateHelloTimer(double interval);
	void AdduserHelloTimer(double delay, std::vector<uint32_t> vec);
	// void HelloCancel(){HelloTimer.Cancel();}
	void ActivateSendRouteRequestTimer(Ipv4Address source,Ipv4Address dst,uint32_t seqNo,uint8_t ttl,double timeCount);
	//adhocでデータパケットを送信する場合のタイマーの開始
	void HandleUserDataTimer(double delay);
	void StartOutputLogTimer(double delay);
	void ActivateOutputLogTimer(double delay);
	void HandleDataAsFloodingTimer(double delay); //10/20追加
	void HandlClusterViewTimer();
	void ActivateCheckDataTimer(double delay);
	void HandleShelterDataTimer(double delay);
	void AoiTimer(double delay);
	void ALLAoITimer(double delay);
	void BlockAoiTimer(double delay);
	void SetSocketTimer(std::vector<uint32_t> vec);
	void AddUserCheckDataTimer(std::vector<uint32_t> vec);
	void CheckMovingUserNumTimer(double delay);
	void CheckMovingAddUserNumTimer(std::vector<uint32_t> vec);
	void ActivateCheckAoITimer(double delay);

	void WriteAoiLog(void);
	void WriteBlockAoiLog(void);
	void CheckMovingUserNum(void);

	void AddUserCheckAoITimer(std::vector<uint32_t> vec);
	
	
	//データパケットを送信する場合の制御
	void HandleUserData (Ipv4Address destination, uint32_t sender_userID);
	void HandleTestData(Ipv4Address destination);
	void HandleShelterData();
	void HandleSectionData (void);
	void HandleBuildingData (void);
	UserData MakeSendData(Ipv4Address dst);
	//データパケット作成

	UserData MakeUserData(uint32_t userId, Ipv4Address dst, std::set<String>b_set, std::set<String>fullexit_set, std::map <String, std::pair<uint32_t, uint32_t>>aoi_exitinfo);

	Ipv4Address GenerateDst();
	//データパケットの転送

	void HelloCancelTimer(double delay);
	// void HelloCancel(double delay);


	//cluster viewer
	void OutputText_NI(Time now,Ptr<Ipv4> m_ipv4,std::string color,Ipv4Address parent,Ipv4Address clusterId,uint8_t hop);
	void OutputClusterMemberAverage(Time now_t);
	void OutputDatasize(Time now_t, int size);
	void WriteForFile_RW(std::string packetName,Ipv4Address dst,double time,uint64_t splitSecond);
	void OutputText_RW(Time now,int pktType,Ipv4Address dst);
	void WriteLog();
	void WriteALLAoILog();

	void OutputLog();
	void OutputText_Recive_Data(Ipv4Address sender, Ipv4Address reciver);
	void OutputText_HelloRecvTimeInfo(Ipv4Address sender, Ipv4Address reciver);
	void OutputText_HelloSendTimeInfo(u_int32_t senderId, bool quickhelloflag); //QuickHelloによるHelloかのフラグ
	

	void OutputCML(Time now,Ptr<Ipv4> m_ipv4,std::string color,Ipv4Address parent,Ipv4Address clusterId,uint8_t hop,ClusterMemberList cml);


	void SetMaxRange(double value);

	void SetSocket();
	void CompareHopData(Hop r_hop, Hop m_hop);
 

	//経路更新用タイマー
	Timer m_updatetimer;

	//乱数ジェネレータ
	Ptr<UniformRandomVariable> m_uniformRandomVariable;

	Timer AddCheckDataTimer;
	Timer CheckDataTimer;
	Timer HelloTimer;
	// Timer HelloCancelTimer;
	Timer ShelterHelloTimer;
	Timer CancelHelloTimer;
	Timer DataAsFloodingTimer;
	Timer AckTimer;
	Timer rreqTimer;
	Timer ClusterViewTimer;
	Timer CMLprintTimer;
	Timer fromAPtoUserHandleDataTimer;
	Timer fromAPtoAPHandleDataTimer;
	Timer fromUsertoAPHandleDataTimer;
	Timer HandleDataTimer;
	Timer AggregateDataTimer;
	Timer OutputLogTimer;
	Timer SectionDataContainerTimer;
	Timer shelterdataTimer;
	Timer AoITimer;
	Timer ALLAoiTimer;
	Timer BlockAoITimer;
	Timer SocketTimer;
	Timer MovingUserNumTimer;
	Timer CheckAoITimer;
	Timer AddCheckAoITimer;
	Timer HopTimer;

	static bool m_flag;
	static bool quickhelloflag;
	static bool eraseinfoflag;

};

} //namespace simple
} //namespace ns3

#endif /* SIMPLE_PROTOCOL_H_ */
