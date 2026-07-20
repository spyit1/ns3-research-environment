/*
 * test.h
 *
 *  Created on: 2016/03/03
 *      Author:
 */

#ifndef SIMPLE_USER_H_
#define SIMPLE_USER_H_

#include <stdint.h>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <list>
#include <random>
#include <map>
//#include <initializer_list>

#include "evacuation-section.h"
#include "json_data.h"
#include "ns3/timer.h"
#include "ns3/node.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/waypoint.h"
#include "ns3/waypoint-mobility-model.h"
#include "ns3/socket.h"

#define SLOW_WALK = 0.67;
#define NORMAL_WALK = 1.46;
#define SCURRYING_WALK = 1.8; //早歩き
#define BYCYCLE = 4;
#define CAR = 5;
#define FROMRECV 1 //データ受信から呼ばれた
#define FROMTIMER 2 //タイマー周期で呼ばれた

using json = nlohmann::json;
using String =std::string;


namespace ns3 {
namespace simple {

class Dijkstra_Info{
public:
	double dst;
	std::list<std::pair<double,double>> route;
	Dijkstra_Info(double  d,std::list<std::pair<double,double>> r){
		dst = d;
		route = r;
	};

};

// class Hop_Info{
// private:
// 	Ipv4Address m_address;
// 	uint32_t m_userId;
// };

class MyUser: public Node
{
private:
	uint32_t m_nodeType;
	uint32_t m_userId;
	Position m_sectionId;
	uint32_t m_seqNo;
	uint32_t m_status;
	uint32_t m_occupancy;
	uint32_t m_numTransfers;		// データ転送回数
	uint32_t m_volTransfers;		// データ転送量
	Time m_evacTime;			// 避難完了時間
	uint32_t m_exitDoor;			// 通過した出口ドア
	way_info m_currentway;
	std::map<std::pair<double,double>,way_node> m_graph;
	String m_exitnode;
	String m_attribute;
	// ?Ptr<Ipv4> m_ipv4;
//	uint32_t m_changeexit_dueto_blockedinfo;
//	uint32_t m_changeexit_dueto_exitinfo;
//	uint32_t m_changeroute_dueto_blockedinfo;
//	uint32_t m_changeroute_dueto_exitinfo;

	uint32_t m_changeroute;
	uint32_t m_changeexit;

	double total_distance;
	double arrival;

	std::set<String> m_blockednodeid;
	std::set<String> m_blockednodeid_buf; //一つ前の情報を保持する（自分が持っているブロックノード情報が更新されたかの確認用）
	String m_findblockednodeid;
	std::map<String, float> m_blockednodeid_and_time;
	std::map<String, float> m_blockednodeid_and_time_for_Log; //Logのためだけの記録用

	std::set<String> m_fullexitnodeid;
	String m_findfullexitnodeid;
	std::set<String> m_fullexitnodeid_buf;

	std::list<uint32_t> m_recvSeqNo;
	std::vector<uint32_t> m_userVector;
	std::map<Position, uint32_t> m_statusMap;
	std::map<Position, uint32_t> m_occMap;
	std::vector<Position> m_routeVector;
	std::vector<std::string> m_takenRouteVector;
	std::set<Ipv4Address> m_already_sent_address; //すでに送信済みの宛先アドレスを保管

	std::pair<double, double> m_CurrentCoordinate; //現在のユーザの緯度軽度（避難所のAPユーザのみ）

	Ptr<Socket> m_socket;
	std::list<Vector> m_nvct;
	std::list<std::pair<double,double>> m_shortest;
	std::list<std::pair<double,double>> m_passednode;

	std::list<way_node> m_wnlist_buf;
	std::list<way_node> m_wnlist_buf2;

	std::map<uint32_t, std::pair<std::pair<String, uint32_t>, uint32_t>> m_hop;

	// uint32_t m_hop_num;
//	std::map<String,int> m_passedway;

