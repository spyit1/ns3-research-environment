/*
 * simple-user.cc
 *
 *  Created on: 2019/12/25
 *      Author: fujinaka
 */

#include "evacuation-user.h"
//#include "simple-protocol.h"
#include "DTN-helper.h"
#include "write_log.h"
//#include "json_data.h"
//#include "test.cc"

//ns3::simple::JSON_Data m_jsondata=ns3::simple::JSON_Data();

#define THRESHOLD1 0.7
#define THRESHOLD2 1000
#define THRESHOLD3 1.0
//std::mt19937_64 mt64(0);


namespace ns3 {
namespace simple {
// double Max = MyBuilding::GetMaxSpeed();
// double Min = MyBuilding::GetMinSpeed();
std::map<String, uint32_t> info = MyBuilding::GetExitNodes();
// std::map<String, uint32_t> MyUser::moving_user_num; //各避難所ごとに現在動いているユーザ数のmap
MyUser::MyUser ():
		m_nodeType (EVC_USER),
		m_userId (GetId()),
		m_sectionId {0, 0, 0},
		m_seqNo (0),
		m_status (UNKNOWN),
		m_numTransfers (0),
		m_volTransfers (0),
		m_evacTime (0),
		m_exitDoor (0),
//		m_changeexit_dueto_blockedinfo(0),
//		m_changeexit_dueto_exitinfo(0),
//		m_changeroute_dueto_blockedinfo(0),
//		m_changeroute_dueto_exitinfo(0),
		m_changeroute(0),
		m_changeexit(0),
		isDijkstra (true),
		isMove (true),
		isWait (true),
		isExit (false),
		isFirst (true),
		isSecond (true),
		isClose (false),
		changeExit(false),
		changeBlocked(false),
		changerouteforUAV(false),
		m_canexitnummap(info),
		hello_flag(false),
		check_move_flag(false),
		quick_send_flag(false),
		alreadycountflag(false),
		quick_hello_times(0),
		starttime(0.0)
{
	std::vector<std::vector<MySection>>* mySection = &GetMySection();

	// initialize the status and occupancy of each section
	for (uint32_t z=0; z<MyBuilding::GetNumSectionZ(); z++) {
	for (uint32_t y=0; y<MyBuilding::GetNumSectionY(); y++) {
	for (uint32_t x=0; x<MyBuilding::GetNumSectionX(); x++) {
		uint32_t status = (*mySection).at(y).at(x).GetStatus();
		m_statusMap.insert(std::make_pair(Position{x, y, z}, status));
		switch (status) {
			case UNKNOWN:
				m_occMap.insert(std::make_pair(Position{x, y, z}, MyBuilding::GetNumUsers()));
				break;
			case SAFE:
				m_occMap.insert(std::make_pair(Position{x, y, z}, 0));
				break;
			case EXIT:
				m_occMap.insert(std::make_pair(Position{x, y, z}, 0));
				break;
			case BLOCKED:
				m_occMap.insert(std::make_pair(Position{x, y, z}, INF));
				break;
			default:
				m_occMap.insert(std::make_pair(Position{x, y, z}, INF));
				break;
		}
	}}}

	m_exitinfopair=std::make_pair(0,0);
	//MyBuilding::SetExitNodes();
	std::map<String, uint32_t> exitinfo = MyBuilding::GetExitNodes();

	std::map <String, std::pair<uint32_t, uint32_t>> shelterinfo;
	std::map <String, uint32_t> moving_user_num;
	//初期化
	for(auto itr = exitinfo.begin();itr!=exitinfo.end();++itr){
		std::pair<String, uint32_t> it = *itr;
		std::pair<uint32_t,uint32_t> cap = std::make_pair(it.second,it.second);
		//std::cout << cap.first <<","<<cap.second<<std::endl;
		m_exitnodeinfolist.insert(std::make_pair(JSON_Data::NodeIDstoi(it.first),cap));
		std::pair<uint32_t, uint32_t> pair1 = std::make_pair(cap.second, 0);
		// std::cout<<it.first<<std::endl;
		shelterinfo.insert(std::make_pair(it.first, pair1));
		// shelterinfo.at("node/10254");
		uint32_t num = 0;
		moving_user_num.insert(std::make_pair(it.first, num));
		MyBuilding::SetMovingUserNum(moving_user_num); //初期化
		SetAoiExitinfo(shelterinfo);
	}

	std::list<String> block = MyBuilding::GetBlockedNodes2();
	for(auto a : block){
		// std::cout<<"set"<<std::endl;
		std::map<String, float> id_time;
		SetBlockedNodeID_and_time_for_Log(a,0);
	}

}

MyUser::~MyUser ()
{
	// ..
}

void MyUser::HandleUserMovement ()
{
// std::cout<<"HandleUserMovement"<<std::endl;


	if(!isMove) isExit = true;
	if(isWait) isWait = false;
	if(RoutingProtocol::GetUseEraseInfoflag() == true){
		CheckAoI();
	}

	Ptr<MobilityModel> obj = GetObject<MobilityModel>();

	double posx = obj->GetPosition().x;
	double posy = obj->GetPosition().y;
	double posz = obj->GetPosition().z;
	Vector vct(posx, posy, posz);
	String targetnodeid;


	if(!m_shortest.empty()){ //リストが空じゃないとき
		targetnodeid=JSON_Data::get_waynodes().at(m_shortest.back()).GetNodeID();
		decltype(m_fullexitnodeid)::iterator it = m_fullexitnodeid.find(targetnodeid);
		if((!m_fullexitnodeid.empty()&&it != m_fullexitnodeid.end())){
			isDijkstra=true;
		}else{
			std::pair<uint32_t,uint32_t> it2 = m_exitnodeinfolist.at(JSON_Data::NodeIDstoi(targetnodeid));
			double threshold = (double)it2.second/(double)it2.first;
			if(threshold<THRESHOLD1){
				isDijkstra=true;
			}
		}
	}
	String exitnodeid; //現在いるノード
	std::list<way_node> wnlist=m_currentway.GetNodes();

	exitnodeid=JSON_Data::GetIDfromCoordinate(wnlist.front().GetCoordinate()); //現在いるノード

	// try{
	// 	double increse = increase_rate.at(exitnodeid); //毎秒避難者が何人増えていくか
	// 	String nodeid = JSON_Data::GetIDfromCoordinate(m_currentway.GetNodes().front().GetCoordinate());
	// 	double distance = m_exit_distance_map.at(nodeid); //現在のノードから避難所までの距離
	// 	double required_time = distance/speed; //避難所に到着するまでの所要時間
	// 	double expected_num_user = increse * required_time; //到着するまでに何人増えているか
	// 	std::cout<<"-------------------------------------------------------"<<std::endl;
	// 	std::cout<<m_userId + 1<<"----目標としている避難所ID："<<exitnodeid<<std::endl;
	// 	std::cout<<"増加率："<<increse<<std::endl;
	// 	std::cout<<"移動速度："<<speed<<std::endl;
	// 	std::cout<<"避難所までの距離："<<distance<<std::endl;
	// 	std::cout<<"到着までにかかる時間："<<required_time<<std::endl;
	// 	uint32_t remaining_capacity = MyBuilding::GetExitNodes().at(exitnodeid);
	// 	std::cout<<"現在の収容可能人数："<<remaining_capacity<<std::endl;
	// 	std::cout<<"到着までの予想増加人数："<<expected_num_user<<std::endl;
	// 	uint32_t remaining_capacity_arrival = remaining_capacity - expected_num_user; //到着時の予想残り収容可能人数
	// 	std::cout<<"到着時の予想収容可能人数："<<remaining_capacity_arrival<<std::endl;
	// 	std::cout<<"-------------------------------------------------------"<<std::endl;
	// 	if(remaining_capacity_arrival < 1){
	// 		std::cout<<"到着時には避難所が満員になる予想です！！！！"<<std::endl;
	// 		MyBuilding::PlusCount();
	// 	}
	// }catch(std::out_of_range& oor){

	// }


	try{
		if(MyBuilding::GetExitNodes().at(exitnodeid)==0) { //避難所の避難可能人数が０のとき＝つまり避難所がいっぱいの時
			std::cout<<"満員な避難所："<< exitnodeid <<std::endl;
			FindFullexitNodeID(exitnodeid); //いっぱいな避難所を記録
			SetFullexitNodeID(exitnodeid); //いっぱいな避難所を記録
			// SetExitInfoflag(true);
			isDijkstra=true;
		}else if(MyBuilding::GetExitNodes().at(exitnodeid)!=0 && !m_shortest.empty()){
			isMove = false;
			isExit = true;
			std::cout<<"交差点以外の避難所ノードに避難しました。"<<std::endl;
			CompleteEvacuation(exitnodeid);
			return;
		}
	}	catch(std::out_of_range& oor){

	}


	if (isDijkstra) {
		if(m_currentway.GetNodes().front().GetCrossingflag()==true){
			DijkstraRouteCalculationforGeographicInfo(m_currentway.GetNodes().front());
			isDijkstra = false;
		}
	}

	if(m_shortest.empty() && !isDijkstra){ //避難完了を記録する
		isMove=false;
		isExit=true;
	}
	
	//get your current location(random)
	if(isMove){
		Vector nvct = GetWayNextVector();
		if(!isMove) return;
		// std::cout<<"nextVector:"<<nvct<<std::endl; //目標地点までのベクトル
		// std::cout<<"currentVector:"<<vct<<std::endl; //現在の位置
		double dist = CalculateDistance(vct, nvct);
		// std::cout<<dist<<std::endl;
		total_distance += dist;
		// std::cout<<"dist:"<<dist<<std::endl;
//		m_currentway.Print();
		double now = Simulator::Now().GetDouble() / 1000000000; //現在の時間を取得
		// std::mt19937_64 mt64(GetUserId()); //乱数のもとにする整数型の値はユーザIDとする
		if(MyBuilding::GetRandomUserSpeedFlag()){
			std::mt19937_64 mt64(GetUserId());
			//std::normal_distribution<double> uni(1.6, 1); //正規分布、中央値が最瀕値となる
			// std::cout<<MyBuilding::GetMinSpeed()<<", "<<MyBuilding::GetMaxSpeed()<<std::endl;
			std::uniform_real_distribution<double> uni(MyBuilding::GetMinSpeed(), MyBuilding::GetMaxSpeed()); //一様分布
			speed = uni(mt64);
			if(isFirst){
				char filename[1000];
				std::ostringstream oss_run;
				oss_run << RngSeedManager::GetRun();
				String mode="ON";
				if(RoutingProtocol::GetMode()){
					mode="ON";
				}else{
					mode="OFF";
				}

				String quickhello="QuickHello_ON";
				if(RoutingProtocol::GetUseQuickHelloflag()){
					quickhello="QuickHello_ON";
				}else{
					quickhello="QuickHello_OFF";
				}

				String eraseblock = "EraseBlock_ON";
				if(RoutingProtocol::GetUseEraseInfoflag() == true){
					eraseblock="EraseBlock_ON";
				}else{
					eraseblock="EraseBlock_OFF";
				}
				std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello +"/" +eraseblock+ "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Info/Speed/UserSpeed_"  +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
				str.copy(filename,str.size());
				filename[str.size()] = '\0';
				std::ofstream fout;
				fout.open(filename,std::ios::app);
				fout<<GetUserId()<<","<<speed<<std::endl;
				fout.close();
				isFirst = false;
			}
			// std::cout<<"UserId："<<GetUserId()<<", Speed："<<speed<<std::endl;
		}else{
			speed = MyBuilding::GetUserSpeed(); //スピードを設定
		}
		double arrival = dist / speed; //到着時間を計算
		Setarrival(arrival);
		double wpTime = now + arrival;
		Waypoint wpt(Seconds(wpTime), nvct);
		Ptr<WaypointMobilityModel> mob = GetObject<WaypointMobilityModel>();
		mob->AddWaypoint(wpt);
		HandleUserMovementTimer(Seconds(arrival) - NanoSeconds(1));
		return;
	}



	// if (isWait and !isExit) {
	// 	Vector nvct = SelectRandomNextLine();
	// 	double dist = CalculateDistance(vct, nvct);
	// 	double now = Simulator::Now().GetDouble() / 1000000000;
	// 	double arrival = dist / MyBuilding::GetUserSpeed();
	// 	double wpTime = now + arrival;
	// 	Waypoint wpt(Seconds(wpTime), nvct);
	// 	Ptr<WaypointMobilityModel> mob = GetObject<WaypointMobilityModel>();
	// 	mob->AddWaypoint(wpt);
	// 	isWait = false;
	// 	isExit = true;
	// 	HandleUserMovementTimer(Seconds(arrival) - NanoSeconds(1));
	// 	return;
	// }

	if (isExit) {
//		String exitnodeid=GetExitnode();

//		Time waitTime = MyBuilding::CalculationWaitTime(m_userId, m_sectionId, m_routeVector.front());
//		Time delay = /*waitTime -*/ Simulator::Now();
		CompleteEvacuation("NONE");
		return;
	}
}

// double MyUser::CalculateDistance_from_coordinate(std::pair<double, double> start, std::pair<double, double> end) {
//     // ÀW start = (x1, y1), end = (x2, y2)
//     double x1 = start.first;
//     double y1 = start.second;
//     double x2 = end.first;
//     double y2 = end.second;

//     // [Nbh£ÌvZ
//     double distance = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
//     return distance;
// }

void MyUser::TaskOfCrossing (way_node ni){
	// std::cout<<"TaskOfCrossing"<<std::endl;
	// std::pair<double,double> coordinate = ni.GetCoordinate();

	// if(m_userId + 1 == 18){
	// 	std::cout<<coordinate.first<<", "<<coordinate.second<<std::endl;
	// 	// std::cout<<"避難所までの距離："<<std::endl;
	// 	// for(auto a : node_to_exit_distance){
	// 	// 	std::cout<<"("<<a.first.first<<", "<<a.first.second<<"), 距離："<<a.second<<std::endl;
	// 	// }
	// 	try{
	// 		std::cout<<"避難所までの距離は"<<node_to_exit_distance.at(coordinate)<<std::endl;
	// 	}catch(std::out_of_range& oor){
	// 		std::cout<<"スキップ"<<std::endl;
	// 	}
	// }
	String targetnode = "NULL";
	String nodeid;
	double increse;
	double distance;
	double required_time;
	double expected_num_user;
	int remaining_capacity = 400000;
	int remaining_capacity_arrival;
	bool doflag = true;
	try{
		targetnode = GetExitnode();
		nodeid = JSON_Data::GetIDfromCoordinate(ni.GetCoordinate());
		increse = increase_rate.at(GetExitnode()); //毎秒避難者が何人増えていくか
		distance = m_exit_distance_map.at(nodeid); //現在のノードから避難所までの距離
		required_time = distance/speed; //避難所に到着するまでの所要時間
		expected_num_user = increse * required_time; //到着するまでに何人増えているか
		remaining_capacity = MyBuilding::GetExitNodes().at(GetExitnode());
		remaining_capacity_arrival = remaining_capacity - expected_num_user; //到着時の予想残り収容可能人数
		m_generation_time = Simulator::Now().GetDouble() / 1000000000; //情報の生成時間を記録
		m_required_time = required_time;
		if(MyBuilding::GetSendHopFlag() == true){
			UpdateHop(m_userId, std::make_pair(std::make_pair(targetnode, required_time), m_generation_time)); //自身のデータ更新
		}
	}catch(std::out_of_range& oor){
		// std::cout<<"情報がありません（まだ目標とする交差点が定まっていません）"<<std::endl;
		m_required_time = 0;
		doflag = false;
	}

	if(MyBuilding::GetUseExitInfoFlag()){
		try{
			// String nodeid = JSON_Data::GetIDfromCoordinate(ni.GetCoordinate());
			// double increse = increase_rate.at(GetExitnode()); //毎秒避難者が何人増えていくか
			// String nodeid = JSON_Data::GetIDfromCoordinate(m_currentway.GetNodes().front().GetCoordinate());
			// double distance = m_exit_distance_map.at(nodeid); //現在のノードから避難所までの距離
			// double required_time = distance/speed; //避難所に到着するまでの所要時間
			// double expected_num_user = increse * required_time; //到着するまでに何人増えているか
			// std::cout<<"-------------------------------------------------------"<<std::endl;
			// std::cout<<m_userId + 1<<"----目標としている避難所ID："<<GetExitnode()<<std::endl;
			// std::cout<<"増加率："<<increse<<std::endl;
			// std::cout<<"移動速度："<<speed<<std::endl;
			// std::cout<<"現在いるノード："<<nodeid<<std::endl;
			// std::cout<<"避難所までの距離："<<distance<<std::endl;
			// std::cout<<"到着までにかかる時間："<<required_time<<std::endl;
			// int remaining_capacity = MyBuilding::GetExitNodes().at(GetExitnode());
			// std::cout<<"現在の収容可能人数："<<remaining_capacity<<std::endl;
			// std::cout<<"到着までの予想増加人数："<<expected_num_user<<std::endl;
			// int remaining_capacity_arrival = remaining_capacity - expected_num_user; //到着時の予想残り収容可能人数
			// std::cout<<"到着時の予想収容可能人数："<<remaining_capacity_arrival<<std::endl;
			// std::cout<<"-------------------------------------------------------"<<std::endl;
			if(remaining_capacity_arrival < 1 && nodeid != GetExitnode() && doflag == true){
				std::cout<<"-------------------------------------------------------"<<std::endl;
				std::cout<<m_userId + 1<<"----目標としている避難所ID："<<GetExitnode()<<std::endl;
				std::cout<<"増加率："<<increse<<std::endl;
				std::cout<<"移動速度："<<speed<<std::endl;
				std::cout<<"現在いるノード："<<nodeid<<std::endl;
				std::cout<<"避難所までの距離："<<distance<<std::endl;
				std::cout<<"到着までにかかる時間："<<required_time<<std::endl;
				std::cout<<"現在の収容可能人数："<<remaining_capacity<<std::endl;
				std::cout<<"到着までの予想増加人数："<<expected_num_user<<std::endl;
				std::cout<<"到着時の予想収容可能人数："<<remaining_capacity_arrival<<std::endl;
				std::cout<<"-------------------------------------------------------"<<std::endl;
				std::cout<<"到着時には避難所が満員になる予想です！！！！"<<std::endl;
				m_exitnodeinfolist.erase(JSON_Data::NodeIDstoi(nodeid));
				m_fullexitnodeid.insert(nodeid);
				// m_required_time = required_time;
				// m_generation_time = Simulator::Now().GetDouble() / 1000000000; //情報の生成時間を記録
				isDijkstra = true;
				MyBuilding::PlusCount();
			}
		}catch(std::out_of_range& oor){
			m_required_time = 0;
		}
	}

	if(MyBuilding::GetSendHopFlag() == true){
		int count = 0;
		double myrequired_time = m_generation_time - Simulator::Now().GetDouble() / 1000000000;
		std::map <uint32_t, std::pair<std::pair<String,uint32_t>, uint32_t>> only_targetnode_hop; //目標とする避難所のみだけを取り出したmap
		for(auto a : GetHop()){
			// double myrequired_time = m_generation_time - Simulator::Now().GetDouble() / 1000000000;
			if(a.second.first.first == targetnode){
				only_targetnode_hop.insert(a);
			}
		}
		for(auto a : only_targetnode_hop){
			if(myrequired_time > 0){
				uint32_t keika = (Simulator::Now().GetDouble() / 1000000000) - a.second.second; //情報が生成されてからの経過時間
				uint32_t expected_arrival_time = a.second.first.second - keika; //現時点からの到着予想時間
				if(expected_arrival_time < myrequired_time){
					count++;
				}
			}
		}

		if(count > 0) count--; //-1しているのは自分のデータ分を引くため

		if(remaining_capacity - (count) <= 0){ 
			std::cout<<"到着時には避難所が満員になります！！"<<std::endl;
			m_exitnodeinfolist.erase(JSON_Data::NodeIDstoi(nodeid));
			m_fullexitnodeid.insert(nodeid);
			isDijkstra = true;
			MyBuilding::PlusCount();
		}

		for(auto a : GetHop()){
			if(a.second.first.second <= 0){
				std::cout<<"到着予想が0秒後なので削除します"<<std::endl;
				hopTimers[a.first].Cancel();
				EraseHop(a.first); //到着予想時間が0のデータはすぐに削除する．
			}else{
				// std::cout<<a.second.second<<"秒後に情報を削除するタイマーをセットしました"<<std::endl;
				uint32_t keika = (Simulator::Now().GetDouble() / 1000000000) - a.second.second; //情報が生成されてからの経過時間
				uint32_t expected_arrival_time = a.second.first.second - keika; //現時点からの到着予想時間
				EraseHopDataTimer(expected_arrival_time, a.first);
			}
		}
	}

	for(auto itr=m_shortest.begin();itr!=m_shortest.end();++itr){
		std::pair<double,double> it = *itr;
		String id = JSON_Data::get_waynodes().at(it).GetNodeID();
		// std::cout<<"m_shortest:"<<id<<std::endl;
	}


	if(ni.GetCoordinate()==m_shortest.front()){
		m_passednode.push_back(m_shortest.front());
		m_shortest.pop_front();
	}else{
		return;
	}
//	m_shortest.pop_front();
	if(m_shortest.empty()) return;

	std::list<String> select_way;

	auto short_next = m_shortest.begin();
	++short_next;

	std::list<String> wl = ni.GetWayIDs();
	way_info wi;

	for(auto itr = wl.begin();itr != wl.end();++itr){
		String it = *itr;
		// std::cout<<it<<std::endl;
		wi = JSON_Data::get_linestrings().at(it);
		std::list<way_node> nodes = wi.GetNodes();

		for(auto itr2 = nodes.begin();itr2 != nodes.end();++itr2){
			way_node it2 = *itr2;

			if(it2.GetCoordinate() == m_shortest.front()) {
				select_way.push_back(it);
				for(auto itr3 = nodes.begin();itr3 != nodes.end();++itr3){
					way_node it3 = *itr3;
				}
			}
		}
	}

	if(select_way.size()>1){
		// std::cout<<"two candidate way"<<std::endl;
		double select_way_length = INF;
		for(auto itr = select_way.begin();itr != select_way.end();++itr){
			String it = *itr;
			// std::cout<<it<<std::endl;
			double way_length=0.0;
			std::list<way_node> wnlist = JSON_Data::get_linestrings().at(it).GetNodes();
			bool f_flag = false;
			bool b_flag = false;
			std::pair<double,double> pre_coord;
			for(auto itr2 = wnlist.begin();itr2 != wnlist.end();++itr2){
				way_node it2 = *itr2;
				std::pair<double,double> coord=it2.GetCoordinate();

				if(f_flag||b_flag){
					way_length = way_length + JSON_Data::calculate_distance(pre_coord,coord);
					pre_coord=coord;
				}

				if(coord==ni.GetCoordinate()){
					if(!f_flag){
						if(b_flag){
							break;
						}else{
							f_flag=true;
							pre_coord=coord;
						}
					}

				}
				if(coord==m_shortest.front()){
					if(!b_flag){
						if(f_flag){
							break;
						}else{
							b_flag=true;
							pre_coord=coord;
						}
					}
				}

			}

			if(way_length<select_way_length){
				select_way_length=way_length;
				wi = JSON_Data::get_linestrings().at(it);
			}
		}
	}else{
		// if(m_userId + 1 == 243){
		// 	std::cout<<m_userId+1<<std::endl;
		// 	for(auto a : select_way){
		// 		std::cout<<a<<std::endl;
		// 	}
		// }
		wi=JSON_Data::get_linestrings().at(select_way.front());
	}



	if(wi.GetNodes().front().GetCoordinate().first == ni.GetCoordinate().first && wi.GetNodes().front().GetCoordinate().second == ni.GetCoordinate().second){
		// crossing node at front of the next way
		// std::cout<<"Type1"<<std::endl;
		SetCurrentWay(wi);
		// std::cout<<"next_way1:"<<m_currentway.GetID()<<std::endl;
		// m_currentway.Print();
		return;
	}else if(wi.GetNodes().back().GetCoordinate().first == ni.GetCoordinate().first && wi.GetNodes().back().GetCoordinate().second == ni.GetCoordinate().second){
		// crossing node at behind of the next way
		// std::cout<<"Type2"<<std::endl;
		std::list<way_node> add_wnlist;
		std::list<way_node> wnlist=wi.GetNodes();
		for(auto itr = wnlist.rbegin();itr != wnlist.rend();++itr){
			way_node it = *itr;

			add_wnlist.push_back(it);
		}
		wi.SetNodeList(add_wnlist);
		SetCurrentWay(wi);
		// std::cout<<"next_way2:"<<m_currentway.GetID()<<std::endl;
		// m_currentway.Print();
		return;
	}else{
		// crossing node on the next way
		// std::cout<<"Type3"<<std::endl;

		std::list<way_node> add_wnlist;
		std::list<way_node> wnlist=wi.GetNodes();

		// next crossing node is
		bool f_flag = false;
		bool b_flag = false;

		for(auto itr = wnlist.begin();itr != wnlist.end();++itr){
			way_node it = *itr;

			if(it.GetCoordinate()==ni.GetCoordinate()) {
				// std::cout<<"f_flag is true"<<std::endl;
				f_flag=true;
			}
			if(it.GetCoordinate()==m_shortest.front()) {
				// std::cout<<"b_flag is true"<<std::endl;
				b_flag=true;
			}

			if(f_flag||b_flag) break;
		}

		bool add_flag=false;
		if(f_flag){
			for(auto itr = wnlist.begin();itr != wnlist.end();++itr){
				way_node it = *itr;
				if(it.GetCoordinate()==ni.GetCoordinate()){
					add_flag=true;
				}
				if(add_flag){
					add_wnlist.push_back(it);
					if(it.GetCoordinate()==m_shortest.front()) break;
				}
			}
		}else if(b_flag){
			for(auto itr = wnlist.rbegin();itr != wnlist.rend();++itr){
				way_node it = *itr;
				if(it.GetCoordinate()==ni.GetCoordinate()){
					add_flag=true;
				}
				if(add_flag){
					add_wnlist.push_back(it);
					if(it.GetCoordinate()==m_shortest.front()) break;
				}
			}
		}

		wi.SetNodeList(add_wnlist);
		SetCurrentWay(wi);
		return;
	}

}

void MyUser::SetEraseHopTimer(){
	for(auto a : GetHop()){
		if(a.second.first.second <= 0){
			std::cout<<"到着予想が0秒後なので削除します"<<std::endl;
			hopTimers[a.first].Cancel();
			EraseHop(a.first); //到着予想時間が0のデータはすぐに削除する．
		}else{
			// std::cout<<a.second.second<<"秒後に情報を削除するタイマーをセットしました"<<std::endl;
			uint32_t keika = (Simulator::Now().GetDouble() / 1000000000) - a.second.second; //情報が生成されてからの経過時間
			uint32_t expected_arrival_time = a.second.first.second - keika; //現時点からの到着予想時間
			EraseHopDataTimer(expected_arrival_time, a.first);
		}
	}
}


Vector MyUser::GetWayNextVector ()
{

	way_info wi;
	way_node ni;
	std::list<way_node> wnlist;
	wi=m_currentway;
	wnlist=m_currentway.GetNodes();
	wnlist.pop_front();
	// bool skipflag = false;

	try{
		ni = JSON_Data::get_waynodes().at(wnlist.front().GetCoordinate());
		// std::cout<<"next_node:("<<ni.GetCoordinate().first<<","<<ni.GetCoordinate().second<<")"<<std::endl;
	}
	catch(std::out_of_range& oor){

	}

	// if(m_userId + 1 == 271){//debag
	// 	for(auto a : wnlist){
	// 		std::pair<double, double> coordinate = a.GetCoordinate();
	// 		String nodeid = JSON_Data::GetIDfromCoordinate(coordinate);
	// 		std::cout<<nodeid<<std::endl;
	// 	}
	// }


	if(wnlist.size()==0 || ni.GetBlockedflag()){
		// if(m_userId + 1 == 271){//debag
		// 	std::cout<<"1"<<std::endl;
		// }
		std::list<way_node> swapnodelist;
		if(wnlist.size()==0){
			std::cout<<m_userId + 1<<", wnlist_null"<<std::endl;
			swapnodelist=JSON_Data::get_linestrings().at(m_currentway.GetID()).GetNodes();
			if(swapnodelist.front().GetCoordinate()==m_currentway.GetNodes().front().GetCoordinate()){
				// std::cout<<"front"<<std::endl;
				for(auto itr = swapnodelist.begin();itr != swapnodelist.end();++itr){
					way_node it = *itr;
					wnlist.push_back(it);
				}
				wnlist.pop_front();
				m_currentway.SetNodeList(wnlist);
			}
			if(swapnodelist.back().GetCoordinate()==m_currentway.GetNodes().front().GetCoordinate()){
				// std::cout<<"back"<<std::endl;
				for(auto itr = swapnodelist.rbegin();itr != swapnodelist.rend();++itr){
					way_node it = *itr;
					wnlist.push_back(it);
				}
				wnlist.pop_front();
				m_currentway.SetNodeList(wnlist);
			}
		}else if(ni.GetBlockedflag()){
			// if(m_userId + 1 == 271){//debag
			// 	std::cout<<"2"<<std::endl;
			// }
			FindBlockedNodeID(ni.GetNodeID());
			if(!ni.GetNodeID().empty()){
				SetBlockedNodeID(ni.GetNodeID());
				SetBlockedNodeID_and_time(ni.GetNodeID(), (Simulator::Now().GetDouble() / 1000000000)); //mapにBlockノードIDと発見した時間を保存
				SetBlockedNodeID_and_time_for_Log(ni.GetNodeID(), (Simulator::Now().GetDouble() / 1000000000)); //mapにBlockノードIDと発見した時間を保存
				UpdateGraphInfo(ni);
				std::cout<<m_userId<<", "<<ni.GetNodeID()<<", findBlock!!, "<<Simulator::Now().GetDouble() / 1000000000<<std::endl;
			}

			wnlist.clear();

			swapnodelist=JSON_Data::get_linestrings().at(m_currentway.GetID()).GetNodes();
			// if(m_userId + 1 == 271){//debag
			// 	for(auto a : swapnodelist){
			// 		std::pair<double, double> coordinate = a.GetCoordinate();
			// 		String nodeid = JSON_Data::GetIDfromCoordinate(coordinate);
			// 		std::cout<<nodeid<<std::endl;
			// 	}
			// }
			// std::cout<<"//////////////find////////////////"<<std::endl;
			bool f_flag = false;
			bool b_flag = false;
			for(auto itr = swapnodelist.begin();itr != swapnodelist.end();++itr){
				way_node it = *itr;
				if(it.GetCoordinate() == m_currentway.GetNodes().front().GetCoordinate()){
					f_flag = true;
				}
				if(it.GetCoordinate() == ni.GetCoordinate()){
					b_flag = true;
				}
				if(f_flag || b_flag) break;
			}
			// std::cout<<f_flag<<", "<<b_flag<<std::endl; //debag
			bool add_flag=false;

			if(f_flag){
				for(auto itr = swapnodelist.rbegin();itr != swapnodelist.rend();++itr){
					way_node it = *itr;
					if(add_flag) wnlist.push_back(it);
					if(it.GetCoordinate()==m_currentway.GetNodes().front().GetCoordinate()){
						add_flag=true;
						// wnlist.push_back(it);
					}
				}
				if(wnlist.size() == 0){
					// std::cout<<"実行されています"<<std::endl;
					// skipflag = true;
					m_wnlist_buf.pop_front();
					m_currentway.SetNodeList(m_wnlist_buf);
				}else{
					m_currentway.SetNodeList(wnlist);
				}
				
			}else if(b_flag){
				for(auto itr = swapnodelist.begin();itr != swapnodelist.end();++itr){
					way_node it = *itr;
					// std::cout<<"1:"<<add_flag<<std::endl;
					if(add_flag){
						wnlist.push_back(it);
					}
					if(it.GetCoordinate()==m_currentway.GetNodes().front().GetCoordinate()){
						add_flag=true;
						// wnlist.push_back(it);
					}
				}
				if(wnlist.size() == 0){
					// std::cout<<"実行されています２"<<std::endl;
					// skipflag = true;
					m_wnlist_buf.pop_front();
					m_currentway.SetNodeList(m_wnlist_buf);
				}else{
					m_currentway.SetNodeList(wnlist);
				}
				// m_currentway.SetNodeList(wnlist);
			}
			UpdateGraphInfo(ni);
			isDijkstra=true;
		}
	}else{
		m_currentway.SetNodeList(wnlist);
	}

	// if(m_userId + 1 == 271){
	// 	std::cout<<"BUF2"<<std::endl;
	// 	for(auto a : m_wnlist_buf){
	// 		std::pair<double, double> coordinate = a.GetCoordinate();
	// 		String nodeid = JSON_Data::GetIDfromCoordinate(coordinate);
	// 		std::cout<<nodeid<<std::endl;
	// 	}		
	// }

	// m_wnlist_buf2 = m_wnlist_buf; //２つ前のwnlistを保持
	m_wnlist_buf = wnlist; //一つ前のwnlistを保持
	


	if(m_currentway.GetNodes().front().GetCrossingflag()/*&& wnlist.size() != 0*/) {
		// std::cout<<m_userId + 1<<", wnlist_size:"<<wnlist.size()<<std::endl;
		// if(m_userId + 1 == 271){//debag
		// 	for(auto a : wnlist){
		// 		std::pair<double, double> coordinate = a.GetCoordinate();
		// 		String nodeid = JSON_Data::GetIDfromCoordinate(coordinate);
		// 		std::cout<<nodeid<<std::endl;
		// 	}
		// 	std::cout<<std::endl;
		// }
		//std::cout<<m_userId+1<<std::endl;
		TaskOfCrossing(m_currentway.GetNodes().front());
	}else{
		// std::cout<<"スキップしました"<<std::endl;
	}


	double nsecx= JSON_Data::json_x(m_currentway.GetNodes().front().GetCoordinate().first);
	double nsecy = JSON_Data::json_y(m_currentway.GetNodes().front().GetCoordinate().second);
	double nsecz = 0.0;

	// String Id = m_currentway.GetNodes().front().GetNodeID();
	// if(m_userId + 1 == 271){//debag
	// 	std::pair<double, double> coordinate = wnlist.front().GetCoordinate();
	// 	String nodeid = JSON_Data::GetIDfromCoordinate(coordinate);
	// 	std::cout<<nodeid<<", "<<nsecx<<", "<<nsecy<<std::endl;
	// }
	return Vector(nsecx,nsecy,nsecz);
}

///////通行止め復活///////////
void MyUser::RestoreGraphInfo(String id) {
	// std::cout<<id<<"の通行止め情報を破棄しました"<<std::endl;
	std::pair<double,double> coordinate = JSON_Data::GetCoordinatefromID(id);
	std::map<std::pair<double,double>,way_node> nodes = JSON_Data::get_waynodes();
	way_node ni = nodes.at(coordinate);
	Hubeny H;

	if(ni.GetCrossingflag()){
		std::list<String> all_wi_list = ni.GetWayIDs();
		// for(auto a : all_wi_list){ //debag
		// 	std::cout<<a<<std::endl;
		// }
		for(auto wi_name : all_wi_list){
			way_info wi = JSON_Data::get_linestrings().at(wi_name);
			std::list<way_node> wnl = wi.GetNodes();
			// for(auto a : wnl){
			// 	std::pair<double, double> co = a.GetCoordinate();
			// 	String id = JSON_Data::GetIDfromCoordinate(co);
			// 	std::cout<<id<<std::endl;
			// }
			way_node s_node;
			way_node s_node2;
			way_node e_node;
			way_node e_node2;
			bool s_flag = false;
			bool e_flag = false;
			bool inset_s_flag = false;
			bool inset_e_flag = false;
			// uint32_t x = 0;
			// uint32_t y = 0;
			for(auto itr = wnl.begin(); itr != wnl.end(); ++itr) {
				way_node it = *itr;
				if(it.GetCrossingflag()){
					s_node2 = it;
				}
				if(s_flag && it.GetCrossingflag()) {
					// std::cout<<"!!!!!"<<std::endl;
					inset_s_flag = true;
					s_node = it;
					break;
				}
				if(it.GetCoordinate() == ni.GetCoordinate()) {
					s_flag = true;
					// if(x == 0 || itr == std::prev(wnl.end())){ //一回目のループつまりwayの始まりが交差点の場合
					// 	std::cout<<"こっちが実行されてる"<<std::endl;
					// 	s_node = it;
					// 	break;
					// }
				}
				// x++;
			}

			for(auto itr = wnl.rbegin(); itr != wnl.rend(); ++itr) {
				way_node it = *itr;
				if(it.GetCrossingflag()){
					e_node2 = it;
				}
				if(e_flag && it.GetCrossingflag()) {
					e_node = it;
					inset_e_flag = true;
					break;
				}
				if(it.GetCoordinate() == ni.GetCoordinate()) {
					e_flag = true;
					// if(y == 0){
					// 	e_node = it;
					// 	break;
					// }
				}
				// y++;
			}

			if(inset_s_flag == false){
				s_node = s_node2;
			}
			if(inset_e_flag == false){
				e_node = e_node2;
			}
			double neighbor_dist=0.0;
			double node_dist=0.0;
			H.hubeny_distance(s_node.GetCoordinate().first,s_node.GetCoordinate().second,e_node.GetCoordinate().first,e_node.GetCoordinate().second,node_dist);
			neighbor_dist = neighbor_dist + node_dist;

			// std::cout<<inset_s_flag<<", "<<inset_e_flag<<std::endl;
			// std::cout<<s_node.GetCoordinate().first<<", "<<s_node.GetCoordinate().second<<std::endl;
			// std::cout<<e_node.GetCoordinate().first<<", "<<e_node.GetCoordinate().second<<std::endl;
			// std::pair<double, double>s_coordinate = s_node.GetCoordinate();
			// std::pair<double, double>e_coordinate = e_node.GetCoordinate();
			// std::cout<<"start_node = "<<JSON_Data::GetIDfromCoordinate(s_coordinate)<<", end_node = "<<JSON_Data::GetIDfromCoordinate(e_coordinate)<<std::endl;
			std::map<std::pair<double,double>,double> s_neighbor;
			std::map<std::pair<double,double>,double> e_neighbor;
			try{
				s_neighbor = m_graph.at(s_node.GetCoordinate()).GetNeighborNodes();
			}catch(std::out_of_range& oor){

			}
			// s_neighbor = m_graph.at(s_node.GetCoordinate()).GetNeighborNodes();
			s_neighbor.insert(std::make_pair(e_node.GetCoordinate(), neighbor_dist));
			s_node.SetNeighborNodes(s_neighbor);
			m_graph.erase(s_node.GetCoordinate());
			m_graph.insert(std::make_pair(s_node.GetCoordinate(), s_node));

			try{
				e_neighbor = m_graph.at(s_node.GetCoordinate()).GetNeighborNodes();
			}catch(std::out_of_range& oor){

			}
			// e_node Ì×Úm[hîñðXV
			// std::map<std::pair<double,double>,double> e_neighbor = m_graph.at(e_node.GetCoordinate()).GetNeighborNodes();
			e_neighbor.insert(std::make_pair(s_node.GetCoordinate(), neighbor_dist));
			e_node.SetNeighborNodes(e_neighbor);
			m_graph.erase(e_node.GetCoordinate());
			m_graph.insert(std::make_pair(e_node.GetCoordinate(), e_node));
		}
	}

    if(!ni.GetCrossingflag()){
        way_info wi = JSON_Data::get_linestrings().at(ni.GetWayIDs().front());
		std::list<way_node> wnl = wi.GetNodes();
		way_node s_node;
		way_node e_node;
		bool s_flag = false;
		bool e_flag = false;

		for(auto itr = wnl.begin(); itr != wnl.end(); ++itr) {
			way_node it = *itr;
			if(s_flag && it.GetCrossingflag()) {
				s_node = it;
				break;
			}
			if(it.GetCoordinate() == ni.GetCoordinate()) {
				s_flag = true;
			}
		}
		for(auto itr = wnl.rbegin(); itr != wnl.rend(); ++itr) {
			way_node it = *itr;
			if(e_flag && it.GetCrossingflag()) {
				e_node = it;
				break;
			}
			if(it.GetCoordinate() == ni.GetCoordinate()) {
				e_flag = true;
			}
		}

		double neighbor_dist=0.0;
		double node_dist=0.0;
		H.hubeny_distance(s_node.GetCoordinate().first,s_node.GetCoordinate().second,e_node.GetCoordinate().first,e_node.GetCoordinate().second,node_dist);
		neighbor_dist = neighbor_dist + node_dist;

		std::map<std::pair<double,double>,double> s_neighbor = m_graph.at(s_node.GetCoordinate()).GetNeighborNodes();
		s_neighbor.insert(std::make_pair(e_node.GetCoordinate(), neighbor_dist));
		s_node.SetNeighborNodes(s_neighbor);
		m_graph.erase(s_node.GetCoordinate());
		m_graph.insert(std::make_pair(s_node.GetCoordinate(), s_node));

		// e_node Ì×Úm[hîñðXV
		std::map<std::pair<double,double>,double> e_neighbor = m_graph.at(e_node.GetCoordinate()).GetNeighborNodes();
		e_neighbor.insert(std::make_pair(s_node.GetCoordinate(), neighbor_dist));
		e_node.SetNeighborNodes(e_neighbor);
		m_graph.erase(e_node.GetCoordinate());
		m_graph.insert(std::make_pair(e_node.GetCoordinate(), e_node));
	}

}




void MyUser::UpdateGraphInfo(way_node ni){
	// std::cout<<"m_currentway = "<<m_currentway.GetID()<<std::endl;
	// std::cout<< "ni = "<<ni.GetNodeID()<<std::endl;
	// way_info wi = JSON_Data::get_linestrings().at(m_currentway.GetID()); //通行止めが交差点で，両サイドが交差点でないときはこの処理いるかも．．．
	// std::cout<<"2"<<std::endl;
	way_info wi = JSON_Data::get_linestrings().at(ni.GetWayIDs().front());
	if(ni.GetCrossingflag()){
		wi = JSON_Data::get_linestrings().at(m_currentway.GetID());
	}
	std::list<way_node> wnl = wi.GetNodes();
	way_node s_node; //始点ノード
	way_node e_node; //終点ノード
	bool s_flag=false;
	bool e_flag=false;

	for(auto itr = wnl.begin(); itr != wnl.end(); ++itr){
		way_node it = *itr;
		if(s_flag && it.GetCrossingflag()){
			s_node=it;
			break;
		}
		if(it.GetCoordinate() == ni.GetCoordinate()){
			s_flag=true;
		}
	}

	for(auto itr = wnl.rbegin(); itr != wnl.rend(); ++itr){
		way_node it = *itr;
		if(e_flag && it.GetCrossingflag()){
			e_node=it;
			break;
		}
		if(it.GetCoordinate() == ni.GetCoordinate()){
			e_flag=true;
		}
	}
	
	// std::cout<<m_userId+1<<std::endl;
	// std::cout<<s_node.GetCoordinate().first<<std::endl;
	// try{
	// 	std::cout<<"通行止めがある道の最初のノードは "<<JSON_Data::GetIDfromCoordinate(s_node.GetCoordinate())<<", 終わりのノードは = "<<JSON_Data::GetIDfromCoordinate(e_node.GetCoordinate())<<std::endl;
	// }catch(std::out_of_range& oor){
	// 	std::cout<<"見つかりません！！(通行止め箇所情報が反映されていません！)"<<std::endl; //
	// 	s_node = ni;
	// 	e_node = ni;
	// }
	// std::cout<<"start_node = "<<JSON_Data::GetIDfromCoordinate(s_node.GetCoordinate())<<", end_node = "<<JSON_Data::GetIDfromCoordinate(e_node.GetCoordinate())<<std::endl;

	if(ni.GetCrossingflag()){
		std::map<std::pair<double,double>, double> neighbormap = ni.GetNeighborNodes();
		for(auto neighbor : neighbormap){
			way_node node = m_graph.at(neighbor.first);
			std::map<std::pair<double,double>,double> neighbor2 = m_graph.at(neighbor.first).GetNeighborNodes(); //coordinate and distance
			neighbor2.erase(ni.GetCoordinate());
			node.SetNeighborNodes(neighbor2);
			m_graph.erase(ni.GetCoordinate());
			m_graph.insert(std::make_pair(node.GetCoordinate(),node));			
		}
		// std::map<std::pair<double,double>,double> s_neighbor = m_graph.at(s_node.GetCoordinate()).GetNeighborNodes();
		// s_neighbor.erase(ni.GetCoordinate());
		// s_node.SetNeighborNodes(s_neighbor);
		// m_graph.erase(ni.GetCoordinate());
		// m_graph.insert(std::make_pair(s_node.GetCoordinate(),s_node));

		// std::map<std::pair<double,double>,double> e_neighbor = m_graph.at(e_node.GetCoordinate()).GetNeighborNodes();
		// e_neighbor.erase(ni.GetCoordinate());
		// e_node.SetNeighborNodes(e_neighbor);
		// m_graph.erase(ni.GetCoordinate());
		// m_graph.insert(std::make_pair(e_node.GetCoordinate(),e_node));
		
	}else{
		// std::cout<<"あっち"<<std::endl;
		std::map<std::pair<double,double>,double> s_neighbor = m_graph.at(s_node.GetCoordinate()).GetNeighborNodes();
		s_neighbor.erase(e_node.GetCoordinate());
		s_node.SetNeighborNodes(s_neighbor);
		m_graph.erase(s_node.GetCoordinate());
		m_graph.insert(std::make_pair(s_node.GetCoordinate(),s_node));

		std::map<std::pair<double,double>,double> e_neighbor = m_graph.at(e_node.GetCoordinate()).GetNeighborNodes();
		e_neighbor.erase(s_node.GetCoordinate());
		e_node.SetNeighborNodes(e_neighbor);
		m_graph.erase(e_node.GetCoordinate());
		m_graph.insert(std::make_pair(e_node.GetCoordinate(),e_node));
	}

}

void MyUser::UpdateGraphInfoReceivedPacket(String id){
	// std::cout<<"ここでエラー"<<std::endl;
	// std::cout<<"id = "<<id<<std::endl;
	if(m_nodeType == FERRY_NODE){
		return;
	}
	std::pair<double,double> coordinate = JSON_Data::GetCoordinatefromID(id);
	std::map<std::pair<double,double>,way_node> nodes = JSON_Data::get_waynodes();
	UpdateGraphInfo(nodes.at(coordinate));

	isDijkstra=true;
}

Vector MyUser::SelectRandomNextLine ()
{
	std::vector<std::vector<MySection>>* mySection = &GetMySection();
	Position nsecid = m_routeVector.back();
	uint32_t nsecx = nsecid.x;
	uint32_t nsecy = nsecid.y;
	double nposx, nposy, nposz;
	double top = (*mySection).at(nsecy).at(nsecx).GetTop();
	double bottom = (*mySection).at(nsecy).at(nsecx).GetBottom();
	double left = (*mySection).at(nsecy).at(nsecx).GetLeft();
	double right = (*mySection).at(nsecy).at(nsecx).GetRight();
	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());

