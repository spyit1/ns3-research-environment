/*
 * test.cc
 *
 *  Created on: 2015/06/05
 *      Author: terami
 */

#include "evacuation-accesspoint.h"

namespace ns3 {
namespace simple {

MyAccessPoint::MyAccessPoint ():
		m_nodeType (EVC_AP),
		m_apId (GetId()),
		m_sectionId {0, 0, 0},
		m_seqNo (0),
		m_status (UNKNOWN),
		m_occupancy (0),
		m_numTransfers (0),
		m_volTransfers (0)
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
			m_occupancyMap.insert(std::make_pair(Position{x, y, z}, MyBuilding::GetNumUsers()));
			break;
		case SAFE:
			m_occupancyMap.insert(std::make_pair(Position{x, y, z}, 0));
			break;
		case EXIT:
			m_occupancyMap.insert(std::make_pair(Position{x, y, z}, 0));
			break;
		case BLOCKED:
			m_occupancyMap.insert(std::make_pair(Position{x, y, z}, INF));
			break;
		default:
			m_occupancyMap.insert(std::make_pair(Position{x, y, z}, INF));
			break;
		}
	}}}
}

MyAccessPoint::~MyAccessPoint ()
{
	// ..
}

void MyAccessPoint::SetMyAccessPoint ()
{
	// set section-id
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
		if (posx >= left and posx <= right) {
			secx = x;
			break;
	}}
	for (uint32_t y=0; y<MyBuilding::GetNumSectionY(); y++) {
		double top = (*mySection).at(y).at(0).GetTop();
		double bottom = (*mySection).at(y).at(0).GetBottom();
		if (posy >= top and posy <= bottom) {
			secy = y;
			break;
	}}
	secz = 0;
	m_sectionId = Position{secx, secy, secz};
}

void MyAccessPoint::PrintMyAccessPoint ()
{
	Ptr<MobilityModel> obj = GetObject<MobilityModel>();
	double posx = obj->GetPosition().x;
	double posy = obj->GetPosition().y;
	double posz = obj->GetPosition().z;
	std::cout << "Print MyAccessPoint ----------" 	<< std::endl;
	std::cout << std::setw(3) << m_apId		<< ", "
			   << std::setw(10)<< posx << "/" << posy << "/" << posz << ", "
			   << std::setw(8) << m_sectionId	<< ", "
			   << std::setw(3) << m_status		<< std::endl;
	std::cout << "------------------------------" 	<< std::endl;
}

}}
