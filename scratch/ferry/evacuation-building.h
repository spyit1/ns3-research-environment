/*
 * simple-building.h
 *
 *  Created on: 2019/12/25
 *      Author: fujinaka
 */

#ifndef SIMPLE_BUILDING_H_
#define SIMPLE_BUILDING_H_

#include <stdint.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <climits>
#include <random>

#include "ns3/timer.h"
#include "json_data.h"
// #include "evacuation-user.h"


namespace ns3 {
namespace simple {

const int UNKNOWN = 1;
const int SAFE = 2;
const int EXIT = 3000;
const int BLOCKED = 4;

const int EVC_USER = 1;
const int EVC_AP = 2;
const int ADD_EVC_USER = 3;
const int FERRY_NODE = 4;

const int LOCAL_FLOODING = 1;
const int GLOBAL_FLOODING = 2;

const int INF = INT_MAX;

struct Position {
	uint32_t x;
	uint32_t y;
	uint32_t z;
	bool operator== (const Position& rhs) {return (x==rhs.x && y==rhs.y && z==rhs.z);}
};
inline bool operator== (const Position& a, const Position& b) {return (a.x==b.y && a.y==b.y && a.z==b.z);}
inline bool operator< (const Position& a, const Position& b) {return std::tie(a.x, a.y, a.z) < std::tie(b.x, b.y, b.z);}
std::ostream& operator<< (std::ostream& os, const Position& pos);

class MyBuilding
{
private:
	// simulation parameter
	// building model
	static double fieldWidth;			// フィールド横幅 [m]
	static double fieldHight;			// フィールド縦幅 [m]
	static const uint32_t numSections;		// セクション数
	static const uint32_t numSectionX;		// x軸セクション数
	static const uint32_t numSectionY;		// y軸セクション数
	static const uint32_t numSectionZ;		// z軸セクション数
	static const double sectionWidth;		// セクション横幅 [m]
	static const double sectionHight;		// セクション縦幅 [m]
	static const double PenetrationRate;	// 出口ドアのユーザ進入率 [persons/second]
	static uint32_t numUsers;			// ユーザ数
	static uint32_t addUsers;			//added user
	static uint32_t numferrys;			//フェリーノードの数
	static double userSpeed;			// ユーザの移動速度
	static double ferrySpeed;			//フェリーノードの移動速度
	static double HelloInterval;			// 情報共有間隔
	// static double ShelterHelloInterval;
	static double ShelterDataInterval;
	static double UpdateMapinfoInterval;	//マップ更新間隔
	static bool blockflag;					//ブロックの有効、無効
	static bool blockeraseflag;				//ブロック解除の有効、無効
	static bool randomblockeraseflag;		//ブロックノードをランダムに一つづつ解除するかどうか
	static bool setexitAutoflag;
	static bool setexitmanualflag;
	static uint32_t neary;					//避難所を自動で設定するときの入り口として設定するノードの近さ
	static bool randomuserSpeedflag;
	static double MaxSpeed;
	static double MinSpeed;
	static String field_name;
	static bool useexitinfo;
	static std::set<String> blockset_buf;
	static uint32_t Disaster_Type;			//災害の種類
	static bool UseDisasterType;
	static uint32_t AoI; //QuickSendをするときのAoIのしきい値
	static bool QuickSend; //AoIによりHello間隔を短くするかどうか
	static double QuickHelloInterval; //QuickSendをするときのhello間隔
	static double AddUserInterval; //ユーザを追加する間隔
	static double moving_user_check_interval; //ユーザ数をチェックする間隔
	static uint32_t EraseAoI; //情報を破棄するAoI値
	static uint32_t quick_hello_times; //QuickSendをする回数
	static String ferry_start; //フェリーノードの出発地点のノードID
	static double ferry_hellointerval; //フェリーノードのHello間隔
	static bool auto_ferry_num; //自動で避難所の数分ドローンを用意する