	bool isDijkstra;
	bool isMove;
	bool isWait;
	bool isExit;
	bool isFirst;
	bool isSecond;
	bool isClose;
	bool isUseexitinfo;
	bool changeExit;
	bool changeBlocked;
	bool changerouteforUAV;
	// bool alreadycountflag;

	uint32_t m_shelterID;
	uint32_t m_canexitnum;
	std::map<String, uint32_t> m_canexitnummap;

	std::map<uint32_t,std::pair<uint32_t,uint32_t>> m_exitnodeinfolist;
	std::pair<uint32_t,uint32_t> m_exitinfopair;
	std::map <String, std::pair<uint32_t, uint32_t>>m_aoi_exitinfo; //避難所ID、収容人数、時刻
	std::map <String, std::pair<uint32_t, uint32_t>>m_aoi_exitinfo_buf; //避難所ID、収容人数、時刻
	std::map<String, double> increase_rate; //避難所ID，増加率

	bool hello_flag;
	bool check_move_flag;
	bool quick_send_flag;
	bool while_quick_send_flag = false; //quickhelloを実行中を示すフラグ
	bool alreadycountflag; //避難完了ユーザを減算したことを記録するフラグ

	String target_node_buf = "first";

	double hello_send_time = 0; //Helloを送信した時間を記録

	bool already_quick_send_flag = false;
	uint32_t quick_hello_times;
	double starttime;

	double speed;

	double distance_to_exit;
	std::map<String, double> m_exit_distance_map; // 各ノードからの避難所までの距離（ノードの座標がキー，避難所までの最短距離を値として保存）

	double m_required_time;

	double m_generation_time; //避難所到着時間情報の生成時間

	// static std::map<String, uint32_t> moving_user_num; //各避難所ごとの現時点で目指しているユーザ数のマップ

	/////////////////////////////////////////////////////////////////////////////
	std::vector<std::string> m_patrolRoute;
	std::vector<std::string> m_old_patrolRoute;
    uint32_t m_currentPatrolIndex = 0;
	// double fspeed;
    // std::map<std::string, Waypoint> m_shelterCoordinates;
    std::map<std::string, std::map<std::string, double>> m_distances;

	std::pair<String,uint32_t> my_patrol_erea_and_start_time; //nodeid and patrol start time
	std::map<String,uint32_t> other_UAV_patrol_erea_and_id; //ereaid and UAVid(Key is ereaid(nodeid))
	std::map<String, uint32_t> patrol_ereaid_and_time;
public:

	MyUser ();
	~MyUser ();

	void SetWaitStatus (bool flag){isWait = false;}
	bool GetWaitStatus (void){return isWait;}
	void SetShelterID (uint32_t ShelterID){m_shelterID = ShelterID;}
	uint32_t GetShelterID (void){return m_shelterID;}
	void SetCanexitnum (uint32_t cnaexitnum){m_canexitnum = cnaexitnum;}
	uint32_t GetCanexitnum (void){return m_canexitnum;}
	void SetCanexitnummap(std::map<String, uint32_t> canexitnummap){m_canexitnummap = canexitnummap;}
	std::map<String, uint32_t> GetCanexitnummap(void){return m_canexitnummap;}
	void SetNodeType (uint32_t nodeType) {m_nodeType=nodeType;}
	uint32_t GetNodeType (void) {return m_nodeType;}
	void SetUserId (uint32_t userId) {m_userId=userId;}
	uint32_t GetUserId (void) {return m_userId;}
	void SetSectionId (Position pos) {m_sectionId=pos;}
	Position GetSectionId (void) {return m_sectionId;}
	Position GetSectionIdFromPosition (void);
	void SetStatus (uint32_t status) {m_status=status;}
	uint32_t GetStatus (void) {return m_status;}
	void SetSeqNo (uint32_t seqNo) {m_seqNo=seqNo;}
	uint32_t GetSeqNo (void) {return m_seqNo;}
	void SetNumTransfers (uint32_t numTransfers) {m_numTransfers=numTransfers;}
	uint32_t GetNumTransfers (void) {return m_numTransfers;}
	void SetVolTransfers (uint32_t volTransfers) {m_volTransfers=volTransfers;}
	uint32_t GetVolTransfers (void) {return m_volTransfers;}
	void SetEvacuationTime (Time time) {m_evacTime=time;}
	Time GetEvacuationTime (void) {return m_evacTime;}
	void SetExitDoor (uint32_t exitDoor) {m_exitDoor=exitDoor;}
	uint32_t GetExitDoor (void) {return m_exitDoor;}
	void SetCurrentWay (way_info way) {m_currentway=way;}
	way_info GetCurrentWay (void) {return m_currentway;}
	void SetGraph(std::map<std::pair<double,double>,way_node> graph){m_graph=graph;}
	std::map<std::pair<double,double>,way_node> GetGraph (void) {return m_graph;}
	void SetExitnode(String exitnode){m_exitnode=exitnode;}
	String GetExitnode (void) {return m_exitnode;} //ユーザが目指す避難所ノード
	void SetAttribute(String attribute){m_attribute=attribute;}
	String GetAttribute (void) {return m_attribute;}

