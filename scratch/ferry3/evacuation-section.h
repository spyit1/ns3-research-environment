/*
 * test.h
 *
 *  Created on: 2019/12/25
 *      Author: fujinaka
 */

#ifndef SIMPLE_SECTION_H_
#define SIMPLE_SECTION_H_

#include <stdint.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <list>
#include <queue>

#include "evacuation-building.h"

namespace ns3
{
namespace simple
{

class MySection
{
private:
	Position m_sectionId;
	uint32_t m_status;
	double m_top;
	double m_bottom;
	double m_left;
	double m_right;

public:
	void SetSectionId (Position pos) {m_sectionId=pos;}
	Position GetSectionId (void) {return m_sectionId;}
	void SetStatus (uint32_t status) {m_status=status;}
	uint32_t GetStatus (void) {return m_status;}
	void SetTop (double top) {m_top=top;}
	double GetTop (void) {return m_top;}
	void SetBottom (double bottom) {m_bottom=bottom;}
	double GetBottom (void) {return m_bottom;}
	void SetLeft (double left) {m_left=left;}
	double GetLeft (void) {return m_left;}
	void SetRight (double right) {m_right=right;}
	double GetRight (void) {return m_right;}

	static void SetMySection (void);
	static void PrintMySection (void);
};
std::vector<std::vector<MySection>> &GetMySection();

}}

#endif /* SIMPLE_SECTION_H_ */