	//jsonPath
	static std::string SimulationFieldMap;
	static std::string hinanjo_json;
	// OpenStreetMap info model
	static std::map<String,uint32_t> exit_nodes;
	static std::map<String,uint32_t> first_exit_info;
	static std::list<String> blocked_nodes;
	static std::list<String> blocked_nodes2;
	static std::list<String> erase_nodes;
	static std::list<String> rescue_nodes;
	static std::list<String> erase_rescue_nodes;
	static std::list<String> congestion_nodes;
	static std::list<String> erase_congestion_nodes;

	// building model
	static std::vector<std::vector<std::string>> sectionLink;
	static std::list<Position> exit;
	static std::list<Position> blocked;
	static std::map<std::pair<Position, Position>, uint32_t> exitDoor;		// 出口ドア <{front, exit}, doorId>

	//
	static std::map<uint32_t, std::queue<uint32_t>> holdUserId;	// 出口ドアユーザIDキュー <出口ドアID, ユーザIDキュー>
	static std::map<uint32_t, Time> waitTime;

	// result
	static std::vector<uint32_t> userIdCompletedEvacuation;			// 避難完了ユーザID
	static std::map<uint32_t, std::list<uint32_t>> userIdPassedExitDoor;	// 各出口ドアを通過したユーザのID <doorId, userIdList>

	//exit info
	static std::map<std::pair<double, double>, std::list<bool>> exit_info;
	static std::set<uint32_t> number;
	static uint32_t num;

	static std::vector<uint32_t> vect;

	static std::map<String, uint32_t> moving_user_num; //各避難所ごとの現時点で目指しているユーザ数のマップ

	static uint32_t send_hello_num;
	static uint32_t send_hello_bytes;
	static uint32_t recv_hello_num;
	static uint32_t recv_hello_bytes;
	static uint32_t send_ack_hello_num;
	static uint32_t recv_ack_hello_num;

	static uint32_t seed;
	static uint32_t run;
	static uint32_t count;

	static std::map<String, double> max_distance_to_exit;

	static bool set_congestion_flag;
	static bool erase_congestion_flag;
	static bool set_rescue_flag;
	static bool erase_rescue_flag;

	static bool send_hop_flag;

	static std::map<std::string, std::map<std::string, double>> exit_node_distances;
	static std::vector<std::string> m_patrolRoute; //フェリーノード巡回ルート
	
	// static std::map<String,Ptr<MyUser>> exitlist;

public:
	static double GetFieldWidth (void) {return fieldWidth;}
	static double GetFieldHight (void) {return fieldHight;}
	static uint32_t GetNumSections (void) {return numSections;}
	static double GetSectionWidth (void) {return sectionWidth;}
	static double GetSectionHight (void) {return sectionHight;}
	static uint32_t GetNumSectionX (void) {return numSectionX;}
	static uint32_t GetNumSectionY (void) {return numSectionY;}
	static uint32_t GetNumSectionZ (void) {return numSectionZ;}
	static double GetPenetrationRate (void) {return PenetrationRate;}
	static uint32_t GetNumUsers (void) {return numUsers;}
	static uint32_t GetAddUsersNum(void){return addUsers;}
	static uint32_t GetNumFerrys(void){return numferrys;}
	static double GetUserSpeed (void) {return userSpeed;}
	static double GetFerrySpeed (void) {return ferrySpeed;}
	static double GetHelloInterval (void) {return HelloInterval;}
	static double GetShelterDataInterval (void){return ShelterDataInterval;}
	static double GetUpdateMapinfoInterval (void) {return UpdateMapinfoInterval;}
	static std::set<String> GetBlockedNodesbuf_set (void){return blockset_buf;}
	static bool GetBlockFlag (void){return blockflag;}
	static bool GetBlockeraseFlag (void){return blockeraseflag;}
	static bool GetRandomBlockeraseFlag (void){return randomblockeraseflag;}
	static bool GetExitmanualflag(void){return setexitmanualflag;}
	static bool GetExitAutoflag(void){return setexitAutoflag;}
	static void SetBlockedNodesbuf_set (std::set<String> buf){blockset_buf = buf;}
	static std::list<String> GetEraseNodes(){return erase_nodes;}
	static uint32_t Getneary(void){return neary;}
	static std::string GetSimulationFieldPath(void){return SimulationFieldMap;} //使ってない
	static std::string GethinanjojsonPath(void){return hinanjo_json;}
	static bool GetRandomUserSpeedFlag(void){return randomuserSpeedflag;}
	static double GetMaxSpeed(void){return MaxSpeed;}
	static double GetMinSpeed(void){return MinSpeed;}
	static String GetFieldName(void){return field_name;}
	static bool GetUseExitInfoFlag(void){return useexitinfo;}
	static void SetUseExitInfoFlag(bool flag){useexitinfo = flag;}
	static uint32_t GetDisasterType(void){return Disaster_Type;}
	static bool GetUseDisasterTypeFlag(void){return UseDisasterType;}

