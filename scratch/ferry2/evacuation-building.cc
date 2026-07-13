
/*
 * simple-building.cc
 *
 *  Created on: 2019/12/25
 *      Author: fujinaka
 */

#include "evacuation-building.h"
#include "json_data.h"
#include "DTN-protocol.h"
#include <string>
#include <fstream>
#include <iostream>
#include <set>
#include "inicpp.h"

namespace ns3 {
namespace simple {
//基本的にはsetting.iniで記述
double MyBuilding::fieldWidth = 5000.0;
double MyBuilding::fieldHight = 5000.0;
const uint32_t MyBuilding::numSections = 25;
const uint32_t MyBuilding::numSectionX = 5;
const uint32_t MyBuilding::numSectionY = 5;
const uint32_t MyBuilding::numSectionZ = 1;
const double MyBuilding::sectionWidth = fieldWidth / numSectionX;
const double MyBuilding::sectionHight = fieldHight / numSectionY;
const double MyBuilding::PenetrationRate = 100.0;
uint32_t MyBuilding::numUsers = 50;
uint32_t MyBuilding::addUsers = 100;
uint32_t MyBuilding::numferrys = 1;
double MyBuilding::userSpeed = 1.6;
double MyBuilding::ferrySpeed = 5.0;
double MyBuilding::HelloInterval = 20.0;
// double MyBuilding::ShelterHelloInterval = 60.0;
double MyBuilding::ShelterDataInterval = 60.0;
double MyBuilding::UpdateMapinfoInterval = 50.0;
bool MyBuilding::blockflag = true;
bool MyBuilding::blockeraseflag = true;
bool MyBuilding::randomblockeraseflag = false;
bool MyBuilding::setexitAutoflag = true;
bool MyBuilding::setexitmanualflag = true;
std::list<String> MyBuilding::blocked_nodes;
std::list<String> MyBuilding::blocked_nodes2;
std::list<String> MyBuilding::erase_nodes;
std::map<String, uint32_t> MyBuilding::exit_nodes;
std::map<String, uint32_t> MyBuilding::first_exit_info;
std::set<String> MyBuilding::blockset_buf;
uint32_t MyBuilding::neary = 150;
std::string MyBuilding::field_name;
std::string MyBuilding::SimulationFieldMap; //使ってない
std::string MyBuilding::hinanjo_json;
bool MyBuilding::randomuserSpeedflag = false;
double MyBuilding::MaxSpeed = 2.0;
double MyBuilding::MinSpeed = 1.0;
bool MyBuilding::useexitinfo = false;
bool MyBuilding::UseDisasterType = false;
uint32_t MyBuilding::Disaster_Type = 1;
uint32_t MyBuilding::AoI = 60;
bool MyBuilding::QuickSend = true;
double MyBuilding::QuickHelloInterval = 5.0;
double MyBuilding::AddUserInterval = 1.0;
double MyBuilding::moving_user_check_interval = 1.0;
uint32_t MyBuilding::EraseAoI = 300;
//uint32_t MyBuilding::UserDataSize = 128;
uint32_t MyBuilding::quick_hello_times = 3;
String MyBuilding::ferry_start;
double MyBuilding::ferry_hellointerval = 5;
bool MyBuilding::auto_ferry_num = false;

uint32_t MyBuilding::num = 0;
std::set<uint32_t> MyBuilding::number;

std::vector<uint32_t> MyBuilding::vect;
std::map<String, uint32_t> MyBuilding::moving_user_num; //各避難所ごとに現在動いているユーザ数のmap

std::set<uint32_t> MyBuilding::activeUsers;

uint32_t MyBuilding::send_hello_num = 0;
uint32_t MyBuilding::send_hello_bytes = 0;
uint32_t MyBuilding::recv_hello_num = 0;
uint32_t MyBuilding::recv_hello_bytes = 0;
uint32_t MyBuilding::send_ack_hello_num = 0;
uint32_t MyBuilding::recv_ack_hello_num = 0;

uint32_t MyBuilding::send_request_num = 0;
uint32_t MyBuilding::send_request_bytes = 0;
uint32_t MyBuilding::recv_request_num = 0;
uint32_t MyBuilding::recv_request_bytes = 0;

uint32_t MyBuilding::send_userdata_num = 0;
uint32_t MyBuilding::send_userdata_bytes = 0;
uint32_t MyBuilding::recv_userdata_num = 0;
uint32_t MyBuilding::recv_userdata_bytes = 0;

uint32_t MyBuilding::near_shelter_recv_hello_num = 0;
uint32_t MyBuilding::near_shelter_recv_request_num = 0;
uint32_t MyBuilding::near_shelter_recv_userdata_num = 0;

//2026-04-14
uint32_t MyBuilding::addUsersPerInterval = 1;
double MyBuilding::addUserStartTime = 0.0;
double MyBuilding::addUserEndTime = 0.0;
double MyBuilding::measureStartTime = 0.0;
double MyBuilding::measureEndTime = 0.0;
uint32_t MyBuilding::nextAddUserIndex = 0;

uint32_t MyBuilding::seed = 0;
uint32_t MyBuilding::run = 0;
uint32_t MyBuilding::count = 0;

std::list<String> MyBuilding::rescue_nodes;
std::list<String> MyBuilding::erase_rescue_nodes;
std::list<String> MyBuilding::congestion_nodes;
std::list<String> MyBuilding::erase_congestion_nodes;

bool MyBuilding::set_rescue_flag = false;
bool MyBuilding::erase_rescue_flag = false;
bool MyBuilding::set_congestion_flag = false;
bool MyBuilding::erase_congestion_flag = false;
bool MyBuilding::send_hop_flag = false;

std::map<String, double> MyBuilding::max_distance_to_exit;
std::map<std::string, std::map<std::string, double>> MyBuilding::exit_node_distances;
std::vector<std::string> MyBuilding::m_patrolRoute = {};

std::vector<std::vector<std::string>> MyBuilding::sectionLink
		   {{"00","10"},{"10","20"},			{"30","40"},
			{"00","01"},			{"20","21"},{"30","31"},{"40","41"},
			{"01","11"},{"11","21"},{"21","31"},
			{"01","02"},									{"41","42"},
			{"02","12"},{"12","22"},{"22","32"},{"32","42"},
						{"12","13"},{"22","23"},{"32","33"},
			{"03","13"},			{"23","33"},{"33","43"},
			{"03","04"},			{"23","24"},{"33","34"},{"43","44"},
			{"04","14"},{"14","24"},			{"34","44"}				};
std::list<Position> MyBuilding::exit {{0,4,0},{4,4,0}};
std::list<Position> MyBuilding::blocked {{1,1,0}};
std::map<std::pair<Position, Position>, uint32_t> MyBuilding::exitDoor {{{{0,3,0},{0,4,0}},1}, {{{1,4,0},{0,4,0}},2}, {{{3,4,0},{4,4,0}},3}, {{{4,3,0},{4,4,0}},4}};

std::map<uint32_t, std::queue<uint32_t>> MyBuilding::holdUserId {};
std::map<uint32_t, Time> MyBuilding::waitTime {};

std::vector<uint32_t> MyBuilding::userIdCompletedEvacuation {};
std::map<uint32_t, std::list<uint32_t>> MyBuilding::userIdPassedExitDoor {{1,{}}, {2,{}}, {3,{}}, {4,{}}};

Time MyBuilding::CalculationWaitTime (uint32_t userId, Position front, Position exit)
{
	uint32_t doorId = exitDoor.at(std::make_pair(front, exit));
	holdUserId[doorId].push(userId);
	if (holdUserId.at(doorId).size() == 1) {
		waitTime[doorId] = Simulator::Now() + Seconds(1.0/MyBuilding::GetPenetrationRate());
	}
	else if (holdUserId.at(doorId).size() > 1) {
		waitTime[doorId] += Seconds(1.0/MyBuilding::GetPenetrationRate());
	}
	Time delay = waitTime[doorId] - Simulator::Now();

	return waitTime[doorId];
}

std::ostream& operator<< (std::ostream& os, const Position& pos)
{
	os << pos.x << "/" << pos.y << "/" << pos.z;
	return os;
}

void MyBuilding::SettingLoad(){
	ini::IniFile myIni;
	myIni.load("obayashiIOFiles/setting.ini");

	fieldWidth = myIni["Setting1"]["FieldWidth"].as<double>();
	fieldHight = myIni["Setting1"]["FieldHight"].as<double>();
	field_name = myIni["Setting1"]["field_name"].as<String>();

	numUsers = myIni["Setting1"]["Num_user"].as<uint32_t>();
	addUsers = myIni["Setting1"]["Add_user"].as<uint32_t>(); // �ǉ����̑���

	addUsersPerInterval = myIni["Setting1"]["AddUsersPerInterval"].as<uint32_t>();
	addUserStartTime = myIni["Setting1"]["AddUserStartTime"].as<double>();
	addUserEndTime = myIni["Setting1"]["AddUserEndTime"].as<double>();
	AddUserInterval = myIni["Setting1"]["AddUserInterval"].as<double>();

	measureStartTime = myIni["Setting1"]["MeasureStartTime"].as<double>();
	measureEndTime = myIni["Setting1"]["MeasureEndTime"].as<double>();

	nextAddUserIndex = 0;

	numferrys = myIni["Setting1"]["Num_ferry"].as<uint32_t>();
	userSpeed = myIni["Setting1"]["UserSpeed"].as<double>();
	HelloInterval = myIni["Setting1"]["HelloInterval"].as<double>();
	ShelterDataInterval = myIni["Setting1"]["ShelterDataInterval"].as<double>();
	UpdateMapinfoInterval = myIni["Setting1"]["UpdateMapinfoInterval"].as<double>();

	blockflag = myIni["Setting1"]["blockflag"].as<bool>();
	blockeraseflag = myIni["Setting1"]["blockeraseflag"].as<bool>();
	randomblockeraseflag = myIni["Setting1"]["randomblockeraseflag"].as<bool>();
	setexitAutoflag = myIni["Setting1"]["setexitAutoflag"].as<bool>();
	setexitmanualflag = myIni["Setting1"]["setexitmanualflag"].as<bool>();
	neary =  myIni["Setting1"]["setnearly"].as<uint32_t>();

	SimulationFieldMap = myIni["json_data"]["SimulationFieldMap"].as<String>();
	hinanjo_json = myIni["json_data"]["hinanjo_json"].as<String>();

	randomuserSpeedflag = myIni["Setting1"]["RandomUserSpeedFlag"].as<bool>();
	MinSpeed = myIni["Setting1"]["MinSpeed"].as<double>();
	MaxSpeed = myIni["Setting1"]["MaxSpeed"].as<double>();

	UseDisasterType = myIni["Setting1"]["UseDisasterType"].as<bool>();
	Disaster_Type = myIni["Setting1"]["Disaster_Type"].as<uint32_t>();

	AoI = myIni["Setting1"]["AoI"].as<uint32_t>();
	QuickSend = myIni["Setting1"]["UseQuickSend"].as<bool>();
	QuickHelloInterval = myIni["Setting1"]["QuickHelloInterval"].as<double>();

	//UserDataSize = myIni["Setting1"]["UserDataSize"].as<uint32_t>();


	moving_user_check_interval = myIni["Setting1"]["moving_user_check_interval"].as<double>();
	quick_hello_times = myIni["Setting1"]["quick_hello_times"].as<uint32_t>();

	set_congestion_flag = myIni["Setting1"]["Set_congestion_flag"].as<bool>();
	erase_congestion_flag = myIni["Setting1"]["Erase_congestion_flag"].as<bool>();
	set_rescue_flag = myIni["Setting1"]["Set_rescue_flag"].as<bool>();
	erase_rescue_flag = myIni["Setting1"]["Erase_rescue_flag"].as<bool>();

	send_hop_flag = myIni["Setting1"]["SendHopFlag"].as<bool>();

	ferry_start = myIni["Setting1"]["Ferry_start_node"].as<String>();
	ferrySpeed = myIni["Setting1"]["FerryNodeSpeed"].as<double>();
	ferry_hellointerval = myIni["Setting1"]["Ferry_HelloInterval"].as<double>();
	auto_ferry_num = myIni["Setting1"]["Auto_ferry_num"].as<bool>();

	
	// EraseAoI = myIni["Setting1"]["eraseAoI"].as<uint32_t>();

	if(blockflag == false) blockeraseflag = false;
	if(setexitmanualflag == false) setexitAutoflag = true;
	if(setexitAutoflag == false) UseDisasterType = false;
	if(set_congestion_flag == false) erase_congestion_flag = false;
	if(set_rescue_flag == false) erase_rescue_flag = false;

	std::cout << "InitialUsers = " << numUsers << std::endl;
	std::cout << "ReservedAddUsers = " << addUsers << std::endl;
	std::cout << "AddUsersPerInterval = " << addUsersPerInterval << std::endl;
	std::cout << "AddUserStartTime = " << addUserStartTime << std::endl;
	std::cout << "AddUserEndTime = " << addUserEndTime << std::endl;
	std::cout << "AddUserInterval = " << AddUserInterval << std::endl;
	std::cout << "MeasureStartTime = " << measureStartTime << std::endl;
	std::cout << "MeasureEndTime = " << measureEndTime << std::endl;
	std::cout << "MinSpeed = " << MinSpeed << ", MaxSpeed = " << MaxSpeed << std::endl;

	//std::cout << "UserDataSize = "
          //<< UserDataSize
          //<< std::endl;

}

// Active User 



