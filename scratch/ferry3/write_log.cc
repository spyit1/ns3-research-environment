/*
 * write_log.cc
 *
 *  Created on: 2022/06/02
 *      Author: matsunaga
 */

#include "write_log.h"
// #define INF 10000000

namespace ns3{

std::list<uint32_t> WriteLog::traceuserlist;
std::map<String,uint32_t> WriteLog::Backupexitinfo;

WriteLog::WriteLog(){

}

void WriteLog::Initialize_ClusterView(){
	//simple::MyBuilding::SetExitNodes(); //10/19追加
	std::ostringstream oss_delay, oss_run;
//		oss_delay << simple::MyBuilding::GetFloodingInterval();
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

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/"+ eraseblock+ "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterViewer/ClusterViewer_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//	const std::string str = "Log/ClusterViewer/ClusterViewer_"+oss_run.str()+".txt";
	const char *CV_file = str.c_str();
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	// std::cout<<str<<std::endl;

	std::ofstream fout;
	fout.open(CV_file,std::ios::out);
	if(!fout){
		std::cout<<"Could not open file. >>"<<CV_file<<std::endl;
		exit(1);
	}
//	fout<<simple::MyBuilding::GetNumUsers()+simple::MyBuilding::GetNumSections()<<","<<"1000"<<","<<"1000"<<std::endl;
	fout<<simple::MyBuilding::GetNumUsers()+simple::MyBuilding::GetNumSections()<<","<<(int)simple::JSON_Data::json_x(simple::JSON_Data::get_max_lo())<<","<<(int)simple::JSON_Data::json_y(simple::JSON_Data::get_max_la())<<std::endl;

	fout << "NI, 0, 10000,   0,   0, 0, READY, 0, 0" << std::endl;
	fout << "NI, 0, 10001, "<<simple::JSON_Data::json_x(simple::JSON_Data::get_max_lo())<<",  0, 0, READY, 0, 0" << std::endl;
	fout << "NI, 0, 10002, "<<simple::JSON_Data::json_x(simple::JSON_Data::get_max_lo())<<", "<<simple::JSON_Data::json_y(simple::JSON_Data::get_max_la())<<", 0, READY, 0, 0" << std::endl;
	fout << "NI, 0, 10003,   0, "<<simple::JSON_Data::json_y(simple::JSON_Data::get_max_la())<<", 0, READY, 0, 0" << std::endl;

//	std::list<json> nodes=simple::JSON_Data::get_nodes();
//	std::list<simple::way_node> no =simple::JSON_Data::get_waynodes();

	std::map<std::pair<double,double>,simple::way_node> no =simple::JSON_Data::get_waynodes();
	std::map<String,simple::way_info> wi =simple::JSON_Data::get_linestrings();
	std::map<std::pair<double,double>,simple::way_node> gr = simple::JSON_Data::get_graph();


	int i = 10004;


	std::list<String> nodeids;
	std::list<std::pair<double,double>> hinan =simple::JSON_Data::get_hinanjo();
	std::set<std::pair<double,double>> enternode;

	// simple::MyBuilding::SetExitNodes();
	for(auto itr = hinan.begin();itr != hinan.end();++itr){
		std::pair<double,double> hinanjo = *itr;
		std::pair<double,double> enter;
		double nearly = (double)INF;
		for(auto itr2 = gr.begin();itr2 != gr.end();++itr2){
			std::pair<double,double> node = itr2->first;
			double dist = simple::JSON_Data::calculate_distance(node,hinanjo);
			if(dist<nearly){
				nearly = dist;
				enter = node;
			}
		}
		if(nearly<150) enternode.insert(enter);
	}

    for(auto itr = no.begin();itr != no.end();++itr){
    	std::pair<double,double> node = itr->first;
    	simple::way_node wayn = itr->second;
    	String flg="FALSE";
//    	String nodeid;

    	if(wayn.GetCrossingflag()) {
    		flg="TRUE";
			String id = "node/"+std::to_string(i);
			decltype(enternode)::iterator itr3 = enternode.find(node);
			if(itr3!=enternode.end()){
				// simple::MyBuilding::SetExitNodes();
			}
//    		for(auto itr3 = hinan.begin();itr3 != hinan.end();++itr3){
//    			std::pair<double,double> hinanjo = *itr3;
//    			double dist = simple::JSON_Data::calculate_distance(node,hinanjo);
//    			if(dist<150){
//    				simple::MyBuilding::SetExitNodes(id,100);
//    			}
//    		}

    		std::map<String, uint32_t> exits = simple::MyBuilding::GetExitNodes();
			decltype(exits)::iterator itr2 = exits.find(id);
			if(itr2 != exits.end()){
				flg="EXIT";

			}
    	}

//    	nodeid = "node/" + std::to_string(i);
//    	nodeids.push_back(nodeid);

    	fout << "NI, 0, "<<i<<", "<<simple::JSON_Data::json_x(node.first)<<", "<<simple::JSON_Data::json_y(node.second)<<", 0, "<< flg<<", 0, 0"<< std::endl;
    	simple::JSON_Data::SetNodeID(i,node);
    	i++;
    }

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

void WriteLog::Initialize_Logfile_for_ClusterMemberAverage(){
	std::ostringstream oss_delay, oss_run;
//		oss_delay << simple::MyBuilding::GetFloodingInterval();
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

	const std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterInfo/ClusterMemberAverage/ClusterMemberAverage_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	const char *filename = str.c_str();
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
//	str.copy(filename,str.size());
//	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename,std::ios::out);
	if(!fout){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	fout << "ClusterMemberAverage" <<std::endl;
	fout.close();
}

void WriteLog::Initialize_Logfile_for_ClusterMemberSize(){
	std::ostringstream oss_delay, oss_run;
//		oss_delay << simple::MyBuilding::GetFloodingInterval();
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

	const std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterInfo/ClusterMemberSize/ClusterMemberSize_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	const char *filename = str.c_str();
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
//	str.copy(filename,str.size());
//	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename,std::ios::out);
	if(!fout){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	fout << "ClusterMemberSize" <<std::endl;
	fout.close();
}

void WriteLog::Initialize_Logfile_for_ClusterHopInfo(){
	std::ostringstream oss_delay, oss_run;
//		oss_delay << simple::MyBuilding::GetFloodingInterval();
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

	const std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterInfo/ClusterHopInfo/ClusterHopInfo_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	const char *filename = str.c_str();
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
//	str.copy(filename,str.size());
//	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename,std::ios::out);
	if(!fout){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	fout << "ClusterHopInfo" <<std::endl;
	fout.close();
}

void WriteLog::Initialize_Logfile_for_SendSectionDataSize(){
	char filename[100];
	std::ostringstream oss_delay, oss_run;
//		oss_delay << simple::MyBuilding::GetFloodingInterval();
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

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/SectionData/SendSectionDataSize_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename);
	if(!fout){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	fout << "SendSectionDataSize" <<std::endl;
	fout.close();
}
void WriteLog::Initialize_Logfile_for_EvacuationTime(){
	char filename[100];
	std::ostringstream oss_delay, oss_run;
//		oss_delay << simple::MyBuilding::GetFloodingInterval();
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

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Time/time_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename);
	if(!fout){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	// fout << "EvacuationTime" <<std::endl;
	fout << "userID, time, changeroute, changeescape" <<std::endl;
	fout.close();
}


void WriteLog::Initialize_Logfile_for_EvacuationTime2(){
	char filename[100];
	std::ostringstream oss_delay, oss_run;
//		oss_delay << simple::MyBuilding::GetFloodingInterval();
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

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Time/EvacuationTime_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename);
	if(!fout){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	// fout << "EvacuationTime" <<std::endl;
	fout << "userID, time" <<std::endl;
	fout.close();
}


void WriteLog::Initialize_Logfile_for_EscapeInfo(){
	//simple::MyBuilding::SetExitNodes();//追加10/18
	SetTraceUser();
	char filename[100];
	std::ostringstream oss_delay, oss_run;
//		oss_delay << simple::MyBuilding::GetFloodingInterval();
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

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Escape/escape_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename);
	if(!fout){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	fout << "EscapeInfo" <<std::endl;
	fout.close();
}

void WriteLog::Initialize_Logfile_for_UserTrackingLog(){
	// set directory
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
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	for(auto itr = traceuserlist.begin();itr!=traceuserlist.end();++itr){
		auto it = *itr;
		std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/UserInfo/User" + std::to_string(it) + "_"+std::to_string(simple::MyBuilding::GetRun()) + ".txt";
		str.copy(filename,str.size());
		filename[str.size()] = '\0';
		// output to file
		std::ofstream fout;
		fout.open(filename);
		if(!fout){
			std::cout<<"Could not open file."<<std::endl;
			exit(1);
		}
		fout<<"User"<<std::to_string(it)<<std::endl;
		fout<<"---------------------------------------------------------------------------------"<<std::endl;
		fout.close();
	}
}

void Initislize_logfile_for_Recive(){
	char filename[100];
	std::ostringstream oss_delay, oss_run;
//		oss_delay << simple::MyBuilding::GetFloodingInterval();
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

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Recive/recive_data" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename);
	if(!fout){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	fout << "Recive_data" <<std::endl;
	fout << "sender,reciver" <<std::endl;
	fout.close();
}

void Initislize_logfile_for_TestLog(){
	char filename[100];
	std::ostringstream oss_delay, oss_run;
//		oss_delay << simple::MyBuilding::GetFloodingInterval();
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

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/testLog/Recv_data_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//	std::string str = "Log/obayashi/ClusterViewer/ClusterViewer_"+oss_run.str()+".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename,std::ios::out);
// 	char filename[100];
// 	std::ostringstream oss_delay, oss_run;
// //	oss_delay << simple::MyBuilding::GetFloodingInterval();
// 	oss_run << RngSeedManager::GetRun();
// 	String mode="ON";

// 	if(simple::RoutingProtocol::GetMode()){
// 		mode="ON";
// 	}else{
// 		mode="OFF";
// 	}
// 	std::string str = "Log/obayashi/" + mode + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/testLog/Recv_data.txt";
// 	str.copy(filename,str.size());
// 	filename[str.size()] = '\0';
// 	std::ofstream fout;
// 	fout.open(filename);
	if(!fout){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	fout.close();

	char filename2[100];
	std::string str2 = "Log/obayashi/" + mode + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/testLog/testlog.txt";
	str2.copy(filename2,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout2;
	fout2.open(filename);
	if(!fout2){
		std::cout<<"Could not open file."<<std::endl;
		exit(1);
	}
	fout2.close();
}


void WriteLog::OutputText_NI(Time now,Ptr<Ipv4> m_ipv4,std::string color,Ipv4Address parent,Ipv4Address clusterId,uint8_t hop){

}
void WriteLog::OutputText_RW(Time now,int pktType,Ipv4Address dst){}
void WriteLog::OutputClusterMemberAverage(Time now_t){

}
void WriteLog::OutputDatasize(Time now_t, int size){

}
void WriteLog::OutputText_CompleteTime(Time now, uint32_t num, uint32_t userId, uint32_t changeroute, uint32_t changeexit){
	// set parameters
	int64_t time = now.GetInteger();
	int64_t roughTime = time / 1000000000;
	int64_t splitSecond = time % 1000000000;

	// set directory
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
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Time/time_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//	std::string str = "Log/obayashi/Time/time_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
//	uint32_t num = JSON_Data::NodeIDstoi(m_exitnode);
	// char escape;
	// switch(num){
	// 	case 10254:
	// 		escape = 'A';
	// 		break;
	// 	case 10845:
	// 		escape = 'B';
	// 		break;
	// 	case 11732:
	// 		escape = 'C';
	// 		break;
	// 	// case 13139:
	// 	// 	escape = 'D';
	// 	// 	break;
	// 	// case 13514:
	// 	// 	escape = 'E';
	// 	// 	break;
	// 	// case 10485:
	// 	// 	escape = 'F';
	// 	// 	break;
	// 	default:
	// 		break;
	// }

	// output to file
	std::ofstream fout;
	fout.open(filename,std::ios::app);
	fout << userId+1 <<", "
			<< roughTime << "." << splitSecond	<<", "// time [s]
//			<< (int)m_changeroute_dueto_blockedinfo <<", "
//			<< (int)m_changeroute_dueto_exitinfo <<", "
//			<< (int)m_changeexit_dueto_blockedinfo <<", "
//			<< (int)m_changeexit_dueto_exitinfo <<", "
			<< (int)changeroute << ", "
			<< (int)changeexit  << std::endl;
			// << escape
//		 << std::endl;
	fout.close();
}


void WriteLog::Evacuationtime(uint32_t userid, double starttime){

	double now = Simulator::Now().GetDouble()/1000000000;
	double evacuationtime = now - starttime;

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
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Time/EvacuationTime_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//	std::string str = "Log/obayashi/Time/time_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	std::ofstream fout;
	fout.open(filename,std::ios::app);
	fout << userid+1 <<", "<<evacuationtime<<std::endl;
	fout.close();

}


void WriteLog::OutputEscapeInfo(std::map<String, uint32_t> esc){
	// set directory
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
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Escape/escape_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';

	// output to file
	std::ofstream fout;
	fout.open(filename,std::ios::app);
	for(auto itr=esc.begin();itr!=esc.end();++itr){
		String id = itr->first;
		uint32_t num = itr->second;
		fout << id << ", " << Backupexitinfo.at(id)-num <<"/"<<Backupexitinfo.at(id)<<std::endl;
	}
	fout.close();
}

void OutputText_Recive_Data(Ipv4Address sender, Ipv4Address reciver){
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
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/Recive/recive_data" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::app);
	fout << sender << ", " << reciver << std::endl;
	fout.close();
}

void WriteLog::OutputTotal_distance(uint32_t userid, double total_distance){
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
	if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/UserInfo/Total_move_distance" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
//		std::string str = "ClusterViewer/"+oss_delay.str()+"/ClusterViewer_"+oss_run.str()+".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';
	std::ofstream fout;
	fout.open(filename,std::ios::app);
	fout << userid + 1 << ", " << total_distance << std::endl;
	fout.close();
}

void WriteLog::OutputUserTrackingLog(Time now, uint32_t userId,std::list<std::pair<double,double>> shortest, double dst, std::map<std::pair<double,double>,simple::way_node> graph){
	auto result = find(traceuserlist.begin(),traceuserlist.end(),userId);
	if(result == traceuserlist.end()) {
		return;
	}else{
		int64_t time = now.GetInteger();
		int64_t roughTime = time / 1000000000;
		int64_t splitSecond = time % 1000000000;
		// set directory
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
		if(simple::RoutingProtocol::GetUseEraseInfoflag() == true){
			eraseblock="EraseBlock_ON";
		}else{
			eraseblock="EraseBlock_OFF";
		}

		std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/UserInfo/User" + std::to_string(userId) +"_"+std::to_string(simple::MyBuilding::GetRun()) + ".txt";
		str.copy(filename,str.size());
		filename[str.size()] = '\0';

		// output to file
		std::ofstream fout;
		fout.open(filename,std::ios::app);


		for(auto itr=shortest.begin();itr!=shortest.end();++itr){
			std::pair<double,double> it = *itr;
			String id = simple::JSON_Data::GetIDfromCoordinate(it);
			fout << "id:" <<id<<" ("<<it.first<<", "<<it.second<<")"<<std::endl;


//			if(id == "node/11113"){
				std::map<std::pair<double,double>,double> nb = graph.at(simple::JSON_Data::GetCoordinatefromID(id)).GetNeighborNodes();
				for(auto itr = nb.begin();itr!=nb.end();++itr){
					std::pair<double,double> it2 = itr->first;
					double it3 = itr->second;

					fout << "nb_id:" <<simple::JSON_Data::GetIDfromCoordinate(it2)<<" dis:"<<it3<<"m"<<std::endl;
				}
//			}

		}
		fout<<"dst(m):"<<dst<<" time(s):"<< roughTime << "." << splitSecond <<std::endl;
		fout<<"---------------------------------------------------------------------------------"<<std::endl;
		fout.close();
	}

}


}