	void SetCheckMoveflag(){check_move_flag = true;}
	bool GetCheckMoveflag(){return check_move_flag;}

	void SetQuickSendflag(bool flag){quick_send_flag = flag;}
	bool GetQuickSendflag(){return quick_send_flag;}

	void SetCloseflag(bool flag){isClose=flag;}
	bool GetCloseflag(void) {return isClose;}

	void SetUseexitinfoflag(bool flag){isUseexitinfo=flag;}
	bool GetUseexitinfoflag(void) {return isUseexitinfo;}

	bool GetExitflag(void){return isExit;}

	void SetAlreadyCountflag(void){alreadycountflag = true;}
	bool GetAlreadyCountflag(void){return alreadycountflag;}

	void SetAoiExitinfo(std::map <String, std::pair<uint32_t, uint32_t>>Aoi_Exitinfo){m_aoi_exitinfo = Aoi_Exitinfo;}
	std::map <String, std::pair<uint32_t, uint32_t>> GetAoiExitinfo(void){return m_aoi_exitinfo;}
	void EraseAoiExitinfo(String key){m_aoi_exitinfo.erase(key);}

	void SetAoiExitinfo_buf(std::map <String, std::pair<uint32_t, uint32_t>>Aoi_Exitinfo){m_aoi_exitinfo_buf = Aoi_Exitinfo;}
	std::map <String, std::pair<uint32_t, uint32_t>> GetAoiExitinfo_buf(void){return m_aoi_exitinfo_buf;}

	void SetTargetNodeBuf(String s){target_node_buf = s;}
	String GetTargetNodeBuf(void){return target_node_buf;}

	void SetHelloSendTime(double time){hello_send_time = time;}
	double GetHelloSendTime(void){return hello_send_time;}

	void SetAlreadyQuickSendFlag(bool flag){already_quick_send_flag = flag;}
	bool GetAlreadyQuickSendFlag(void){return already_quick_send_flag;}

	void SetWhileQuickHelloFlag(bool flag){while_quick_send_flag = flag;}
	bool GetWhileQuickHelloFlag(void){return while_quick_send_flag;}

	void InitQuickHelloTimes(void){quick_hello_times = 0;}
	void AddQuickHelloTimes(void){quick_hello_times++;}
	uint32_t GetQuickHelloTimes(void){return quick_hello_times;}