	static std::map<String, uint32_t> GetExitNodes(void){return exit_nodes;}
	//static void SetExitNodes(String id, uint32_t upper_user){exit_nodes.insert(std::make_pair(id,upper_user));}
	static void SetExitNodes(); //2023/10/17
	static void SetExitNodesAuto(std::string nodeID, uint32_t exitnum);
	static void ExitNodesPassedUserSet(String nodeid) {exit_nodes.at(nodeid)--;} //避難所の収容可能人数を更新（ユーザが避難完了したら1減らす）
	static std::list<String> GetBlockedNodes(void){return blocked_nodes;}
	static void SetBlockedNodes(uint32_t update_num); //2023/10/10
	static void EraseBlockedNodes(uint32_t update_num);
	static std::vector<std::vector<std::string>> GetSectionLink (void) {return sectionLink;}
	static std::list<Position> GetExitSection (void) {return exit;}
	static std::list<Position> GetBlockedSection (void) {return blocked;}
	static std::map<std::pair<Position, Position>, uint32_t> GetExitDoor (void) {return exitDoor;}
	static std::map<String, uint32_t> Get_first_exit_info(void){return first_exit_info;}
	static std::list<String> GetBlockedNodes2(void){return blocked_nodes2;} //初期化用
	static void SetBlockedNodes2(); //初期化用

	static void PopHoldUserId (uint32_t doorId) {holdUserId[doorId].pop();}
	static Time CalculationWaitTime (uint32_t userId, Position front, Position exit);
	static void ProcessUserOut (uint32_t doorId);

	static std::vector<uint32_t> GetUserIdCompEvc (void) {return userIdCompletedEvacuation;}
	static void PushUserIdCompEvac (uint32_t userId) {userIdCompletedEvacuation.push_back(userId);}
	static std::map<uint32_t, std::list<uint32_t>> GetUserIdPassedExitDoor (void) {return userIdPassedExitDoor;}
	static void PushUserIdPassedExitDoor (uint32_t doorId, uint32_t userId) {userIdPassedExitDoor.at(doorId).push_back(userId);}

	static void SettingLoad();
	static void Write_Logfile_for_simulation_environment();

	static void SetExitInfo(std::pair<double, double> zahyou, std::list<bool> info){exit_info.insert(std::make_pair(zahyou,info));}
	static std::map<std::pair<double, double>, std::list<bool>> GetExitInfo (void){return exit_info;}
	static void SetRandomNum();
	static std::vector<uint32_t> GetRandomNum(){return vect;}

	static void SetMovingUserNum (std::map<String, uint32_t> map){moving_user_num = map;}
	static std::map<String, uint32_t> GetMovingUserNum(){return moving_user_num;}

