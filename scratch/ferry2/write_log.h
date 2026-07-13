/*
 * write_log.h
 *
 *  Created on: 2022/06/02
 *      Author: matsunaga
 */

#ifndef SCRATCH_CLUSTER_EVACUATION_FOR_GEOGRAPHIC_INFO_WRITE_LOG_H_
#define SCRATCH_CLUSTER_EVACUATION_FOR_GEOGRAPHIC_INFO_WRITE_LOG_H_

#include <string>

#include "ns3/ptr.h"
#include "ns3/timer.h"
#include "ns3/node-list.h"
#include "ns3/ipv4.h"

#include "evacuation-user.h"
#include "evacuation-building.h"
//#include "simple-protocol.h"
#include "DTN-protocol.h"
#include "json_data.h"



namespace ns3 {
class WriteLog
{
private:
	static std::list<uint32_t> traceuserlist;
	static std::map<String,uint32_t> Backupexitinfo;

public:
	WriteLog();
	~WriteLog();

	static void Initialize_ClusterView();
	static void Initialize_Logfile_for_ClusterMemberAverage();
	static void Initialize_Logfile_for_ClusterMemberSize();
	static void Initialize_Logfile_for_SendSectionDataSize();
	static void Initialize_Logfile_for_EvacuationTime();
	static void Initialize_Logfile_for_EvacuationTime2();
	static void Initialize_Logfile_for_EscapeInfo();
	static void Initialize_Logfile_for_ClusterHopInfo();
	static void Initialize_Logfile_for_UserTrackingLog();
	static void Initislize_logfile_for_Recive();
	static void Initislize_logfile_for_TestLog();
//	static void Initialize_Logfile_for_PassedWay();

	static void OutputText_NI(Time now,Ptr<Ipv4> m_ipv4,std::string color,Ipv4Address parent,Ipv4Address clusterId,uint8_t hop);
	static void OutputText_RW(Time now,int pktType,Ipv4Address dst);
	static void OutputClusterMemberAverage(Time now_t);
	static void OutputDatasize(Time now_t, int size);
	static void OutputText_CompleteTime(Time now, uint32_t num, uint32_t userId, uint32_t changeroute, uint32_t changeexit, String exitnodeid);
	static void OutputEscapeInfo(std::map<String, uint32_t> esc);
	static void OutputText_Recive_Data(Ipv4Address sender, Ipv4Address reciver);
	static void OutputTotal_distance(uint32_t userID, double total_distance);
	static void Evacuationtime(uint32_t userid, double starttime);

	static void SetTraceUser(){Backupexitinfo=simple::MyBuilding::GetExitNodes();}

	static void SetTraceUser(String traceusers){
		auto separator = String(", ");
		auto separator_length = separator.length();

		if(separator_length==0){
			traceuserlist.push_back(stoi(traceusers));
		} else {
			auto offset = String::size_type(0);
			while (1){
				auto pos = traceusers.find(separator, offset);
				if(pos == String::npos){

					traceuserlist.push_back(stoi(traceusers.substr(offset)));
					break;
				}
				traceuserlist.push_back(stoi(traceusers.substr(offset, pos - offset)));
				offset = pos + separator_length;
			}
		}
	}

	static void OutputUserTrackingLog(Time now, uint32_t userId,std::list<std::pair<double,double>> shortest, double dst,std::map<std::pair<double,double>,simple::way_node> graph);


//	static void OutputText_PassedWay();
};

}


#endif /* SCRATCH_CLUSTER_EVACUATION_FOR_GEOGRAPHIC_INFO_WRITE_LOG_H_ */