	// OK!!
	void InsertExitnodeInfoList(std::pair<uint32_t,uint32_t> targetnodeinfo){
		if(targetnodeinfo.first !=0 && m_exitnodeinfolist.at(targetnodeinfo.first).second > targetnodeinfo.second){
			std::pair<uint32_t,uint32_t> cap = m_exitnodeinfolist.at(targetnodeinfo.first);
			m_exitnodeinfolist.at(targetnodeinfo.first)=std::make_pair(cap.first,targetnodeinfo.second);
		}
	}
	// 問題はこっちかも
	void SetExitnodeInfoList (std::map<uint32_t,uint32_t>  targetnodelist){
		for(auto itr = targetnodelist.begin();itr!=targetnodelist.end();++itr){
			std::pair<uint32_t,uint32_t> it = *itr;
			if(m_exitnodeinfolist.at(it.first).second>it.second){
				m_exitnodeinfolist.at(it.first).second=it.second;
			}
		}
	}
	std::map<uint32_t,std::pair<uint32_t,uint32_t>> GetExitnodeInfoList (void) {return m_exitnodeinfolist;}
	void SetExitInfo (std::pair<uint32_t,uint32_t> exitinfopair){m_exitinfopair=exitinfopair;}
	std::pair<uint32_t,uint32_t> GetExitInfo(void) {return m_exitinfopair;}

//	void SetBlockedInfoflag(bool flag){changeBlocked=flag;}
//	void SetExitInfoflag(bool flag){changeExit=flag;}

//	void SetChangeRouteBlockedInfo(uint32_t changeroute){m_changeroute_dueto_blockedinfo=changeroute;}
//	uint32_t GetChangeRouteBlockedInfo (void) {return m_changeroute_dueto_blockedinfo;}
//	void SetChangeExitDuetoBlockedInfo(uint32_t changeexit){m_changeexit_dueto_blockedinfo=changeexit;}
//	uint32_t GetChangeExitDuetoBlockedInfo (void) {return m_changeexit_dueto_blockedinfo;}


	void SetStatusMap (std::map<Position, uint32_t> statusMap) {m_statusMap=statusMap;}
	std::map<Position, uint32_t> GetStatusMap (void) {return m_statusMap;}
	void SetOccupancyMap (std::map<Position, uint32_t> occMap) {m_occMap=occMap;}
	std::map<Position, uint32_t> GetOccupancyMap (void) {return m_occMap;}
	void SetRouteVector (std::vector<Position> routeVec) {m_routeVector=routeVec;}
	std::vector<Position> GetRouteVector (void) {return m_routeVector;}

	void SetBlockedNodeIDBuf(String id){m_blockednodeid_buf.insert(id);}
	std::set<String> GetBlockedNodeIDBuf(void){return m_blockednodeid_buf;}

	void SetAlreadySentAddress(Ipv4Address already_sent_address){m_already_sent_address.insert(already_sent_address);} //すでに送信済みのアドレスをセット
	std::set<Ipv4Address> GetAlreadySentAddressSet(void){return m_already_sent_address;} //すでに送信済みのアドレスのセットを返す
	void ClearAlreadySentAddress(void){m_already_sent_address.clear();} //すでに送信済みのアドレスのセットをクリアする

	void SetBlockedNodeID(String id){m_blockednodeid.insert(id);} //set型のリスト
	void FindBlockedNodeID(String id){m_findblockednodeid=id;} //String型
	void SetBlockedNodeID_and_time(String id ,float time){
		m_blockednodeid_and_time.erase(id); //上書きされないから一回消す。
		m_blockednodeid_and_time.insert(std::make_pair(id, time));
	} //ブロックノードIDと時間
	void SetBlockedNodeID_and_time2(std::map<String, float> data){m_blockednodeid_and_time = data;}
	std::set<String> GetBlockedNodeIDSet(void){return m_blockednodeid;}
	String GetFindBlockedNodeID(void){return m_findblockednodeid;}
	std::map<String, float> GetBlockedNodeID_and_time(){return m_blockednodeid_and_time;} //ブロックノードIDと時間

	void SetBlockedNodeID_and_time_for_Log(String id ,float time){
		m_blockednodeid_and_time_for_Log.erase(id); //上書きされないから一回消す。
		m_blockednodeid_and_time_for_Log.insert(std::make_pair(id, time));
	}
	void SetBlockedNodeID_and_time_for_Log2(std::map<String, float> data){
		for(auto a : data){
			SetBlockedNodeID_and_time_for_Log(a.first, a.second);
		}
	}
	std::map<String, float> GetBlockedNodeID_and_time_for_Log(){return m_blockednodeid_and_time_for_Log;} //ブロックノードIDと時間