	if (m_sectionId.y < nsecy) {
		std::uniform_real_distribution<> dist(left, right);
		nposx = dist(engine);
		nposy = top;
		nposz = 0;
	}
	else if (m_sectionId.y > nsecy) {
		std::uniform_real_distribution<> dist(left, right);
		nposx = dist(engine);
		nposy = bottom;
		nposz = 0;
	}
	else if (m_sectionId.x < nsecx) {
		std::uniform_real_distribution<> dist(top, bottom);
		nposx = left;
		nposy = dist(engine);
		nposz = 0;
	}
	else if (m_sectionId.x > nsecx) {
		std::uniform_real_distribution<> dist(top, bottom);
		nposx = right;
		nposy = dist(engine);
		nposz = 0;
	}

	return Vector(nposx, nposy, nposz);
}

Vector MyUser::SelectRandomNextPosition ()
{
	std::vector<std::vector<MySection>>* mySection = &GetMySection();
	Position nsecid = m_routeVector.back();
	uint32_t nsecx = nsecid.x;
	uint32_t nsecy = nsecid.y;
	double top = (*mySection).at(nsecy).at(nsecx).GetTop();
	double bottom = (*mySection).at(nsecy).at(nsecx).GetBottom();
	double left = (*mySection).at(nsecy).at(nsecx).GetLeft();
	double right = (*mySection).at(nsecy).at(nsecx).GetRight();

	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());
	std::uniform_real_distribution<> distw(left, right);
	std::uniform_real_distribution<> disth(top, bottom);
	double nposx = distw(engine);
	double nposy = disth(engine);
	double nposz = 0;

