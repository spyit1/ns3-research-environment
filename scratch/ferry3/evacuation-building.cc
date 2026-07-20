
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
double MyBuilding::userSpeed = 1.6;
double MyBuilding::HelloInterval = 20.0;
// double MyBuilding::ShelterHelloInterval = 60.0;
double MyBuilding::ShelterDataInterval = 60.0;
double MyBuilding::UpdateMapinfoInterval = 50.0;
bool MyBuilding::blockflag = true;
bool MyBuilding::blockeraseflag = true;
bool MyBuilding::setexitAutoflag = true;
bool MyBuilding::setexitmanualflag = true;
std::list<String> MyBuilding::blocked_nodes;
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
uint32_t MyBuilding::quick_hello_times = 3;

uint32_t MyBuilding::num = 0;
std::set<uint32_t> MyBuilding::number;

std::vector<uint32_t> MyBuilding::vect;
std::map<String, uint32_t> MyBuilding::moving_user_num; //各避難所ごとに現在動いているユーザ数のmap

uint32_t MyBuilding::send_hello_num = 0;
uint32_t MyBuilding::send_hello_bytes = 0;
uint32_t MyBuilding::recv_hello_num = 0;
uint32_t MyBuilding::recv_hello_bytes = 0;
uint32_t MyBuilding::send_ack_hello_num = 0;
uint32_t MyBuilding::recv_ack_hello_num = 0;

uint32_t MyBuilding::seed = 0;
uint32_t MyBuilding::run = 0;

std::map<String, double> MyBuilding::max_distance_to_exit;

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
	myIni.load("setting.ini");
	fieldWidth = myIni["Setting1"]["FieldWidth"].as<double>();
	fieldHight = myIni["Setting1"]["FieldHight"].as<double>();
	field_name = myIni["Setting1"]["field_name"].as<String>();
	numUsers = myIni["Setting1"]["Num_user"].as<uint32_t>();
	addUsers = myIni["Setting1"]["Add_user"].as<uint32_t>();
	userSpeed = myIni["Setting1"]["UserSpeed"].as<double>();
	HelloInterval = myIni["Setting1"]["HelloInterval"].as<double>();
	ShelterDataInterval = myIni["Setting1"]["ShelterDataInterval"].as<double>();
	UpdateMapinfoInterval = myIni["Setting1"]["UpdateMapinfoInterval"].as<double>();
	blockflag = myIni["Setting1"]["blockflag"].as<bool>();
	blockeraseflag = myIni["Setting1"]["blockeraseflag"].as<bool>();
	setexitAutoflag = myIni["Setting1"]["setexitAutoflag"].as<bool>();
	setexitmanualflag = myIni["Setting1"]["setexitmanualflag"].as<bool>();
	neary =  myIni["Setting1"]["setnearly"].as<uint32_t>();
	SimulationFieldMap = myIni["json_data"]["SimulationFieldMap"].as<String>(); //使ってない
	hinanjo_json = myIni["json_data"]["hinanjo_json"].as<String>();
	randomuserSpeedflag = myIni["Setting1"]["RandomUserSpeedFlag"].as<bool>();
	MinSpeed = myIni["Setting1"]["MinSpeed"].as<double>();
	MaxSpeed = myIni["Setting1"]["MaxSpeed"].as<double>();
	useexitinfo = myIni["Setting1"]["useexitinfo"].as<bool>();
	UseDisasterType = myIni["Setting1"]["UseDisasterType"].as<bool>();
	Disaster_Type = myIni["Setting1"]["Disaster_Type"].as<uint32_t>();
	AoI = myIni["Setting1"]["AoI"].as<uint32_t>();
	QuickSend = myIni["Setting1"]["UseQuickSend"].as<bool>();
	QuickHelloInterval = myIni["Setting1"]["QuickHelloInterval"].as<double>();
	AddUserInterval = myIni["Setting1"]["AddUserInterval"].as<double>();
	moving_user_check_interval = myIni["Setting1"]["moving_user_check_interval"].as<double>();
	quick_hello_times = myIni["Setting1"]["quick_hello_times"].as<uint32_t>();
	// EraseAoI = myIni["Setting1"]["eraseAoI"].as<double>();
	if(blockflag == false) blockeraseflag = false;
	if(setexitmanualflag == false) setexitAutoflag = true;
	if(setexitAutoflag == false) UseDisasterType = false;
	
	std::cout<<MinSpeed<<", "<<MaxSpeed<<std::endl;
	// MyBuilding::Write_Logfile_for_simulation_environment();
	// std::cout<<"UpdateMapinfoInterval："<<UpdateMapinfoInterval<<std::endl;
}

//ブロックノード指定
void MyBuilding::SetBlockedNodes(uint32_t update_num){
	// uint32_t update_num = JSON_Data::GetUpdateNum();
	uint32_t time = MyBuilding::GetUpdateMapinfoInterval() * update_num; 
	// std::cout<<"time："<<num<<std::endl;
	std::string str2 = "./data/" + field_name + "/block_nodes_data/set/blocked_nodes_" + std::to_string(time) + ".txt";
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
	char filename[100];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(MyBuilding::GetNumUsers()) + "/Info/Block/BlockNode_" +std::to_string(simple::MyBuilding::GetRun()) +".txt";
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
	std::string str2 = "./data/"+ field_name +"/block_nodes_data/erase/erase_nodes_" + std::to_string(time) + ".txt";
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
	for(auto a : blocked_nodes){
		std::cout<<a<<", ";
	}
	std::cout<<std::endl;
}

void MyBuilding::SetExitNodes(){
	std::string file_name = "data/"+ field_name +"/exit_nodes.txt";
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

void MyBuilding::Write_Logfile_for_simulation_environment(){
	std::cout<<"Write_Log"<<std::endl;
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

	//std::string str = "Log/simlation_environment.txt";
	
	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" +eraseblock+ "/" + std::to_string(simple::MyBuilding::GetNumUsers()) +"/Info/Simlation_envaironment.txt";
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

}}