/*
 * create-environment.h
 *
 *  Created on: 2015/12/03
 *      Author: terami
 */

#ifndef CREATE_ENVIRONMENT_H_
#define CREATE_ENVIRONMENT_H_

#include <sstream>
#include <string>
#include <fstream>
#include <list>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <map>
#include <random>
#include <unordered_set>

#include <boost/algorithm/string.hpp>

#include "cluster-list.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4-routing-protocol.h"


namespace ns3{


class Cellular{
public:
	Cellular(){m_x=0.0;m_y=0.0;m_active=false;}
	Cellular(double x,double y,bool active){
		m_x=x;
		m_y=y;
		m_active=active;
	}

	void SetX(double x){m_x=x;}
	double GetX(){return m_x;}

	void SetY(double y){m_y=y;}
	double GetY(){return m_y;}

	void SetActive(bool active){m_active=active;}
	bool GetActive(){return m_active;}

private:
	double m_x;
	double m_y;
	bool m_active;
};

class Environment{
public:
	Environment();
	Environment(bool flag);

	std::string GetMode(){return m_mode;}
	std::string GetDstType(){return m_dstType;}
	std::string GetProtocol(){return m_protocol;}
	int GetNumSuperPeer(){return m_numSuperpeer;}
	int GetMinRange(){return m_minRange;}
	int GetMaxRange(){return m_maxRange;}
	double GetP2PInterval(){return m_P2PInterval;}
	double GetP2PDelay(){return m_P2PDelay;}
	double GetHelloInterval(){return m_helloInterval;}
	double GetEpidemicDataInterval(){return m_epidemicDataInterval;}
	int GetLowerCluster(){return m_lowerCluster;}
	int GetUpperCluster(){return m_upperCluster;}
	double GetMepInterval(){return m_mepInterval;}
	int GetMepTtl(){return m_mepTtl;}
	double GetMapDelay(){return m_mapDelay;}
	double GetGatewayLimit(){return m_gatewayLimit;}
	double GetMepLimit(){return m_mepLimit;}
	std::string GetMobility(){return m_mobility;}
	double GetMinSpeed(){return m_minSpeed;}
	double GetMaxSpeed(){return m_maxSpeed;}
	double GetSimTime(){return m_simTime;}
	int GetFieldX(){return m_fieldX;}
	int GetFieldY(){return m_fieldY;}
	int GetGridWidth(){return m_gridWidth;}
	int GetGridX(){return m_gridX;}
	int GetGridY(){return m_gridY;}
	int GetNumAllNode(){return m_numAllNode;}
	int GetNumNode(){return m_numNode;}
	std::string GetServerType(){return m_serverType;}
	int GetNumServer(){return m_numServer;}
	std::list< std::pair<double,double> > GetServerPosition(){return m_serverPosition;}
	int GetDataSize(){return m_dataSize;}
	int GetNumData(){return m_numData;}
	double GetDataInterval(){return m_dataInterval;}
	double GetDataStart(){return m_dataStart;}
	double GetDetectData(){return m_detectData;}


	std::string GetDirectory(){return m_directory;}

	bool GetLinked(uint32_t &cellularId,double x,double y);

	void InputSetting();

	void InputFromFile();


private:
	std::string m_mode;
	std::string m_dstType;
	std::string m_protocol;
	int m_numSuperpeer;
	int m_minRange;
	int m_maxRange;
	double m_P2PInterval;
	double m_P2PDelay;
	double m_helloInterval;
	double m_epidemicDataInterval;
	int m_lowerCluster;
	int m_upperCluster;
	double m_mepInterval;
	int m_mepTtl;
	double m_mapDelay;
	double m_gatewayLimit;
	double m_mepLimit;
	std::string m_mobility;
	double m_minSpeed;
	double m_maxSpeed;
	double m_simTime;
	int m_fieldX;
	int m_fieldY;
	int m_gridWidth;
	int m_gridX;
	int m_gridY;
	int m_numAllNode;
	int m_numNode;
	std::string m_serverType;
	int m_numServer;
	std::list< std::pair<double,double> > m_serverPosition;
	int m_dataSize;
	int m_numData;
	double m_dataInterval;
	double m_dataStart;
	double m_detectData;

	std::string m_directory;
};


}




#endif /* CREATE_ENVIRONMENT_H_ */