	// static bool GetQuickSendflag(){return QuickSend;} //使ってない使うな！！
	static uint32_t GetAoIthreshold(){return AoI;}
	static double GetQuickHelloInterval(){return QuickHelloInterval;}
	static uint32_t GetEraseAoI(){return EraseAoI;}
	static void SetEraseAoI(uint32_t eraseAoI){EraseAoI = eraseAoI;}

	static double GetMovingUserNumInterval(){return moving_user_check_interval;}

	static uint32_t GetQuickHelloTimes(){return quick_hello_times;}
	static String GetFerryStart(){return ferry_start;}

	static void PlusSendHelloNum(){send_hello_num++;}
	static void PlusRecvHelloNum(){recv_hello_num++;}
	static void PlusSendHelloBytes(uint32_t bytes){send_hello_bytes = send_hello_bytes + bytes;}
	static void PlusRecvHelloBytes(uint32_t bytes){recv_hello_bytes = recv_hello_bytes + bytes;}
	static void PlusSendAckHelloNum(){send_ack_hello_num++;}
	static void PlusRecvAckHelloNum(){recv_ack_hello_num++;}

	static uint32_t GetSendHelloNum(){return send_hello_num;}
	static uint32_t GetRecvHelloNum(){return recv_hello_num;}
	static uint32_t GetSendHelloBytes(){return send_hello_bytes;}
	static uint32_t GetRecvHelloBytes(){return recv_hello_bytes;}
	static uint32_t GetSendAckHelloNum(){return send_ack_hello_num;}
	static uint32_t GetRecvAckHelloNum(){return recv_ack_hello_num;}

	static void SetSeed(uint32_t s){seed = s;}
	static uint32_t GetSeed(){return seed;}
	static void SetRun(uint32_t s){run = s;}
	static uint32_t GetRun(){return run;}

	static std::map<String, double> GetMaxDistanceToExit(){return max_distance_to_exit;}
	static void SetMaxDistanceToExit(String id, double dis){
		if(max_distance_to_exit.at(id) < dis){
			max_distance_to_exit.erase(id);
			max_distance_to_exit.insert(std::make_pair(id, dis));
		}
	}
	static uint32_t GetCount(){return count;}
	static void PlusCount(){count++;}
	static void SetRescueNode(uint32_t update_num);
	static void EraseRescueNode(uint32_t update_num);
	static void SetCongestionNode(uint32_t update_num);
	static void EraseCongestionNode(uint32_t update_num);
	static std::list<String> GetRescueNode(){return rescue_nodes;}
	static std::list<String> GetEraseRescueNode(){return erase_rescue_nodes;}
	static std::list<String> GetCongestionNode(){return congestion_nodes;}
	static std::list<String> GetEraseCongestionNode(){return erase_congestion_nodes;}
	static bool GetRescueFlag(){return set_rescue_flag;}
	static bool GetEraseRescueFlag(){return erase_rescue_flag;}
	static bool GetCongestionFlag(){return set_congestion_flag;}
	static bool GetEraseCongestionFlag(){return erase_congestion_flag;}	
	static bool GetSendHopFlag(){return send_hop_flag;}
	static void SetSendHopFlag(String mode){
		if(mode == "ON"){
			send_hop_flag = true;
		}else{
			send_hop_flag = false;
		}
	}

	static void SetExitNodeDistance();
	static  std::map<std::string, std::map<std::string, double>> GetExitNodeDistance(){return exit_node_distances;}
	static void CalculatePatrolRoute();
	static std::vector<std::string> GetPatrolRoute(){return m_patrolRoute;}
	static double GetFerryHelloInterval(){return ferry_hellointerval;}
	static bool GetAutoFerryNumFlag(){return auto_ferry_num;}

	// static void SetExitList(std::pair<String, Ptr<MyUser>> pair){exitlist.emplace(pair);}
	// static std::map<String, Ptr<MyUser>> GetExitList(void){return exitlist;}

};

}}

#endif /* SIMPLE_BUILDING_H_ */