	void SetFullexitNodeID(String id){m_fullexitnodeid.insert(id);}
	void FindFullexitNodeID(String id){m_findfullexitnodeid=id;}
	void SetFullexitNodeID_Buf(std::set<String> fullexitnodeidset){m_fullexitnodeid_buf = fullexitnodeidset;}
	std::set<String> GetFullexitNodeIDSet(void){return m_fullexitnodeid;}
	String GetFindFullexitNodeID(void){return m_findfullexitnodeid;}
	std::set<String> GetFullexitNodeIDSet_Buf(void){return m_fullexitnodeid_buf;}


	void RestoreGraphInfo(String id);


	void SetShortestroute  (std::list<std::pair<double,double>> shortest) {m_shortest=shortest;}
	std::list<std::pair<double,double>> GetShortestroute (void) {return m_shortest;}
	void SetPassedroute  (std::list<std::pair<double,double>> passed) {m_passednode=passed;}
	std::list<std::pair<double,double>> GetPassedroute (void) {return m_passednode;}
//	void SetPassedWay(way_info wi);
//	std::map<String,int> GetPassedWay (void){return m_passedway;}


	void SetUserVector (std::vector<uint32_t> userVector) {m_userVector=userVector;}
	std::vector<uint32_t> GetUserVector (void) {return m_userVector;}
	void SetOccupancy (uint32_t occupancy) {m_occupancy=occupancy;}
	uint32_t GetOccupancy (void) {return m_occupancy;}

	void SetSocket (Ptr<Socket> socket) {m_socket=socket;}

	void PushRecvSeqNo(uint32_t seqNo){m_recvSeqNo.push_back(seqNo);}
	void PopRecvSeqNo(){m_recvSeqNo.pop_front();}
	std::list<uint32_t> GetRecvSeqNo(){return m_recvSeqNo;}

	void SetMyUser (void);
	void PrintMyUser (void);

	void SetCurrentCoordinate(std::pair<double, double> CurrentCoordinate){m_CurrentCoordinate = CurrentCoordinate;}
	std::pair<double, double> GetCurrentCoordinate (void){return m_CurrentCoordinate;}
	void Setarrival(double m_arrival){arrival=m_arrival;}
	double Getarrival(void){return arrival;}

	void HandleUserMovement (void);
	Vector SelectRandomNextLine (void);
	Vector SelectRandomNextPosition (void);
	void DijkstraRouteCalculationforGeographicInfo (way_node no);
	void DijkstraRouteCalculation (void);
	void CompleteEvacuation (String ID);
	void SocketClose(void);


	Vector GetWayNextVector();
	void TaskOfCrossing(way_node ni);
	void UpdateGraphInfo(way_node ni);
	void UpdateGraphInfoReceivedPacket(String id);
	double CalculateDistance_from_coordinate(std::pair<double, double> start, std::pair<double, double> end);


	void OutputText_CloseUser (Time now);

	bool Gethelloflag(void){return hello_flag;}
	void Sethelloflag(bool flag){hello_flag = flag;}

//****************************************************************
	void PushTakenRouteVector(std::string sectionId){m_takenRouteVector.push_back(sectionId);}
	void PopTakenRouteVector(){m_takenRouteVector.pop_back();}
	std::vector<std::string> GetTakenRouteVector(){return m_takenRouteVector;}
	void CheckAoI();

	uint32_t GetSumEvacuationTime();
	uint32_t GetWorstEvacuationTime();
	uint32_t GetNumLoops();

	void ShortestRouteCalculation(uint32_t nodeId);
	void OutputText_NI(Ptr<Node> node, std::string color);

	Timer fdTimer;
	Timer humTimer;
	Timer ceTimer;
	Timer scTimer;
	Timer AoITimer;
	std::map<uint32_t, ns3::Timer> hopTimers;
	void FirstDijkstraTimer (Time delay);
	void HandleUserMovementTimer (Time delay);
	void AddUserMovementTimer(Time delay, std::vector<uint32_t> vec);
	void CompleteEvacuationTimer (Time delay);
	void SocketCloseTimer (Time delay);
	void AoICheckTimer(Time delay);
	void EraseHopDataTimer(double delay, uint32_t key);