	return Vector(nposx, nposy, nposz);
}

void MyUser::DijkstraRouteCalculationforGeographicInfo (way_node no)
{
	// std::cout<<m_userId+1<<", DijkstraRouteCalculationforGeographicInfo"<<std::endl;

	std::map<std::pair<double,double>,way_node> g = GetGraph();

	std::map<std::pair<double,double> ,Dijkstra_Info> p_nodes;
	std::map<std::pair<double,double> ,Dijkstra_Info> nodes;
	std::map<std::pair<double,double>,double> neighbor;
	double min_cost;
	std::pair<double,double> select_node;
	std::pair<double,double> target_node;
	std::list<std::pair<double,double>> route_list;
	std::map<String, double> exit_distance_map;
	// bool flag = false;

///////////////////////////Initialization/////////////////////////////
//交差点ノードしか実行していない（全ノードあるいは避難所ノードは実行したいなぁ）
	std::list<std::pair<double,double>> s;
	for(const auto &node : g){
		if(node.first==no.GetCoordinate()){
			// std::cout<<"避難所？"<<std::endl;
			//避難所ノードの場合実行されてそう
			Dijkstra_Info Ini(0.0,s); //初期化
			p_nodes.insert(std::make_pair(node.first,Ini));
			neighbor.insert(std::make_pair(node.first,0.0));
		}else{
			// std::cout<<"交差点ノード？"<<std::endl;
			//交差点ノードの場合実行されてそう
			Dijkstra_Info Ini(INF,s);
			p_nodes.insert(std::make_pair(node.first,Ini));
		}
	}
///////////////////////////Initialization/////////////////////////////

	String no_id = JSON_Data::GetIDfromCoordinate(no.GetCoordinate());

	// node_to_exit_distance.clear();
//	std::cout<<"size:"<<p_nodes.size()<<std::endl;
	while(true){
//		std::cout<<"test"<<std::endl;
		if(p_nodes.empty()) break;
		min_cost=(double)INF;
		for(const auto & node : p_nodes){
			// if(m_userId + 1 == 18){
			// 	std::cout<<"node.second.dst:"<<node.second.dst<<" min_cost:"<<min_cost<<std::endl;
			// }
			if(node.second.dst < min_cost){
				min_cost=node.second.dst;
				select_node=node.first;
			}
		}

		try{
			// std::cout<<"実行"<<std::endl;
			nodes.insert(std::make_pair(select_node,p_nodes.at(select_node)));
		}
		catch(std::out_of_range& oor){
			std::cout<<m_userId + 1<<"：おかしいかもよ？"<<std::endl;
			break;
		}

		std::list<std::pair<double,double>> upd_route = p_nodes.at(select_node).route;
		upd_route.push_back(select_node);
		p_nodes.erase(select_node);

		neighbor=g.at(select_node).GetNeighborNodes();
//		std::cout<<p_nodes.size()<<std::endl;

		std::map<std::pair<double,double>,Dijkstra_Info> upd_map;
		for(const auto &node : p_nodes){
			for(const auto &n_node : neighbor){
				double upd_dst=n_node.second + min_cost;
				if(node.first == n_node.first && node.second.dst > upd_dst){
					Dijkstra_Info  upd_info(upd_dst,upd_route);
					//std::cout<<"1"<<std::endl;
					//現在地から各交差点ノードまでの最短距離
					String id = JSON_Data::GetIDfromCoordinate(n_node.first);
					if (exit_distance_map.find(id) == exit_distance_map.end()) { // ÇÁR[h
						exit_distance_map[id] = upd_dst;                             // ÇÁR[h
					} else if (exit_distance_map[id] > upd_dst) {                   // ÇÁR[h
						exit_distance_map[id] = upd_dst;                             // ÇÁR[h
					}
					upd_map.insert(std::make_pair(n_node.first,upd_info));
				}
			}
		}
		//std::cout<<"3"<<std::endl;

		for(const auto &node : upd_map){
			p_nodes.erase(node.first);
			p_nodes.insert(std::make_pair(node.first,node.second));
		}

	}
	// if(m_userId + 1 == 18){ //debag
	// 	for(auto a : exit_distance_map){
	// 		std::cout<<a.first<<", "<<a.second<<std::endl;
	// 	}	
	// }


	
	double exit_distance=(double)INF;
	std::map<String,std::pair<double,double>> exitinfomap={};

//	std::set<String> fullexitnodeid = RoutingProtocol::GetFullexitNodeID();

	//////////////////////////避難先選択//////////////////////////////////
	for(auto itr = m_exitnodeinfolist.begin();itr != m_exitnodeinfolist.end();++itr){
		uint32_t id = itr->first;
		String ID = "node/" + std::to_string(id);
		std::pair<uint32_t,uint32_t> exitinfo = itr->second;
		uint32_t evac_upper = exitinfo.second;
		std::pair<double,double> it = JSON_Data::GetCoordinatefromID(ID);
		// std::cout<<it.first<<","<<it.second<<std::endl;
		decltype(m_fullexitnodeid)::iterator it2 = m_fullexitnodeid.find(ID);
		// std::cout<<"ここでエラー？"<<std::endl;
		// for(auto a : nodes){
		// 	std::cout<<a.first.first<<","<<a.first.second<<std::endl;
		// }
		double dis;
		try{
			dis = nodes.at(it).dst; //距離を取得
		}catch(const std::out_of_range& e){
			std::cout<<"避難所までの経路が検索できません"<<std::endl;
			return;
		}

		std::pair<double,double> capanddis = std::make_pair((double)exitinfo.second/(double)exitinfo.first,dis); //キャパと距離
		exitinfomap.insert(std::make_pair(ID,capanddis)); //(ID)と(キャパと距離)をexitinfomapに挿入

		// if(userId == 21){
		// 	std::cout << "evac_upper = " <<evac_upper<< std::endl;
		// }
		if((dis < exit_distance)&&(evac_upper != 0)){
			if(it2 == m_fullexitnodeid.end()){
				exit_distance = dis;
				target_node = it;
			}
		}
	}

	// node_to_exit_distance.clear();
	//各ノードから避難所までの距離
	// Hubeny H;
    // for(const auto &node : nodes){
	// 	// if(m_userId + 1 == 18){
	// 	// 	String id = JSON_Data::GetIDfromCoordinate(node.first);
	// 	// 	std::cout<<id<<std::endl;
	// 	// }
	// 	double distance_to_exit;
	// 	H.hubeny_distance(node.first.first, node.first.second, target_node.first, target_node.second, distance_to_exit);
    //     node_to_exit_distance[node.first] = distance_to_exit;
    // }


	distance_to_exit = exit_distance;
	//std::cout<<"1"<<std::endl;
	//////////////////////////////////////////////useexitinfo_obayashi/////////////////////////////////////////////////

	// if(MyBuilding::GetUseExitInfoFlag()){

	// 	// std::cout<<m_userId + 1<<", ホップ数："<<m_hop.size()<<std::endl;
	// 	// std::cout<<"UserId = "<<m_userId + 1<<"：";
	// 	// for(auto a : increase_rate){
	// 	// 	std::cout<<a.first<<", "<<a.second<<std::endl;
	// 	// }
	// 	// std::cout<<std::endl;
	// 	MyBuilding::SetMaxDistanceToExit(JSON_Data::GetIDfromCoordinate(target_node), exit_distance);
		
	// 	for(auto exit : m_exitnodeinfolist){
	// 		String exit_node_id = "node/"+std::to_string(exit.first);
	// 		// std::cout<<"現在目指している避難所は"<<exit_node_id<<std::endl; //debag
	// 		std::pair<double, double> zahyou = JSON_Data::GetCoordinatefromID(exit_node_id);
	// 		if(target_node == zahyou){
				
	// 		}
	// 	}
	// }

	///////////////////////////////////////////////////////useexitinfo//////////////////////////////////////////////////////////////
	// if(isUseexitinfo){ //モードがONの場合
	// 	double caprat = exitinfomap.at(JSON_Data::GetIDfromCoordinate(target_node)).first; //容量率を取得
	// 	if(caprat < THRESHOLD1){ //容量率が0.7より小さい時
	// 		for(auto itr = m_exitnodeinfolist.begin(); itr != m_exitnodeinfolist.end(); ++itr){
	// 			uint32_t id = itr->first;
	// 			String ID = "node/" + std::to_string(id);
	// 			std::pair<double,double> it = exitinfomap.at(ID); //キャパと距離
	// 			std::pair<double,double> upd = std::make_pair(it.first, it.second - exit_distance);
	// 			exitinfomap.at(ID) = upd;
	// 		}

	// 		if(exit_distance > THRESHOLD2){
	// 			double rat = (double)INF;
	// 			double shortdst = THRESHOLD2*THRESHOLD3;
	// 			for(auto itr = exitinfomap.begin();itr != exitinfomap.end();++itr){
	// 				std::pair<String,std::pair<double,double>> it = *itr;
	// 				std::pair<double,double> coord = JSON_Data::GetCoordinatefromID(it.first);
	// 				decltype(m_fullexitnodeid)::iterator it2 = m_fullexitnodeid.find(it.first);

	// 				if(it.second.second<rat&&it.second.second<shortdst&&it.second.first>caprat&&it2==m_fullexitnodeid.end()&&it.second.second!=0.0){
	// 					rat=it.second.second;
	// 					exit_distance=nodes.at(coord).dst;
	// 					target_node=coord;
	// 				}
	// 			}
	// 		}

	// 	}
	// }
	route_list=nodes.at(target_node).route;
	route_list.push_back(target_node);

	// if(m_userId + 1 == 18){
	// std::cout<<"-----------"<<m_userId + 1<<"-----------"<<std::endl;
	// for(auto itr = route_list.rbegin(); itr != route_list.rend(); ++itr){
	// 	std::pair<double,double> it = *itr;
	// 	std::cout<<JSON_Data::GetIDfromCoordinate(it)<<std::endl;
	// }
	// std::cout<<"---------------------------"<<std::endl;
	// 	std::cout<<nodes.at(target_node).dst<<std::endl;
	// }

	m_exit_distance_map.clear();
	for(auto itr = route_list.rbegin(); itr != route_list.rend(); ++itr){
		std::pair<double,double> it = *itr;
		double dist;
		String id = JSON_Data::GetIDfromCoordinate(it);
		try{
			dist = exit_distance - exit_distance_map.at(id);
			m_exit_distance_map[id] = dist;
		}catch(std::out_of_range & oor){
			// std::cout<<"初期位置が交差点です"<<std::endl;
		}
	}
	// if(m_userId + 1 == 18){
	// 	for(auto a : m_exit_distance_map){
	// 		std::cout<<a.first<<", 距離："<<a.second<<std::endl;
	// 	}
	// }



	//////////////////////////////////////route_change_check//////////////////////////////////////////////////
	std::list<std::pair<double,double>> comp_route_list=route_list;
	String comp_exit_node = JSON_Data::GetIDfromCoordinate(target_node);
	comp_route_list.pop_front();

	bool equal = m_shortest.size() == comp_route_list.size() && std::equal(m_shortest.cbegin(), m_shortest.cend(), comp_route_list.cbegin());


	if(!m_shortest.empty()||!GetExitnode().empty()){
		if(equal){
			//std::cout<<"no_change"<<std::endl;

		}else{
			if(comp_exit_node!=GetExitnode()){
				//std::cout<<"change_exit"<<std::endl;
				m_changeexit = m_changeexit + 1;
				m_changeroute = m_changeroute + 1;
			}else{
				//std::cout<<"change_route"<<std::endl;
				m_changeroute = m_changeroute + 1;
			}
		}
	}


	changeBlocked=false;
	changeExit=false;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	SetShortestroute(route_list);
	// std::cout<<"Set Exit!"<<std::endl;
	// std::cout<<JSON_Data::GetIDfromCoordinate(target_node)<<std::endl;
	SetExitnode(JSON_Data::GetIDfromCoordinate(target_node));

	if(JSON_Data::GetIDfromCoordinate(target_node) == GetTargetNodeBuf() || GetTargetNodeBuf() == "first"){
		//目指していた避難所に変更がないときの処理
		// std::cout<<"避難所に変更はありません"<<std::endl;
	}else{
		//目指していた避難所に変更があるときの処理
		std::cout<<"避難所を変更しました"<<std::endl;
		// m_hop.clear();
		std::cout<<GetTargetNodeBuf()<<std::endl;
		//変更前の避難所の移動ユーザの数を変更する処理
		std::map<String, uint32_t> moving_user_num = MyBuilding::GetMovingUserNum();
		uint32_t old_num = moving_user_num.at(GetTargetNodeBuf());
		old_num--;
		moving_user_num.erase(GetTargetNodeBuf());
		moving_user_num.insert(std::make_pair(GetTargetNodeBuf(), old_num));

		//変更後に目指す避難所のユーザ数を増やす処理
		uint32_t new_num = moving_user_num.at(JSON_Data::GetIDfromCoordinate(target_node));
		new_num++;
		moving_user_num.erase(JSON_Data::GetIDfromCoordinate(target_node));
		moving_user_num.insert(std::make_pair(JSON_Data::GetIDfromCoordinate(target_node), new_num));

		//変更を更新
		MyBuilding::SetMovingUserNum(moving_user_num);
	}
//	for(const auto & node : m_shortest){
//		std::cout<<"("<<node.first<<","<<node.second<<")"<<std::endl;
//	}
	SetTargetNodeBuf(JSON_Data::GetIDfromCoordinate(target_node)); //目指していた避難所を保存

	WriteLog::OutputUserTrackingLog(Simulator::Now(),m_userId,m_shortest,exit_distance,m_graph);

	if(no.GetCrossingflag()){
		TaskOfCrossing(no);
	}

}


