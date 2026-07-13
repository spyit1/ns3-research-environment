/*
 * test.h
 *
 *  Created on: 2016/03/03
 *      Author:
 */

#ifndef EVACUATION_ACCESSPOINT_H_
#define EVACUATION_ACCESSPOINT_H_

#include <stdint.h>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <list>

#include "evacuation-section.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/node-container.h"
#include "ns3/waypoint.h"
#include "ns3/waypoint-mobility-model.h"


#define INF 10000000

namespace ns3 {
namespace simple {

class MyAccessPoint: public Node
{
private:
	uint32_t m_nodeType;
	uint32_t m_apId;
	Position m_sectionId;
	uint32_t m_seqNo;
	uint32_t m_status;
	uint32_t m_occupancy;
	uint32_t m_numTransfers;
	uint32_t m_volTransfers;

	std::list<uint32_t> m_recvSeqNo;
	std::vector<uint32_t> m_userVector;
	std::map<Position, uint32_t> m_statusMap;
	std::map<Position, uint32_t> m_occupancyMap;

public:
	MyAccessPoint ();
	~MyAccessPoint ();

	void SetNodeType (uint32_t nodeType) {m_nodeType=nodeType;}
	uint32_t GetNodeType (void) {return m_nodeType;}
	void SetAccessPointId (uint32_t apId) {m_apId=apId;}
	uint32_t GetAccessPointId (void) {return m_apId;}
	void SetSectionId (Position sectionId) {m_sectionId=sectionId;}
	Position GetSectionId (void) {return m_sectionId;}
	void SetSeqNo(uint32_t seqNo){m_seqNo=seqNo;}
	uint32_t GetSeqNo(){return m_seqNo;}
	void SetStatus (uint32_t status) {m_status=status;}
	uint32_t GetStatus (void) {return m_status;}
	void SetNumTransfers (uint32_t numTransfers) {m_numTransfers=numTransfers;}
	uint32_t GetNumTransfers (void) {return m_numTransfers;}
	void SetVolTransfers (uint32_t volTransfers) {m_volTransfers=volTransfers;}
	uint32_t GetVolTransfers (void) {return m_volTransfers;}
	void SetOccupancy (uint32_t occupancy) {m_occupancy=occupancy;}
	uint32_t GetOccupancy (void) {return m_occupancy;}
	void SetUserVector (std::vector<uint32_t> userVector) {m_userVector=userVector;}
	std::vector<uint32_t> GetUserVector (void) {return m_userVector;}
	void SetStatusMap (std::map<Position, uint32_t> statusMap) {m_statusMap=statusMap;}
	std::map<Position, uint32_t> GetStatusMap (void) {return m_statusMap;}
	void SetOccupancyMap (std::map<Position, uint32_t> occMap) {m_occupancyMap=occMap;}
	std::map<Position, uint32_t> GetOccupancyMap (void) {return m_occupancyMap;}

	void PushRecvSeqNo(uint32_t seqNo){m_recvSeqNo.push_back(seqNo);}
	void PopRecvSeqNo(){m_recvSeqNo.pop_front();}
	std::list<uint32_t> GetRecvSeqNo(){return m_recvSeqNo;}

	void SetMyAccessPoint (void);
	void PrintMyAccessPoint (void);
};

std::vector<std::vector<MyAccessPoint>> &GetMyAccessPoint();
}}

#endif /* EVACUATION_ACCESSPOINT_H_ */