	void MyBuilding::AddActiveUser(uint32_t userId)
	{
		activeUsers.insert(userId);
	}

	void MyBuilding::RemoveActiveUser(uint32_t userId)
	{
		activeUsers.erase(userId);
	}

	bool MyBuilding::IsActiveUser(uint32_t userId)
	{
		return activeUsers.find(userId) != activeUsers.end();
	}

	uint32_t MyBuilding::GetActiveUserNum()
	{
		return activeUsers.size();
	}

	std::set<uint32_t> MyBuilding::GetActiveUsers()
	{
		return activeUsers;
	}


void MyBuilding::SetBlockedNodes2(){
	std::string str2 = "./obayashiIOFiles/data/" + field_name + "/block_nodes_data/set/blocked_nodes_ALL.txt";
	std::ifstream file(str2);  // 読み込むファイルのパスを指定
	// std::cout<<"以下のブロックノードを追加しました。 time："<<time<<std::endl;
    std::string line;
    while (std::getline(file, line)) {  // 1行ずつ読み込む
        // std::cout << line << std::endl;
		blocked_nodes2.push_back(line);
    }
}


//ブロックノード指定
void MyBuilding::SetBlockedNodes(uint32_t update_num){
	// uint32_t update_num = JSON_Data::GetUpdateNum();
	uint32_t time = MyBuilding::GetUpdateMapinfoInterval() * update_num; 
	// std::cout<<"time："<<num<<std::endl;
	std::string str2 = "./obayashiIOFiles/data/" + field_name + "/block_nodes_data/set/blocked_nodes_" + std::to_string(time) + ".txt";
	std::ifstream file(str2);  // 読み込むファイルのパスを指定
	if(!file){
		std::cout<<"追加のブロックファイルはありません。 time = "<<time<<std::endl;
		blocked_nodes.clear();
		return;
	}
	// std::cout<<"以下のブロックノードを追加しました。 time："<<time<<std::endl;
    std::string line;
    while (std::getline(file, line)) {  // 1行ずつ読み込む
        // std::cout << line << std::endl;
		blocked_nodes.push_back(line);
		std::pair<double,double> coordinate = JSON_Data::GetCoordinatefromID(line);
		std::cout<<line<<", "<<coordinate.first<<", "<<coordinate.second<<std::endl;
    }

	////////////////////////////////Logファイル出力////////////////////////////////////////////////
	String mode="ON";
	if(RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
		return;
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
	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(MyBuilding::GetNumUsers()) + "/Info/Block/BlockNode_" +std::to_string(simple::MyBuilding::GetRun()) +".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::app);
	fout << "time = " << time << std::endl;
	for(auto a : blocked_nodes){
		fout << a <<std::endl;
	}
	fout << std::endl;
	fout.close();
}

void MyBuilding::EraseBlockedNodes(uint32_t update_num){
	uint32_t time = MyBuilding::GetUpdateMapinfoInterval() * update_num; 
	// std::cout<<"time："<<time<<std::endl;
	std::string str2 = "./obayashiIOFiles/data/"+ field_name +"/block_nodes_data/erase/erase_nodes_" + std::to_string(time) + ".txt";
	std::ifstream file(str2);  // 読み込むファイルのパスを指定
	if(!file){
		std::cout<<"削除するブロックノードが記述されたファイルはありません"<<std::endl;
		return;
	}
	std::cout<<"以下のブロックノードを解除しました。 time = "<<time<<std::endl;
    std::string line;
    while (std::getline(file, line)) {  // 1行ずつ読み込む
        // std::cout << line << std::endl;
		blocked_nodes.remove(line);
		erase_nodes.push_back(line);
    }
	for(auto a : erase_nodes){
		std::cout<<a<<", ";
	}
	std::cout<<std::endl;
}

void MyBuilding::SetExitNodes(){
	std::string file_name = "obayashiIOFiles/data/"+ field_name +"/exit_nodes.txt";
	std::ifstream file(file_name);  // 読み込むファイルのパスを指定
    std::string buf;
	std::string line;
	std::string line2;
	u_int32_t num;
	// std::getline(file,line);
 	// std::cout << line << std::endl;
	std::string delimiter = ",";
    std::vector<std::string> words{};

    size_t pos;
    while (std::getline(file, buf)) {  // 1行ずつ読み込む
		while ((pos = buf.find(delimiter)) != std::string::npos) {
			line = buf.substr(0, pos);
			// std::cout<<"line:"<<line<<std::endl;
			buf.erase(0, pos + delimiter.length());
			line2 = buf;
			// std::cout<<"line2:"<<line2<<std::endl;
			num = std::stoi(line2);
    	}
		exit_nodes.insert(std::make_pair(line, num));
		first_exit_info.insert(std::make_pair(line, num));
		max_distance_to_exit.insert(std::make_pair(line, 0));
        // std::cout << exit_nodes.size() << std::endl;
    }
}

void MyBuilding::SetExitNodesAuto(std::string nodeID, uint32_t exitnum){
	exit_nodes.insert(std::make_pair(nodeID, exitnum));
	first_exit_info.insert(std::make_pair(nodeID, exitnum));
	max_distance_to_exit.insert(std::make_pair(nodeID, 0));
	std::cout<<"自動で避難所を設定しました。"<<std::endl;
}

void MyBuilding::SetRandomNum(){
	for(uint32_t i = 0; i < GetAddUsersNum(); i++){
		// std::cout<<num<<std::endl;
		vect.push_back(num);
		num = num + AddUserInterval;
	}
	std::mt19937 get_rand_mt(RngSeedManager::GetSeed());
	// vect(number.begin(), number.end());
	std::shuffle(vect.begin(), vect.end(), get_rand_mt);
}

void MyBuilding::SetRescueNode(uint32_t update_num){
	uint32_t time = MyBuilding::GetUpdateMapinfoInterval() * update_num;
	std::string str2 = "./obayashiIOFiles/data/" + field_name + "/rescue_nodes_data/set/rescue_nodes_" + std::to_string(time) + ".txt";
	std::ifstream file(str2);  // 読み込むファイルのパスを指定
	if(!file){
		std::cout<<"追加の要救助者がいる場所はありません。 time = "<<time<<std::endl;
		rescue_nodes.clear();
		return;
	}
	
    std::string line;
	bool flag = true;
    while (std::getline(file, line)) {  // 1行ずつ読み込む
        // std::cout << line << std::endl;
		if(flag){
			std::cout<<"以下の要救助者がいるノードを追加しました． time = "<<time<<std::endl;
			flag = false;
		}
		rescue_nodes.push_back(line);
		std::pair<double,double> coordinate = JSON_Data::GetCoordinatefromID(line);
		std::cout<<line<<", "<<coordinate.first<<", "<<coordinate.second<<std::endl;
    }
}

void MyBuilding::EraseRescueNode(uint32_t update_num){
	uint32_t time = MyBuilding::GetUpdateMapinfoInterval() * update_num; 
	// std::cout<<"time："<<time<<std::endl;
	std::string str2 = "./obayashiIOFiles/data/"+ field_name +"/rescue_nodes_data/erase/erase_rescue_nodes_" + std::to_string(time) + ".txt";
	std::ifstream file(str2);  // 読み込むファイルのパスを指定
	if(!file){
		std::cout<<"削除する要救助者ノードが記述されたファイルはありません"<<std::endl;
		return;
	}

    std::string line;
	bool flag = true;
    while (std::getline(file, line)) {  // 1行ずつ読み込む
        // std::cout << line << std::endl;
		if(flag){
			std::cout<<"以下の要救助者ノードを削除しました． time = "<<time<<std::endl;
			flag = false;
		}
		rescue_nodes.remove(line);
		erase_rescue_nodes.push_back(line);
    }
	for(auto a : rescue_nodes){
		std::cout<<a<<", ";
	}
	std::cout<<std::endl;
}

void MyBuilding::SetCongestionNode(uint32_t update_num){
	uint32_t time = MyBuilding::GetUpdateMapinfoInterval() * update_num;
	std::string str2 = "./obayashiIOFiles/data/" + field_name + "/congestion_nodes_data/set/congestion_nodes_" + std::to_string(time) + ".txt";
	std::ifstream file(str2);  // 読み込むファイルのパスを指定
	if(!file){
		std::cout<<"追加の混雑箇所はありません． time = "<<time<<std::endl;
		congestion_nodes.clear();
		return;
	}
	
    std::string line;
	bool flag = true;
    while (std::getline(file, line)) {  // 1行ずつ読み込む
        // std::cout << line << std::endl;
		if(flag){
			std::cout<<"以下の混雑箇所を追加しました． time = "<<time<<std::endl;
			flag = false;
		}
		congestion_nodes.push_back(line);
		std::pair<double,double> coordinate = JSON_Data::GetCoordinatefromID(line);
		std::cout<<line<<", "<<coordinate.first<<", "<<coordinate.second<<std::endl;
    }
}

void MyBuilding::EraseCongestionNode(uint32_t update_num){
	uint32_t time = MyBuilding::GetUpdateMapinfoInterval() * update_num; 
	// std::cout<<"time："<<time<<std::endl;
	std::string str2 = "./obayashiIOFiles/data/"+ field_name +"/congestion_nodes_data/erase/erase_congestion_nodes_" + std::to_string(time) + ".txt";
	std::ifstream file(str2);  // 読み込むファイルのパスを指定
	if(!file){
		std::cout<<"削除する混雑ノードが記述されたファイルはありません"<<std::endl;
		return;
	}

    std::string line;
	bool flag = true;
    while (std::getline(file, line)) {  // 1行ずつ読み込む
        // std::cout << line << std::endl;
		if(flag){
			std::cout<<"以下の混雑ノードを削除しました． time = "<<time<<std::endl;
			flag = false;
		}
		congestion_nodes.remove(line);
		erase_congestion_nodes.push_back(line);
    }
	for(auto a : congestion_nodes){
		std::cout<<a<<", ";
	}
	std::cout<<std::endl;
}

void MyBuilding::WriteActiveUserLog()
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
        "obayashiIOFiles/Log/obayashi/" + mode + "/" +
        quickhello + "/" + eraseblock + "/" +
        std::to_string(simple::MyBuilding::GetNumUsers()) +
        "/Info/ActiveUser_" +
        std::to_string(simple::MyBuilding::GetRun()) + ".txt";

    std::ofstream fout;
    fout.open(path, std::ios::app);

    if (fout.tellp() == 0) {
        fout << "time,active_user" << std::endl;
    }

    double now = Simulator::Now().GetSeconds();
    fout << now << " " << MyBuilding::GetActiveUserNum() << std::endl;
    fout.close();

    Simulator::Schedule(Seconds(10.0), &MyBuilding::WriteActiveUserLog);
}

void MyBuilding::Write_Logfile_for_simulation_environment(){
	std::cout<<"Write_Log"<<std::endl;
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
	if(RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	//std::string str = "obayashiIOFiles/Log/simlation_environment.txt";
	
	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" +eraseblock+ "/" + std::to_string(simple::MyBuilding::GetNumUsers()) +"/Info/Simlation_envaironment.txt";
//	std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::out);
	fout << "simlation_envaironment" << std::endl;
	
	fout << "Field_Name = " << field_name << std::endl;
	fout << "Num_User = " << numUsers << std::endl;
	fout << "Add_User_Num = " << addUsers <<std::endl;
	fout << "User_Speed = " << userSpeed << std::endl;
	fout << "HelloInterval = " << HelloInterval << std::endl;
	fout << "UpdateMapinfoInterval = " << UpdateMapinfoInterval << std::endl;
	fout << "BlockFlag = " << blockflag << std::endl;
	fout << "BlockEraseFlag = " << blockeraseflag << std::endl;
	fout << "SetExitAutoFlag = " << setexitAutoflag << std::endl;
	fout << "SetExitManualFlag = " << setexitmanualflag << std::endl;
	fout << "SetNearly = " << neary << std::endl;
	fout << "RandomUserSpeedFlag = " << randomuserSpeedflag << std::endl;
	fout << "MinSpeed = " << MinSpeed << std::endl;
	fout << "MaxSpeed = " << MaxSpeed << std::endl;
	fout << "UseExitInfo = " << useexitinfo << std::endl;
	fout << "UseDisasterTypeFlag = "<< UseDisasterType << std::endl;
	fout << "DisasterType = " << Disaster_Type << std::endl;
	fout << "AoI_Threshold = "<<AoI<< std::endl;
	fout << "UseQuickSendFlag = "<<QuickSend<< std::endl;
	fout << "QuickHelloInterval = "<<QuickHelloInterval<< std::endl;
	fout << "AddUserInterval = "<<AddUserInterval<< std::endl;
	fout << "MovingUserCheckInterval = "<<moving_user_check_interval<< std::endl;
	fout << "QuickHelloTimes = "<<quick_hello_times<< std::endl;
 	fout.close();
}

void MyBuilding::SetExitNodeDistance(){
	for(auto a : exit_nodes){
		double dst;
		std::map<String, double> kyori;
		std::pair<double,double> first = JSON_Data::GetCoordinatefromID(a.first);
		// std::cout<<first.first<<","<<first.second<<std::endl;
		for(auto b : exit_nodes){
			std::pair<double,double> second = JSON_Data::GetCoordinatefromID(b.first);
			dst = JSON_Data::calculate_distance(first,second);
			kyori.insert(std::make_pair(b.first,dst));
			// std::cout<<dst<<std::endl;
		}
		exit_node_distances.insert(std::make_pair(a.first,kyori));
	}
	CalculatePatrolRoute();
	// std::exit(1);
}


void MyBuilding::CalculatePatrolRoute() {
    m_patrolRoute.clear();
    if(MyBuilding::GetExitAutoflag()){
        std::map<String, uint32_t> exitnodes = MyBuilding::GetExitNodes();
        ferry_start = exitnodes.begin()->first;
    }

    m_patrolRoute.push_back(ferry_start);

    std::set<std::string> unvisited;
    for (const auto& pair : exit_node_distances) {
        if (pair.first != ferry_start) {
            unvisited.insert(pair.first);
        }
    }

    std::string current_shelter = ferry_start;
    while (!unvisited.empty()) {
        std::string nearest_shelter;
        double min_dist_to_next = std::numeric_limits<double>::max();

        for (const auto& candidate : unvisited) {
            if (exit_node_distances[current_shelter][candidate] < min_dist_to_next) {
                min_dist_to_next = exit_node_distances[current_shelter][candidate];
                nearest_shelter = candidate;
            }
        }
        
        current_shelter = nearest_shelter;
        m_patrolRoute.push_back(current_shelter);
        unvisited.erase(current_shelter);
    }

    // --- ここから追加：一周の距離計算 ---
    double total_distance = 0.0;

    if (!m_patrolRoute.empty()) {
        // 1. ルート順の距離を加算 (A -> B -> C ...)
        for (size_t i = 0; i < m_patrolRoute.size() - 1; ++i) {
            String from = m_patrolRoute[i];
            String to = m_patrolRoute[i + 1];
            // exit_node_distances[from][to] で距離を取得
            total_distance += exit_node_distances.at(from).at(to);
        }

        // 2. 最後のノードから始点に戻る距離を加算 (Z -> A)
        String last = m_patrolRoute.back();
        String start = m_patrolRoute.front();
        total_distance += exit_node_distances.at(last).at(start);
    }
    // --- 追加ここまで ---

    std::cout << "Calculated Patrol Route: ";
    for(const auto& s : m_patrolRoute) std::cout << s << " ";
    std::cout << std::endl;

    // 距離の表示
    std::cout << "Total Patrol Distance (One Lap): " << total_distance << " m" << std::endl;

    // std::exit(1);
}

}}