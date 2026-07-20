/*
 * data-counter.h
 *
 *  Created on: 2015/12/21
 *      Author: terami
 */

#ifndef DATA_COUNTER_H_
#define DATA_COUNTER_H_

#include <unordered_map>

#include "cluster-list.h"

namespace ns3{
namespace simple{

class SenderInfo{
public:
	SenderInfo(){m_x=0.0;m_y=0.0;m_sendTime=0.0;}
	SenderInfo(double x,double y,double sendTime){
		m_x=x;
		m_y=y;
		m_sendTime=sendTime;
	}

	void SetX(double x){m_x=x;}
	double GetX(){return m_x;}

	void SetY(double y){m_y=y;}
	double GetY(){return m_y;}

	void SetSendTime(double sendTime){m_sendTime=sendTime;}
	double GetSendTime(){return m_sendTime;}

	void Print(){
		std::cout<<m_x<<","<<m_y<<","<<m_sendTime<<std::endl;
	}
private:
	double m_x;
	double m_y;
	double m_sendTime;
};

class SenderInfoList : public std::unordered_map<DataKey,SenderInfo,DataKey::Hash> {
public:
	void Print(){
		std::cout<<"============================"<<std::endl;
		std::cout<<"SenderInfo: Sender,SeqNo,X,Y"<<std::endl;
		std::cout<<"============================"<<std::endl;

		SenderInfoList::iterator sili;
		for(sili=this->begin();sili!=this->end();sili++){
			std::pair<DataKey,SenderInfo> pair=*sili;
			std::cout<<pair.first.GetId()<<","<<pair.first.GetSeqNo()<<",";
			pair.second.Print();
		}
		std::cout<<std::endl;
	}
};

class ReceiverInfo{
public:
	ReceiverInfo(){m_x=0.0;m_y=0.0;m_distance=-1;m_disseminationTime=-1;}
	ReceiverInfo(double x,double y,double distance,double disseminationTime){
		m_x=x;
		m_y=y;
		m_distance=distance;
		m_disseminationTime=disseminationTime;
	}

	void SetX(double x){m_x=x;}
	double GetX(){return m_x;}

	void SetY(double y){m_y=y;}
	double GetY(){return m_y;}

	void SetDistance(double distance){m_distance=distance;}
	double GetDistance(){return m_distance;}

	void SetDisseminationTime(double disseminationTime){m_disseminationTime=disseminationTime;}
	double GetDisseminationTime(){return m_disseminationTime;}

private:
	double m_x;
	double m_y;
	double m_distance;
	double m_disseminationTime;
};

class ReceiverInfoList : public std::unordered_map<DataKey,ReceiverInfo,DataKey::Hash> {

};



}
}



#endif /* DATA_COUNTER_H_ */