void MyUser::SocketClose()
{
	SetCloseflag(true);
	m_socket->Close();
}

void MyUser::CompleteEvacuation (String ID)
{
	//MyBuilding::SetExitNodes(); //11/28コメントアウト
	m_evacTime = Simulator::Now() - Seconds(0.1); //避難完了時間を記録
	MyBuilding::PushUserIdCompEvac(m_userId); //避難完了下したユーザノードをリストに記録
	String exitnodeid;
	if(ID == "NONE"){
		exitnodeid=JSON_Data::get_waynodes().at(m_passednode.back()).GetNodeID(); //通過したノードの最後を取得することでexitノードが取得できる
	}else{
		exitnodeid = ID;
		//std::cout<<"ID"<<ID<<std::endl;
	}
	// String exitnodeid=JSON_Data::get_waynodes().at(m_passednode.back()).GetNodeID(); //通過したノードの最後を取得することでexitノードが取得できる
	std::cout<<"避難した避難所ノード："<<exitnodeid<<std::endl;
	uint32_t exitnodeidi=JSON_Data::NodeIDstoi(exitnodeid); //IDをint型に変換
	MyBuilding::ExitNodesPassedUserSet(exitnodeid); //ユーザが避難した避難所の避難可能人数を減らす
	// std::cout<<"ここでエラー2"<<std::endl;
	uint32_t user_space;
	std::map<String,uint32_t> exit_info= MyBuilding::GetExitNodes();
	user_space=exit_info.at(exitnodeid); //残りの避難可能人数を取得
	SetExitInfo(std::make_pair(exitnodeidi,user_space)); //避難所情報として避難所のノードIDと残りの避難可能人数をペアにしてセットする
	std::cout << "m_exitinfopair："<<m_exitinfopair.first<<", "<<m_exitinfopair.second<<std::endl;
	InsertExitnodeInfoList(m_exitinfopair); //更にリストとして管理？？
	m_status=EXIT;
	m_socket->Close();
	m_socket=0;

	WriteLog::OutputText_CompleteTime(m_evacTime,JSON_Data::NodeIDstoi(m_exitnode),m_userId,m_changeroute ,m_changeexit,exitnodeid);
	WriteLog::Evacuationtime(m_userId, starttime); //ユーザごとの避難時間
	WriteLog::OutputTotal_distance(m_userId, total_distance);

	std::cout<<"避難完了人数"<<MyBuilding::GetUserIdCompEvc().size()<<"/"<<MyBuilding::GetNumUsers()+MyBuilding::GetAddUsersNum()<<std::endl;

	if (MyBuilding::GetUserIdCompEvc().size() >= MyBuilding::GetNumUsers()+MyBuilding::GetAddUsersNum()) {
		WriteLog::OutputEscapeInfo(MyBuilding::GetExitNodes()); //ログを出力
		Simulator::Stop(Seconds(1.0));
	}
}

