/*
 * create-environment.cc
 *
 *  Created on: 2015/12/03
 *      Author: terami
 */

#include "create-environment.h"

namespace ns3{


Environment::Environment(){

	InputFromFile();


}

Environment::Environment(bool flag){
	InputFromFile();

//	std::ofstream fout;
//	fout.open("directory.sim");
//	if(!fout){
//		std::cout<<"Couldn't open file <directory.sim> bp2"<<std::endl;
//		exit(1);
//	}else{
//		fout<<"CLEAR";
//	}
//	fout.close();
}

void Environment::InputFromFile(){
	std::ifstream fin;
	std::string str;
	std::vector<std::string> str_vct;

	fin.open(std::string("obayashiIOFiles/CLEAR/environment.csv"),std::ios::in);
	if(!fin){
		std::cout<<"couldn't open file <environment.csv> bp1"<<std::endl;
		exit(1);
	}else{
		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_mode=str_vct[1];
		// std::cout<<"m_mode:"<<m_mode<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_dstType=str_vct[1];
		// std::cout<<"m_dstType:"<<m_dstType<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_protocol=str_vct[1];
		// std::cout<<"m_protocol:"<<m_protocol<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_numSuperpeer=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_numSuperpeer:"<<m_numSuperpeer<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_minRange=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_minRange:"<<m_minRange<<std::endl;

		fin>>str;
		boost::split(str_vct,str_vct[1],boost::is_any_of(","));
		m_maxRange=std::atoi(str.c_str());
		// std::cout<<"m_maxRange:"<<m_maxRange<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_P2PInterval=std::atof(str_vct[1].c_str());
		// std::cout<<"m_P2PInterval:"<<m_P2PInterval<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_P2PDelay=std::atof(str_vct[1].c_str());
		// std::cout<<"m_P2PDelay:"<<m_P2PDelay<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_helloInterval=std::atof(str_vct[1].c_str());
		// std::cout<<"m_helloInterval:"<<m_helloInterval<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_epidemicDataInterval=std::atof(str_vct[1].c_str());
		// std::cout<<"m_epidemicDataInterval:"<<m_epidemicDataInterval<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_lowerCluster=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_lowerCluster:"<<m_lowerCluster<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_upperCluster=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_upperCluster:"<<m_upperCluster<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_mepInterval=std::atof(str_vct[1].c_str());
		// std::cout<<"m_mepInterval:"<<m_mepInterval<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_mepTtl=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_mepTtl:"<<m_mepTtl<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_mapDelay=std::atof(str_vct[1].c_str());
		// std::cout<<"m_mapDelay:"<<m_mapDelay<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_gatewayLimit=std::atof(str_vct[1].c_str());
		// std::cout<<"m_gatewayLimit:"<<m_gatewayLimit<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_mepLimit=std::atof(str_vct[1].c_str());
		// std::cout<<"m_mepLimit:"<<m_mepLimit<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_mobility=str_vct[1];
		// std::cout<<"m_mobility:"<<m_mobility<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_minSpeed=std::atof(str_vct[1].c_str());
		// std::cout<<"m_minSpeed:"<<m_minSpeed<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_maxSpeed=std::atof(str_vct[1].c_str());
		// std::cout<<"m_maxSpeed:"<<m_maxSpeed<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_simTime=std::atof(str_vct[1].c_str());
		// std::cout<<"m_simTime:"<<m_simTime<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_fieldX=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_fieldX:"<<m_fieldX<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_fieldY=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_fieldY:"<<m_fieldY<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_gridWidth=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_gridWidth:"<<m_gridWidth<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_gridX=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_gridX:"<<m_gridX<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_gridY=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_gridY:"<<m_gridY<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_numAllNode=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_numAllNode:"<<m_numAllNode<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_numNode=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_numNode:"<<m_numNode<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_serverType=str_vct[1];
		// std::cout<<"m_serverType:"<<m_serverType<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_numServer=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_numServer:"<<m_numServer<<std::endl;

		for(int i=0;i<m_numServer;i++){
			fin>>str;
			boost::split(str_vct,str,boost::is_any_of(","));
			double x=std::atof(str_vct[1].c_str());
			double y=std::atof(str_vct[2].c_str());
			m_serverPosition.push_back(std::pair<int,int>(x,y));
		}

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_dataSize=std::atoi(str_vct[1].c_str());
		std::cout<<"m_dataSize:"<<m_dataSize<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_numData=std::atoi(str_vct[1].c_str());
		// std::cout<<"m_numData:"<<m_numData<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_dataInterval=std::atof(str_vct[1].c_str());
		// std::cout<<"m_dataInterval:"<<m_dataInterval<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_dataStart=std::atof(str_vct[1].c_str());
		// std::cout<<"m_dataStart:"<<m_dataStart<<std::endl;

		fin>>str;
		boost::split(str_vct,str,boost::is_any_of(","));
		m_detectData=std::atof(str_vct[1].c_str());
		// std::cout<<"m_detectData:"<<m_detectData<<std::endl;


	}
	fin.close();
}




}



