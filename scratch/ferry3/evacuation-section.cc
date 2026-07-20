/*
 * test.cc
 *
 *  Created on: 2015/06/05
 *      Author: terami
 */

#include "evacuation-section.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <random>
#include <fstream>
#include <algorithm>
#include <queue>

//using namespace ns3;
namespace ns3 {
namespace simple {

std::vector<std::vector<MySection>> m_section(MyBuilding::GetNumSectionX(), std::vector<MySection>(MyBuilding::GetNumSectionY()));
std::vector<std::vector<MySection>> &GetMySection() {return m_section;}

void MySection::SetMySection ()
{
	for (uint32_t sectionZ=0; sectionZ<MyBuilding::GetNumSectionZ(); sectionZ++) {
	for (uint32_t sectionY=0; sectionY<MyBuilding::GetNumSectionY(); sectionY++) {
	for (uint32_t sectionX=0; sectionX<MyBuilding::GetNumSectionX(); sectionX++) {
		// section id
		m_section.at(sectionY).at(sectionX).m_sectionId = {sectionX, sectionY, sectionZ};

		// border line
		double top = MyBuilding::GetSectionHight() * sectionY;
		double bottom = top + MyBuilding::GetSectionHight();
		double left = MyBuilding::GetSectionWidth() * sectionX;
		double right = left + MyBuilding::GetSectionWidth();
		m_section.at(sectionY).at(sectionX).m_top = top;
		m_section.at(sectionY).at(sectionX).m_bottom = bottom;
		m_section.at(sectionY).at(sectionX).m_left = left;
		m_section.at(sectionY).at(sectionX).m_right = right;

		// section status
		m_section.at(sectionY).at(sectionX).m_status = UNKNOWN;
		std::list<Position> els = MyBuilding::GetExitSection();
		for (const auto& e : els) {
			if (m_section.at(sectionY).at(sectionX).m_sectionId == e)
				m_section.at(sectionY).at(sectionX).m_status = EXIT;
		}
		std::list<Position> bls = MyBuilding::GetBlockedSection();
		for (const auto& e : bls) {
			if (m_section.at(sectionY).at(sectionX).GetSectionId() == e)
				m_section.at(sectionY).at(sectionX).m_status = BLOCKED;
		}
	}}}

	std::cout << "Finish setting MySection" << std::endl;
}

void MySection::PrintMySection ()
{
	std::cout << "Print MySection ----------" << std::endl;
	for (uint32_t sectionId_int=0; sectionId_int<MyBuilding::GetNumSections(); sectionId_int++)
	{
		uint32_t sectionX = sectionId_int % MyBuilding::GetNumSectionX();
		uint32_t sectionY = sectionId_int / MyBuilding::GetNumSectionY();
		std::cout	<< std::setw(8) << m_section.at(sectionY).at(sectionX).m_sectionId			<< ", "
					<< std::setw(4) << m_section.at(sectionY).at(sectionX).m_status			<< ", "
					<< std::setw(4) << m_section.at(sectionY).at(sectionX).m_top				<< ", "
					<< std::setw(4) << m_section.at(sectionY).at(sectionX).m_bottom			<< ", "
					<< std::setw(4) << m_section.at(sectionY).at(sectionX).m_left				<< ", "
					<< std::setw(4) << m_section.at(sectionY).at(sectionX).m_right				<< std::endl;
	}
	std::cout << "--------------------------" << std::endl;
}

}}