void MyUser::CheckAoI (){
	std::map<String, float> Blocked_info = GetBlockedNodeID_and_time();
	std::map<String, float> Blocked_info_buf = GetBlockedNodeID_and_time();
	for(auto a : Blocked_info){
		double Aoi = (Simulator::Now().GetDouble()/1000000000) - (a.second);
		// std::cout<<"aoi = "<<Aoi<<std::endl;
		if(Aoi > MyBuilding::GetEraseAoI()){
			// std::cout<<"aoiがしきい値以上です"<<std::endl; //debag
			Blocked_info_buf.erase(a.first);
			RestoreGraphInfo(a.first);
		}
	}
	SetBlockedNodeID_and_time2(Blocked_info_buf);
}


void MyUser::EraseHopData(uint32_t key){
	m_hop.erase(key);
	// std::cout<<m_userId<<"：mapからkey:"<<key<<"の情報を削除しました"<<std::endl;
}

void MyUser::EraseHopDataTimer(double delay, uint32_t key){
    // keyに対応するタイマーを取得または作成
	if (hopTimers.find(key) == hopTimers.end()) {
        hopTimers.emplace(key, ns3::Timer());
    }
    hopTimers[key].SetFunction(&MyUser::EraseHopData, this);
	hopTimers[key].SetArguments(key);
    hopTimers[key].Cancel();
    hopTimers[key].Schedule(ns3::Seconds(delay));

}

