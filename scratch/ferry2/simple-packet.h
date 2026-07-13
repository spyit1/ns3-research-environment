/*
 * simple-packet.h
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#ifndef SIMPLE_PACKET_H_
#define SIMPLE_PACKET_H_

#include <iostream>
#include <map>
#include <list>
#include <unordered_map>
#include <vector>
#include <set>
#include <string>

#include "cluster-list.h"
#include "evacuation-accesspoint.h"
#include "evacuation-user.h"
#include "simple-rtable.h"
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"


#define HOGEHOGE 0
#define FLOODING 1
#define ADHOC_CLUSTER 5

namespace ns3 {
namespace simple {
//必要なパケットを識別するための識別子を定義しておく
enum MessageType
{
	TYPE_META_DATA = 1,
	TYPE_HELLO = 2,
	TYPE_USERDATA = 3,
	TYPE_SHELTERDATA = 4,
	TYPE_REQUEST = 5,
	// TYPE_UAVDATA = 6
};

class TypeHeader : public Header
{
public:
	TypeHeader(MessageType t = TYPE_HELLO);

	static TypeId GetTypeId (void);
	TypeId GetInstanceTypeId (void) const;
	uint32_t GetSerializedSize (void) const; //カプセル化後のヘッダサイズを取得する関数
	void Serialize (Buffer::Iterator start) const; //ヘッダ部のカプセル化関数
	uint32_t Deserialize (Buffer::Iterator start); //ヘッダ部のデカプセル化関数
	void Print (std::ostream &os) const;

	//プロトコルパケットのタイプ識別子を取得する関数
	MessageType Get (void) const {return m_type;}
	bool IsValid (void) const {return m_valid;}
	bool operator== (TypeHeader const & o) const;

private:
	MessageType m_type;
	bool m_valid;
};
std::ostream &operator << (std::ostream &os,TypeHeader const &h);


class UserData: public Header
{
private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_source;
	Ipv4Address m_dst;
	uint32_t m_userId;
	std::set<String> m_blocked_set;
	std::set<String> m_blocked_set_buf;
	std::set<String> m_fullexit_set;
	std::map <String, std::pair<uint32_t, uint32_t>> m_aoi_exitinfo;
	std::map <String, float> m_aoi_blockinfo;
	uint32_t m_broadcast_id;
	std::pair<String, uint32_t>my_patrol_erea_and_start_time; //nodeid and patrol start time
	std::map<String,uint32_t>other_UAV_patrol_erea_and_id; //ereaid and UAVid(Key is ereaid(nodeid))
	std::map<String, uint32_t> patrol_ereaid_and_time;
	

	// std::map <Ipv4Address, std::pair<String, uint32_t>> m_hop;


public:

	UserData(){
		m_userId = 0;
	}

	UserData (uint32_t userId, Ipv4Address dst, std::set<String> b_set, std::set<String> fullexit_set, std::map <String, std::pair<uint32_t, uint32_t>> aoi_exitinfo, std::map <String, float> aoi_blockinfo)
	{
		m_userId = userId;
		m_dst=dst;
		for(auto b : b_set){
			m_blocked_set.insert(b);
		}
		m_fullexit_set = fullexit_set;
		m_aoi_exitinfo = aoi_exitinfo;
		m_aoi_blockinfo = aoi_blockinfo;
		// m_hop = hop;
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;
	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;
	uint64_t GetSerializedSize(bool flag) const;
	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void PrintList() const;
	void Print (std::ostream &os) const;
	bool operator==(UserData const &o) const;
	void SetUserId(uint32_t userId){m_userId=userId;}
	uint32_t GetUserId(){return m_userId;}
	void SetDst(Ipv4Address dst){m_dst=dst;}
	Ipv4Address GetDst(){return m_dst;}
	void SetBlockedNodeIDSet(String id){m_blocked_set.insert(id);}
	std::set<String> GetBlockedNodeIDSet(void){return m_blocked_set;}
	uint32_t GetBlockedNodeIDSetSize(void){return m_blocked_set.size();}
	std::set<String> GetFullExitNodeIDSet(void){return m_fullexit_set;}
	std::map<String, std::pair<uint32_t, uint32_t>> GetAoIExitinfo(void){return m_aoi_exitinfo;}
	std::map<String, float> GetAoIBlockedInfo(void){return m_aoi_blockinfo;}
	void SetBroadcastId(uint32_t id){m_broadcast_id=id;}
	uint32_t GetBroadcastId(){return m_broadcast_id;}
	void SetId (Ipv4Address id) { m_source = id; }
	Ipv4Address GetId () const { return m_source; }
	void SetMyPatrolErea_and_StartTime(std::pair<String,uint32_t> data){my_patrol_erea_and_start_time = data;}
	void SetOtherUAVPatrolErea_and_Id(std::map<String,uint32_t> data){other_UAV_patrol_erea_and_id = data;}
	void SetPatrolErea_and_Time(std::map<String,uint32_t> data){patrol_ereaid_and_time = data;}
	std::pair<String,uint32_t> GetMyPatrolErea_and_StartTime(){return my_patrol_erea_and_start_time;}
	std::map<String,uint32_t> GetOtherUAVPatrolErea_and_Id(){return other_UAV_patrol_erea_and_id;}
	std::map<String,uint32_t> GetPatrolErea_and_Time(){return patrol_ereaid_and_time;}
	// void SetBlockedNodeIDSetBuf(){m_blocked_set_buf = m_blocked_set;}
	// std::set<String> GetBlockedNodeIDSetBuf(void){return m_blocked_set_buf;}


};
std::ostream &operator << (std::ostream &os,UserData const &);

class SummaryVector : public Header
{
public:
        SummaryVector() {}
        SummaryVector(uint32_t userId,
                      Ipv4Address dst,
                      std::map<String, uint32_t> exitSummary,
                      std::map<String, float> blockSummary)
                : m_userId(userId),
                  m_dst(dst),
				  m_broadcast_id(0),
                  m_exitSummary(exitSummary),
                  m_blockSummary(blockSummary)
                  
        {
        }

        static TypeId GetTypeId(void);
        TypeId GetInstanceTypeId(void) const;
        uint32_t GetSerializedSize(void) const;
        void Serialize(Buffer::Iterator i) const;
        uint32_t Deserialize(Buffer::Iterator start);
        void Print(std::ostream &os) const;

        uint32_t GetUserId(void) const { return m_userId; }
        Ipv4Address GetDst(void) const { return m_dst; }

        void SetBroadcastId(uint32_t id) { m_broadcast_id = id; }
        uint32_t GetBroadcastId(void) const { return m_broadcast_id; }

        std::map<String, uint32_t> GetExitSummary(void) const { return m_exitSummary; }
        std::map<String, float> GetBlockSummary(void) const { return m_blockSummary; }

private:
        uint32_t m_userId;
        Ipv4Address m_dst;
        uint32_t m_broadcast_id;

        std::map<String, uint32_t> m_exitSummary;
        std::map<String, float> m_blockSummary;
};

std::ostream &operator << (std::ostream &os, SummaryVector const &h);


class RequestData : public Header
{
public:
    RequestData() {}

    RequestData(uint32_t requesterId,
                std::vector<String> exitIds,
                std::vector<String> blockIds)
        : m_requesterId(requesterId),
          m_exitIds(exitIds),
          m_blockIds(blockIds)
    {
    }

    static TypeId GetTypeId(void);
    TypeId GetInstanceTypeId(void) const;

    uint32_t GetSerializedSize(void) const;
    void Serialize(Buffer::Iterator i) const;
    uint32_t Deserialize(Buffer::Iterator start);
    void Print(std::ostream &os) const;

    uint32_t GetRequesterId(void) const { return m_requesterId; }
    std::vector<String> GetExitIds(void) const { return m_exitIds; }
    std::vector<String> GetBlockIds(void) const { return m_blockIds; }

private:
    uint32_t m_requesterId;
    std::vector<String> m_exitIds;
    std::vector<String> m_blockIds;
};

std::ostream &operator << (std::ostream &os, RequestData const &h);


class Hop : public Header
{
public:

	Hop(){}

	Hop(std::map<uint32_t, std::pair<std::pair<String, uint32_t>, uint32_t>> hop){
		m_hop = hop;
		// for(auto a : m_hop){
		// 	std::cout<<a<<std::endl;
		// }
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);

	void SetHop(uint32_t id, std::pair<std::pair<String, uint32_t>, uint32_t> time){m_hop.insert(std::make_pair(id,time));} //insertなのですでに要素がある場合は置き換えない
	void SetHop(std::map<uint32_t, std::pair<std::pair<String, uint32_t>, uint32_t>> hop){m_hop = hop;} //置き換え
	void UpdateHop(uint32_t id, std::pair<std::pair<String, uint32_t>, uint32_t> time){ //上書き
		m_hop.erase(id);
		m_hop.insert(std::make_pair(id,time));
	}
	std::map<uint32_t, std::pair<std::pair<String, uint32_t>, uint32_t>> GetHop(){return m_hop;}

	void Print(std::ostream &os) const;

	bool operator==(Hop const &o) const;

private:
	mutable uint32_t m_serialSize;
	std::map<uint32_t, std::pair<std::pair<String, uint32_t>, uint32_t>>m_hop; //ipアドレス，目標とする避難所ID，避難所までの所要時間，情報生成時間
};
std::ostream &operator << (std::ostream &os,Hop const &);



class Hello : public Header
{
public:

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void Print(std::ostream &os) const;

	void SetId (Ipv4Address id) { m_id = id; }
	Ipv4Address GetId () const { return m_id; }

	void SetBroadcastId(uint32_t id){m_broadcast_id=id;}
	uint32_t GetBroadcastId(){return m_broadcast_id;}

	void SetUserID(uint32_t userID){m_userID=userID;}
	uint32_t GetUserID(){return m_userID;}

	bool operator==(Hello const &o) const;

private:
	mutable uint32_t m_serialSize;
	Ipv4Address m_id;
	uint32_t m_broadcast_id;
	uint32_t m_userID;
};
std::ostream &operator << (std::ostream &os,Hello const &);

// class UAVdata : public Header
// {
// public:

// 	static TypeId GetTypeId();
// 	TypeId GetInstanceTypeId() const;

// 	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
// 	uint32_t GetSerializedSize() const;

// 	void Serialize(Buffer::Iterator start) const;
// 	uint32_t Deserialize (Buffer::Iterator start);
// 	void Print(std::ostream &os) const;

// 	void SetMyPatrolErea_and_StartTime(std::pair<String,double> data){my_patrol_erea_and_start_time = data;}
// 	void SetOtherUAVPatrolErea_and_Id(std::map<String,String> data){other_UAV_patrol_erea_and_id = data;}
// 	std::pair<String,double> GetMyPatrolErea_and_StartTime(){return my_patrol_erea_and_start_time;}
// 	std::map<String,String> GetOtherUAVPatrolErea_and_Id(){return other_UAV_patrol_erea_and_id;}
// 	bool operator==(UAVdata const &o) const;

// private:
// 	mutable uint32_t m_serialSize;
// 	std::pair<String,double> my_patrol_erea_and_start_time = {}; //nodeid and patrol start time
// 	std::map<String,String> other_UAV_patrol_erea_and_id = {}; //ereaid and UAVid(Key is ereaid(nodeid))
// 	uint32_t m_userID;
// };
// std::ostream &operator << (std::ostream &os,UAVdata const &);

class ShelterInfo: public Header
{
private:
	mutable uint32_t m_serialSize;
	Ipv4Address m_id;
	uint32_t m_broadcast_id;
	uint32_t m_exit_num;
	uint32_t m_shelterID;
	uint32_t m_time;


public:

	ShelterInfo ()
	{
		m_shelterID = 0;
		m_exit_num = 0;
		m_time =  0;

	}
	ShelterInfo (uint32_t shelterid, uint32_t exit_num, uint32_t time)
	{
		// m_dst = dst;
		m_shelterID = shelterid;
		m_exit_num = exit_num;
		m_time = time;

	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;
	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;
	uint64_t GetSerializedSize(bool flag) const;
	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void PrintList() const;
	void Print (std::ostream &os) const;
	bool operator==(ShelterInfo const &o) const;
	// DataKey GetKey(){return DataKey(m_source, m_seqNo);}
	void SetId (Ipv4Address id) { m_id = id; }
	Ipv4Address GetId () const { return m_id; }
	void SetBroadcastId(uint32_t id){m_broadcast_id=id;}
	uint32_t GetBroadcastId(){return m_broadcast_id;}
	uint32_t GetShelterId(void){return m_shelterID;}
	uint32_t GetExitNum(void){return m_exit_num;}
	uint32_t GetTime(void){return m_time;}
};
std::ostream &operator << (std::ostream &os,ShelterInfo const &);

}}

#endif /* SIMPLE_PACKET_H_ */