	void EraseHopData(uint32_t key);

	void SetEraseHopTimer();

	void SetSpeed(double s){speed = s;}
	double GetSpeed(){return speed;}

	void SetHop(uint32_t id, std::pair<std::pair<String, uint32_t>, uint32_t> p){m_hop.insert(std::make_pair(id,p));}
	// 要素があれば削除してinsertする（上書き），ない場合はそのままinsertする（ただの追加）
	void UpdateHop(uint32_t id, std::pair<std::pair<String, uint32_t>, uint32_t> p){
		try{
			m_hop.at(id);
			m_hop.erase(id);
			m_hop.insert(std::make_pair(id,p));
		}catch(const std::out_of_range& e){
			m_hop.insert(std::make_pair(id,p));
		}
	}
	void SetHop(std::map<uint32_t, std::pair<std::pair<String, uint32_t>, uint32_t>> ad_set){m_hop = ad_set;} //置き換え
	void ClearHop(){m_hop.clear();}
	std::map<uint32_t, std::pair<std::pair<String, uint32_t>, uint32_t>> GetHop(){return m_hop;}
	void SetIncreaseRate(String id, double rate){
		increase_rate.emplace(std::make_pair(id, rate));
	}
	void EraseHop(uint32_t key){m_hop.erase(key);}
	std::map<String, double> GetIncreaseRate(){return increase_rate;}

	double GetRequiredTime(void){return m_required_time;}
	double GetGenerationTime(void){return m_generation_time;}


	////////////////////////////////////////////////////////////////////////

	void SetPatrolRoute(std::vector<std::string>route){m_patrolRoute = route;}
	void SetOldPatrolRoute(std::vector<std::string>route){m_old_patrolRoute = route;}
	std::vector<std::string> GetPatrolRoute(){return m_patrolRoute;}
	Vector GetNextXY();
	void HandleFerryNodeMovement (void);
	void HandleFerryMovementTimer(Time delay);
	void GetExitInfoforFerry(String id);
	void SetCurrentPatrolIndex(uint32_t n){m_currentPatrolIndex = n;}
	void HandleFerryNodeMovementAroundBlock (void);
	void HandleFerryMovementAroundBlockTimer(Time delay);
	void ChangePatrolRoute(uint32_t from);
	void SetMyPatrolErea_and_StartTime(std::pair<String,uint32_t> data){my_patrol_erea_and_start_time = data;}
	void SetOtherUAVPatrolErea_and_Id(std::map<String,uint32_t> data){other_UAV_patrol_erea_and_id = data;}
	void SetPatrolErea_and_Time(std::map<String,uint32_t> data){patrol_ereaid_and_time = data;}
	std::pair<String,uint32_t> GetMyPatrolErea_and_StartTime(){return my_patrol_erea_and_start_time;}
	std::map<String,uint32_t> GetOtherUAVPatrolErea_and_Id(){return other_UAV_patrol_erea_and_id;}
	std::map<String,uint32_t> GetPatrolErea_and_Time(){return patrol_ereaid_and_time;}
	Timer ferryTimer;
	Timer aroundblockferryTimer;

};

// class PatrolFerry : public Node{
// private:
//     std::vector<std::string> m_patrolRoute;
//     uint32_t m_currentPatrolIndex = 0;
// 	// double fspeed;

//     // std::map<std::string, Waypoint> m_shelterCoordinates;
//     std::map<std::string, std::map<std::string, double>> m_distances;

// public:
// 	PatrolFerry() {
        
//     }
// 	void SetPatrolRoute(std::vector<std::string>route){m_patrolRoute = route;}
// 	std::vector<std::string> GetPatrolRoute(){return m_patrolRoute;}
// 	Vector GetNextXY();
// 	void HandleFerryNodeMovement (void);
// 	void HandleFerryMovementTimer(Time delay);
// 	Timer ferryTimer;
// };

}}

#endif /* SIMPLE_USER_H_ */