void MyUser::FirstDijkstraTimer (Time delay)
{
	fdTimer.SetFunction(&MyUser::DijkstraRouteCalculationforGeographicInfo, this);
	fdTimer.SetArguments(m_currentway.GetNodes().front());
	fdTimer.Remove();
	fdTimer.Schedule(delay);
}

void MyUser::HandleUserMovementTimer (Time delay)
{
	humTimer.SetFunction(&MyUser::HandleUserMovement, this);
	humTimer.Remove();
	humTimer.Schedule(delay);
}

int a = 0;
// std::vector<uint32_t> randomnum = MyBuilding::GetRandomNum();
void MyUser::AddUserMovementTimer(Time delay, std::vector<uint32_t> vec)
{
	humTimer.SetFunction(&MyUser::HandleUserMovement, this);
	humTimer.Remove();
	humTimer.Schedule(Seconds(vec[a])+delay);
	starttime = vec[a];
	a++;
}

void MyUser::CompleteEvacuationTimer (Time delay)
{
	ceTimer.SetFunction(&MyUser::CompleteEvacuation, this);
	ceTimer.Remove();
	ceTimer.Schedule(delay);
}

void MyUser::SocketCloseTimer (Time delay)
{
	scTimer.SetFunction(&MyUser::SocketClose, this);
	scTimer.Remove();
	scTimer.Schedule(delay);
}

