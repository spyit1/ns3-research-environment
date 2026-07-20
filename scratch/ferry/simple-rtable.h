/*
 * simple-rtable.h
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#ifndef SIMPLE_RTABLE_H_
#define SIMPLE_RTABLE_H_

#include <stdint.h>
#include <cassert>
#include <map>
#include <unordered_map>
#include <sys/types.h>
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/timer.h"
#include "ns3/net-device.h"
#include "ns3/output-stream-wrapper.h"


namespace ns3
{
namespace simple
{
enum RouteFlags
{
	VALID = 0,          //!< VALID
	INVALID = 1,        //!< INVALID
	IN_SEARCH = 2,      //!< IN_SEARCH
};

//経路エントリークラスの定義
class RoutingTableEntry
{
public:
	//コントラクタ
	RoutingTableEntry(Ptr<NetDevice> dev=0,
			Ipv4Address dst=Ipv4Address(),
			Ipv4InterfaceAddress iface=Ipv4InterfaceAddress(),
			Ipv4Address nextHop=Ipv4Address(),
			uint16_t metrics=0,
			std::string rtype="",
			std::string iftype="",
			Time lifetime=Simulator::Now());

	~RoutingTableEntry();

	//あて先IPアドレスの取得
	Ipv4Address GetDestination() const {return m_ipv4Route->GetDestination();}

	//経路情報の取得
	Ptr<Ipv4Route> GetRoute() const {return m_ipv4Route;}
	void setRoute(Ptr<Ipv4Route> r){m_ipv4Route=r;}

	//ネクストホップを設定する
	void SetNextHop(Ipv4Address nextHop){m_ipv4Route->SetGateway(nextHop);}
	//ネクストホップを取得する
	Ipv4Address GetNextHop() const {return m_ipv4Route->GetGateway();}

	//出力デバイスを設定する
	void SetOutputDevice(Ptr<NetDevice> dev){m_ipv4Route->SetOutputDevice(dev);}
	//出力デバイスを取得する
	Ptr<NetDevice> GetOutputDevice() const {return m_ipv4Route->GetOutputDevice();}

	//インターフェースアドレスを設定する
	void SetInterface(Ipv4InterfaceAddress iface){m_iface=iface;}
	//インターフェイスアドレスを取得する
	Ipv4InterfaceAddress GetInterface() const {return m_iface;}

	//メトリックを設定する
	void SetMetrics(uint8_t metrics){m_metrics=metrics;}
	//メトリックを取得する
	uint8_t GetMetrics() const {return m_metrics;}

	//経路エントリの維持時間を設定する
	void SetLifeTime(Time lt){m_lifetime=lt+Simulator::Now();}
	//経路エントリの維持時間を取得する
	Time GetLifeTime() const {return m_lifetime-Simulator::Now();}

	//経路タイプ(C/R/S)を設定する
	void SetRtype(std::string rtype){m_rtype=rtype;}
	//経路タイプを取得する
	std::string GetRtype(){return m_rtype;}

	//I/Fタイプ(p2p,csma,wifi)を設定する
	void SetIFtype(std::string iftype){m_iftype=iftype;}
	//I/Fタイプを取得する
	std::string GetIFtype(){return m_iftype;}

	//経路エントリーをファイルに出力
	void Print(Ptr<OutputStreamWrapper> stream) const;
	//経路エントリーを画面に出力
	void Print() const;

	bool operator==(Ipv4Address const dst) const {
		return (m_ipv4Route->GetDestination()==dst);
	}

	void SetFlag (RouteFlags flag) { m_flag = flag; }
	RouteFlags GetFlag () const { return m_flag; }



private:
	//経路情報
	Ptr<Ipv4Route> m_ipv4Route;

	//メトリック情報
	uint8_t m_metrics;

	//出力I/Fアドレス
	Ipv4InterfaceAddress m_iface;

	//経路タイプ
	std::string m_rtype;

	//I/Fタイプ
	std::string m_iftype;

	//経路エントリーの維持時間
	Time m_lifetime;

	RouteFlags m_flag;
};


//ルーティングテーブルクラスの定義
class RoutingTable
{
public:
	RoutingTable();

	//ルーティングテーブルに経路エントリーを追加する
	bool AddRoute(RoutingTableEntry &r);
	//ルーティングテーブルから経路エントリーを削除する
	bool DeleteRoute(Ipv4Address dst);

	//ルーティングテーブルからあて先dstの経路エントリーを検索する
	bool LookupRoute(Ipv4Address dst,RoutingTableEntry &rt);

	bool LookupValidRoute (Ipv4Address dst, RoutingTableEntry & rt);

	//ルーティングテーブルを更新する
	bool Update(RoutingTableEntry &rt);

	//ルーティングテーブルのすべてのエントリーを削除する
	void Clear(){m_RoutingTable.clear();}

	//ルーティングテーブルをファイルに出力する
	void Print(Ptr<OutputStreamWrapper> stream) const;

	void Print();

	//ルーティングテーブルを取得する
	std::map<Ipv4Address,RoutingTableEntry> m_RoutingTable;
	//std::unordered_map<Ipv4Address,RoutingTableEntry,Ipv4Address::Hash> m_RoutingTable;
};
}
}



#endif /* SIMPLE_RTABLE_H_ */
