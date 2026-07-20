/*
 * section-data-trade.h
 *
 *  Created on: Jan 12, 2021
 *      Author: matsunaga
 */

#include <iostream>
#include <map>
#include <list>
#include <unordered_map>
#include <vector>

#include "cluster-list.h"
#include "evacuation-accesspoint.h"
#include "evacuation-section.h"
#include "evacuation-user.h"
#include "simple-rtable.h"

#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"



#ifndef SCRATCH_EVACUATION_VER6_SECTION_DATA_TRADE_H_
#define SCRATCH_EVACUATION_VER6_SECTION_DATA_TRADE_H_


namespace ns3 {
namespace simple {

class StoreSectionData
{
private:
	Ipv4Address m_source;
	uint32_t m_seqNo;
	Position m_sectionId;
	uint32_t m_apId;
	uint32_t m_status;
	uint32_t m_occupancy;

//	uint8_t m_cmlSize;
//	std::list<Ipv4Address> m_memberId;
//	std::list<uint8_t> m_memberHop;
//	std::list<uint8_t> m_memberChildren;
//	std::list<uint32_t> m_memberCellular;
//	std::list<uint32_t> m_memberX;
//	std::list<uint32_t> m_memberY;

	ClusterMemberList m_cml;

	uint32_t m_blocked;
	uint32_t m_fullexit;

	uint32_t m_exitid;
	uint32_t m_exitcapacity;
	std::map<uint32_t,uint32_t> m_exitinfoMap;

public:
	StoreSectionData ()
	{
		m_seqNo = 0;
		m_sectionId = Position{0, 0, 0};
		m_apId = 0;
		m_status = 0;
		m_occupancy = 0;
		m_blocked = 0;
		m_fullexit = 0;
		m_exitid = 0;
		m_exitcapacity = 0;
		m_exitinfoMap = {};
//		m_cmlSize=0;
	}
	StoreSectionData (Ipv4Address source,uint32_t seqNo, Position sectionId, uint32_t apId, uint32_t status, uint32_t occupancy,ClusterMemberList cml, uint32_t b_id,uint32_t fe_id,uint32_t exitid,uint32_t exitcapacity,std::map<uint32_t,uint32_t> exitinfoMap)
	{
		m_source=source;
		m_seqNo = seqNo;
		m_sectionId = sectionId;
		m_apId = apId;
		m_status = status;
		m_occupancy = occupancy;

		m_cml = cml;

		m_blocked = b_id;
		m_fullexit = fe_id;

		m_exitid = exitid;
		m_exitcapacity = exitcapacity;

		m_exitinfoMap = exitinfoMap;
//		this->SetClusterMemberList(cml);
	}



	void PrintList() const;
	void Print();
	DataKey GetKey(){return DataKey(m_source, m_seqNo);}

	void SetSeqNo(uint32_t seqNo){m_seqNo=seqNo;}
	uint32_t GetSeqNo(){return m_seqNo;}
	void SetSectionId (Position sectionId) {m_sectionId=sectionId;}
	Position GetSectionId (void) {return m_sectionId;}
	void SetApId(uint32_t apId){m_apId=apId;}
	uint32_t GetApId(){return m_apId;}
	void SetStatus(uint32_t status){m_status=status;}
	uint32_t GetStatus(){return m_status;}
	void SetOccupancy(uint32_t occupancy){m_occupancy=occupancy;}
	uint32_t GetOccupancy(){return m_occupancy;}

	void SetClusterMemberList(ClusterMemberList cml);
	ClusterMemberList GetCoordinatelusterMemberList(){return m_cml;};
//
//	void SetClusterAllMemberList(Ipv4Address source,ClusterMemberList cml){m_caml.AddList(source, cml);}
//	ClusterAllMemberList GetCoordinatelusterAllMemberList(){return m_caml;}

	void SetSource(Ipv4Address source){m_source=source;}
	Ipv4Address GetSource(){return m_source;}

	void SetBlockedNodeID(uint32_t id){m_blocked=id;}
	uint32_t GetBlockedNodeID() {return m_blocked;}

	void SetFullexitNodeID(uint32_t id){m_fullexit=id;}
	uint32_t GetFullexitNodeID() {return m_fullexit;}

	void SetExitID(uint32_t id){m_exitid=id;}
	uint32_t GetExitID() {return m_exitid;}

	void SetExitCapacity(uint32_t capacity){m_exitcapacity=capacity;}
	uint32_t GetExitCapacity() {return m_exitcapacity;}

	void SetExitInfoMap(std::map<uint32_t, uint32_t> capacity){m_exitinfoMap=capacity;}
	std::map<uint32_t, uint32_t> GetExitInfoMap() {return m_exitinfoMap;}

};
}
}



#endif /* SCRATCH_EVACUATION_VER6_SECTION_DATA_TRADE_H_ */