void MyUser::SetMyUser ()
{
	// set section-id
	m_sectionId = GetSectionIdFromPosition();
}

void MyUser::PrintMyUser ()
{
	// Ptr<MobilityModel> obj = GetObject<MobilityModel>();
	// double posx = obj->GetPosition().x;
	// double posy = obj->GetPosition().y;
	// double posz = obj->GetPosition().z;
	// std::cout << "Print MyUser ----------" 	<< std::endl;
	// std::cout << std::setw(3) << m_userId		<< ", "
	// 		   << std::setw(10)<< posx << "/" << posy << "/" << posz << ", "
	// 		   << std::setw(8) << m_sectionId	<< ", "
	// 		   << std::setw(3) << m_status		<< std::endl;
	// std::cout << "-----------------------" 	<< std::endl;
}

Position MyUser::GetSectionIdFromPosition ()
{
	std::vector<std::vector<MySection>>* mySection = &GetMySection();
	Ptr<MobilityModel> obj = GetObject<MobilityModel>();
	double posx = obj->GetPosition().x;
	double posy = obj->GetPosition().y;
	//double posz = obj->GetPosition().z;
	uint32_t secx;
	uint32_t secy;
	uint32_t secz;
	for (uint32_t x=0; x<MyBuilding::GetNumSectionX(); x++) {
		double left = (*mySection).at(0).at(x).GetLeft();
		double right = (*mySection).at(0).at(x).GetRight();
		if (posx > left and posx < right) {
			secx = x;
			break;
	}}
	for (uint32_t y=0; y<MyBuilding::GetNumSectionY(); y++) {
		double top = (*mySection).at(y).at(0).GetTop();
		double bottom = (*mySection).at(y).at(0).GetBottom();
		if (posy > top and posy < bottom) {
			secy = y;
			break;
	}}
	secz = 0;

	return Position{secx, secy, secz};
}

Vector MyUser::GetNextXY() {
    if (m_patrolRoute.empty()) {
        std::cout<<"ドローン飛行ルートがありません!!!!!!!!"<<std::endl;
        return {0, 0, 20};
    }

    String next_shelter_id = m_patrolRoute[m_currentPatrolIndex];
    std::pair<double,double> latlon = JSON_Data::GetCoordinatefromID(next_shelter_id);
    double x = JSON_Data::json_x(latlon.first);
    double y = JSON_Data::json_y(latlon.second);

    return Vector{x,y,20};
}

// ---------------------------------------------------------------------
// 1. 避難所周回用 (通常モード)
// ---------------------------------------------------------------------
void MyUser::HandleFerryNodeMovement(){
    // ルートが空なら何もしない
    if (m_patrolRoute.empty()) return;

    // -----------------------------------------------------
    // 1. 分岐処理 (情報更新 or ルート変更)
    // -----------------------------------------------------
    // 現在到着しているノードIDを取得
    String arrivedShelter = m_patrolRoute[m_currentPatrolIndex];

    std::map<String, float> m_aoi_blocked_data = GetBlockedNodeID_and_time();
    
    // 分岐: 通行止め情報があるか？
    if(m_aoi_blocked_data.size() != 0){
        // ★通行止めがある場合★
        if(changerouteforUAV == false){
            // まだ通常モードなら、新ルートへ切り替えを試みる
            ChangePatrolRoute(FROMTIMER);
            
            // 注意: ChangePatrolRoute内でルートやインデックスが書き換わっている可能性があります
        }
        // ※ここでは GetExitInfoforFerry を呼び出さない（ご要望通り）
    }
    else{
        // ★通行止めがない場合 (else)★
        // 通常の避難所周回中なので、避難所情報を更新する
        GetExitInfoforFerry(arrivedShelter);
    }

    // -----------------------------------------------------
    // 2. 次の地点へインデックスを進める
    // -----------------------------------------------------
    // ChangePatrolRouteが実行された場合、ルートサイズが変わっている可能性があるため
    // 安全に計算します。
    if (!m_patrolRoute.empty()) {
        m_currentPatrolIndex = (m_currentPatrolIndex + 1) % m_patrolRoute.size();
    } else {
        return; // ルートが空になった場合は終了
    }

    // -----------------------------------------------------
    // 3. 次の目的地へ移動設定
    // -----------------------------------------------------
    Ptr<MobilityModel> obj = GetObject<MobilityModel>();
    Vector vct = obj->GetPosition();
    
    // インデックスが進んだ状態で呼ぶので、正しい「次の場所」が返る
    Vector nvct = GetNextXY(); 

    double dist = CalculateDistance(vct, nvct);

    // ★安全対策 (必須): 距離0（同じ場所）なら移動設定をスキップして、すぐに次の処理へ移る
    if (dist < 0.5) {
        // わずかな時間待機して、再度この関数を呼ぶことで次のインデックスへ進む
        HandleFerryMovementTimer(Seconds(0.1));
        return;
    }

    double speed = MyBuilding::GetFerrySpeed();
    double arrival = dist / speed;

    Ptr<WaypointMobilityModel> ferry = GetObject<WaypointMobilityModel>();
    double now = Simulator::Now().GetSeconds();
    
    Waypoint wpt(Seconds(now + arrival), nvct);
    ferry->AddWaypoint(wpt);

    // 次の移動タイマー
    HandleFerryMovementTimer(Seconds(arrival) - NanoSeconds(1));
}

// ---------------------------------------------------------------------
// 2. ブロック周回用 (新ルートモード)使ってない
// ---------------------------------------------------------------------
void MyUser::HandleFerryNodeMovementAroundBlock(){
    // ルートが空なら何もしない
    if (m_patrolRoute.empty()) return;

    // A. 到着地点の情報更新 (必要に応じて)
    // -----------------------------------------------------------------
    // ブロック周回ルートのノードも避難所の可能性があるため、念のため更新を試みる
    // もし避難所以外なら GetExitInfoforFerry 内でエラーになる可能性があるなら
    // 事前に避難所IDリストに含まれるかチェックが必要ですが、既存通り呼び出します。
    String arrivedNode = m_patrolRoute[m_currentPatrolIndex];
    // GetExitInfoforFerry(arrivedNode); // ※必要ならコメントアウト解除

    // B. 次の地点へインデックスを進める
    // -----------------------------------------------------------------
    m_currentPatrolIndex = (m_currentPatrolIndex + 1) % m_patrolRoute.size();

    // C. 次の目的地へ移動設定
    // -----------------------------------------------------------------
    Ptr<MobilityModel> obj = GetObject<MobilityModel>();
    Vector vct = obj->GetPosition();
    Vector nvct = GetNextXY(); 

    double dist = CalculateDistance(vct, nvct);

    // // ★安全対策 (必須): 距離0の場合はスキップ★
    // if (dist < 0.5) {
    //     HandleFerryMovementAroundBlockTimer(Seconds(0.1));
    //     return;
    // }

    double speed = MyBuilding::GetFerrySpeed();
    double arrival = dist / speed;

    Ptr<WaypointMobilityModel> ferry = GetObject<WaypointMobilityModel>();
    double now = Simulator::Now().GetSeconds();
    
    Waypoint wpt(Seconds(now + arrival), nvct);
    ferry->AddWaypoint(wpt);

    // 次の移動タイマー (ブロック周回用)
    HandleFerryMovementAroundBlockTimer(Seconds(arrival) - NanoSeconds(1));
}

void MyUser::ChangePatrolRoute(uint32_t from){
    std::map<Pos, way_node> wnl = JSON_Data::get_waynodes();
    std::vector<String> new_patrolRoute;
    std::map<String, float> m_aoi_blocked_data = GetBlockedNodeID_and_time();
    std::map<String, uint32_t> m_ereaid_time = GetPatrolErea_and_Time();
    
    // 現在の時刻
    double my_change_time_sec = Simulator::Now().GetDouble() / 1000000000.0;


    // =================================================================
    // ★追加修正: 現在担当中のエリアがある場合、継続判定を行う
    // =================================================================
    if (changerouteforUAV) {
        String currentTarget = my_patrol_erea_and_start_time.first;
        double myStartTime = my_patrol_erea_and_start_time.second;

        // 1. そのターゲットはまだ通行止めリストにあるか？（解消されていないか？）
        bool stillBlocked = false;
        for(auto &b : m_aoi_blocked_data){
            if(b.first == currentTarget){
                stillBlocked = true;
                break;
            }
        }

        if (stillBlocked) {
            // 2. 競合チェック: 他のUAVにもっと早く取られていないか？
            bool lostPriority = false;
            if (m_ereaid_time.count(currentTarget)) {
                uint32_t other_time = m_ereaid_time.at(currentTarget);
                // 相手が有効な時間を持ち、かつ自分より早い(小さい)場合
                // ※ myStartTimeと比較する際は型に注意。
                //   相手の情報が更新され、自分より早い記録が来たら譲る
                if (other_time > 0 && other_time < (uint32_t)myStartTime) {
                    std::cout << "ALERT: 現在担当中のエリア " << currentTarget 
                              << " は他のUAVが優先されました。担当を外れます。" << std::endl;
                    lostPriority = true;
                }
            }

            if (!lostPriority) {
                // まだ通行止めで、かつ優先権も持っている -> 【現状維持】
                // 何も変更せずにリターンする（再探索しない）
                // std::cout << "DEBUG: 現在の任務(" << currentTarget << ")を継続します" << std::endl;
                return; 
            }
            // 優先権を失った場合は、下流の処理へ進み、新しいターゲットを探すか元に戻る
        } else {
            // 通行止めが解消された -> 下流の処理へ進み、新しいターゲットを探すか元に戻る
            std::cout << "INFO: 担当エリア " << currentTarget << " の通行止めは解消されました。" << std::endl;
        }
    }
    // =================================================================


    String targetid = "";
    bool foundValidTarget = false;

    // ----------------------------------------------------------------
    // 1. 全ての通行止め情報を走査し、「担当すべきエリア」を探す
    // ----------------------------------------------------------------
    for(auto &b : m_aoi_blocked_data){
        String candidateId = b.first;

        // --- Check A: 既に他のUAVの「担当エリアリスト」に入っているか？ ---
        bool isAssigned = false;
        for(auto &a : other_UAV_patrol_erea_and_id){
            if(candidateId == a.first){
                isAssigned = true;
                break;
            }
        }

        if(isAssigned){
            continue; 
        }

        // --- Check B: 時間的な競合で負けていないか？ ---
        bool lostRace = false;
        if (m_ereaid_time.count(candidateId)) {
            uint32_t other_uav_start_time_sec = m_ereaid_time.at(candidateId);
            
            if (other_uav_start_time_sec > 0 && other_uav_start_time_sec < (uint32_t)my_change_time_sec) {
                // ここはデバッグログがうるさければコメントアウトでも可
                // std::cout << "DEBUG: 候補エリア負け: " << candidateId << std::endl;
                lostRace = true;
            }
        }

        if(lostRace){
            continue;
        }

        // --- 採用決定 ---
        targetid = candidateId;
        foundValidTarget = true;
        std::cout << targetid << " を新しい巡回ターゲットとして決定しました" << std::endl;
        break; 
    }


    // ----------------------------------------------------------------
    // 2. ターゲットが見つからなかった場合
    // ----------------------------------------------------------------
    if(!foundValidTarget){
        // ★修正: 既に別ルートを飛んでいた(changerouteforUAV==true)のに、
        // ここに来たということは「担当エリアが解消された」か「優先権を奪われた」場合のみ。
        // なので、堂々と元のルートに戻して良い。
        
        m_patrolRoute = m_old_patrolRoute;
        changerouteforUAV = false; // フラグを下ろす
        my_patrol_erea_and_start_time = {"", 0.0}; // 情報クリア

        if(from == FROMTIMER){
            std::cout << "DEBUG: 担当すべきエリアがないため、元のルートを巡回します" << std::endl;
            GetExitInfoforFerry(m_patrolRoute[(m_currentPatrolIndex - 1) % m_patrolRoute.size()]);
        }else if(from == FROMRECV){
            std::cout << "DEBUG: 有効なターゲットがありません（元のルートへ復帰）" << std::endl;
        }
        return;
    }


    // ----------------------------------------------------------------
    // 3. ルート計算
    // ----------------------------------------------------------------
    std::pair<double,double> blockcoordinate = JSON_Data::GetCoordinatefromID(targetid);
    
    for(auto b : wnl){

		if (!b.second.GetCrossingflag()) continue; //交差点のみ飛行する処理
        const std::vector<std::pair<double,double>>& path = b.second.GetShortestPath();
        if (path.empty()) continue;

        for(auto const& c : path){
            if(c == blockcoordinate){
                new_patrolRoute.push_back(b.second.GetNodeID());
                break; 
            }
        }
    }
    
    // ----------------------------------------------------------------
    // 4. パトロールルートの適用
    // ----------------------------------------------------------------
    if(new_patrolRoute.size() != 0){
        m_patrolRoute = new_patrolRoute;
        changerouteforUAV = true;

		// ★★★ ルートの並べ替え (Nearest Neighbor) ★★★
        std::vector<String> sorted_route;
        std::pair<double, double> current_pos = blockcoordinate; 
        std::vector<String> unsorted = new_patrolRoute;
        
        while (!unsorted.empty()) {
            double min_dist = 99999999.9; // infのかわり
            int nearest_idx = -1;

            for (size_t i = 0; i < unsorted.size(); ++i) {
                std::pair<double, double> node_pos = JSON_Data::GetCoordinatefromID(unsorted[i]);
                double d = JSON_Data::calculate_distance(current_pos, node_pos);
                if (d < min_dist) {
                    min_dist = d;
                    nearest_idx = i;
                }
            }

            if (nearest_idx != -1) {
                sorted_route.push_back(unsorted[nearest_idx]);
                current_pos = JSON_Data::GetCoordinatefromID(unsorted[nearest_idx]);
                unsorted.erase(unsorted.begin() + nearest_idx);
            } else {
                break;
            }
        }
        m_patrolRoute = sorted_route;
        
        std::cout << "飛行ルートを切り替えます: Target " << targetid << std::endl;
        
        // ★ 自分の開始時間を記録
        my_patrol_erea_and_start_time = {targetid, my_change_time_sec};
        
        std::cout << "Change Patrol Route: ";
        for(const auto& s : m_patrolRoute) std::cout << s << " ";
        std::cout << std::endl;
    }else{
        std::cout << "DEBUG: ターゲットへの影響ノードなし。元のルートを維持します。" << std::endl;
        m_patrolRoute = m_old_patrolRoute;
        changerouteforUAV = false; // フラグを下ろす
        GetExitInfoforFerry(m_patrolRoute[(m_currentPatrolIndex - 1) % m_patrolRoute.size()]);
    }
}



// void MyUser::HandleFerryNodeMovementAroundBlock(){
// 	Ptr<MobilityModel> obj = GetObject<MobilityModel>();

// 	double posx = obj->GetPosition().x;
// 	double posy = obj->GetPosition().y;
// 	double posz = obj->GetPosition().z;
// 	Vector vct(posx, posy, posz);

// 	Vector nvct = GetNextXY();
// 	double dist = CalculateDistance(vct, nvct);
// 	double speed = MyBuilding::GetFerrySpeed();
// 	double arrival = dist / speed;
// 	double now = Simulator::Now().GetDouble() / 1000000000;
// 	double wpTime = now + arrival;
// 	Waypoint wpt(Seconds(wpTime), nvct);
// 	Ptr<WaypointMobilityModel> ferry = GetObject<WaypointMobilityModel>();
// 	ferry->AddWaypoint(wpt);
// 	// std::cout<<"arrival="<<arrival<<std::endl;
// 	if(arrival==0){
// 		HandleFerryMovementAroundBlockTimer(Seconds(arrival));
// 		return;
// 	}else{
// 		HandleFerryMovementAroundBlockTimer(Seconds(arrival) - NanoSeconds(1));
// 		return;
// 	}


// }


void MyUser::GetExitInfoforFerry(String id){
	std::map<String,uint32_t> exit_info= MyBuilding::GetExitNodes();
	// for(auto a : exit_info){
	// 	std::cout<<a.first<<std::endl;
	// }
	uint32_t user_space=exit_info.at(id); //残りの避難可能人数を取得
	uint32_t time = Simulator::Now().GetInteger()/1000000000;


	std::map <String, std::pair<uint32_t, uint32_t>> old_data = GetAoiExitinfo(); //避難所ID、収容可能人数、時間

	std::pair<uint32_t, uint32_t> canexitnum_and_time = old_data.at(id);
	std::map <String, std::pair<uint32_t, uint32_t>> new_Aoi_exitinfo;
	if(canexitnum_and_time.second < time){
		std::cout<<id<<"の避難所情報を強制更新"<<std::endl;
		// std::cout<<"所持データの生成された時間："<<canexitnum_and_time.second<<", 受信したデータの生成された時間："<<Time<<std::endl;
		std::pair<uint32_t, uint32_t> new_canexitnum_and_time = std::make_pair(user_space, time);
		// new_Aoi_exitinfo.insert(std::make_pair(s_shelterID, new_canexitnum_time));
		// user->EraseAoiExitinfo(s_shelterID);
		old_data.erase(id);
		old_data.insert(std::make_pair(id, new_canexitnum_and_time));
		SetAoiExitinfo(old_data);
		double increase_rate = double(canexitnum_and_time.first - user_space) / double(time - canexitnum_and_time.second); //1秒あたりの増加率
		SetIncreaseRate(id, increase_rate);
	}else{
		std::cout<<"FerryNodeID = "<< GetUserId() + 1 <<", フェリーノードが避難所から古いデータを受信しました。(実行されるわけない)"<<std::endl;
	}
}

void MyUser::HandleFerryMovementTimer (Time delay)
{
	ferryTimer.SetFunction(&MyUser::HandleFerryNodeMovement, this);
	ferryTimer.Remove();
	ferryTimer.Schedule(delay);
}

void MyUser::HandleFerryMovementAroundBlockTimer (Time delay)
{
	aroundblockferryTimer.SetFunction(&MyUser::HandleFerryNodeMovementAroundBlock, this);
	aroundblockferryTimer.Remove();
	aroundblockferryTimer.Schedule(delay);
}

}}
