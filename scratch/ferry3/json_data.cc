/*
 * json_data.cc
 *
 *  Created on: 2021/07/05
 *      Author: matsunaga
 */
#include "json_data.h"
#include "evacuation-building.h"
//#include "simple-protocol.h"
#include "DTN-protocol.h"
//#include "DTN-helper.h"

#include <algorithm>
#include <limits>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <sstream>
#include <unordered_map>

//#include <nlohmann/json.hpp>
//using json = nlohmann::json;
//using String =std::string;


namespace ns3{
namespace simple{

json JSON_Data::j;
std::list<json> JSON_Data::features;
std::map<String,way_info> JSON_Data::linestrings;
//	std::list<way_node> nodes;
std::map<std::pair<double,double>,way_node> JSON_Data::nodes;
std::map<std::pair<double,double>,way_node> JSON_Data::graph;
std::map<std::pair<double,double>,way_node> JSON_Data::s_graph;
std::list<json> JSON_Data::points;
std::list<std::pair<double,double>> JSON_Data::hinanjo;
std::map<std::pair<double,double>, uint32_t> JSON_Data::capacity;

Timer JSON_Data::updTimer;

uint32_t JSON_Data::nodenum;
std::map<String,std::pair<double,double>> JSON_Data::id_to_coordinate;

// ido
double JSON_Data::latitude_max=0.0;
double JSON_Data::latitude_min=9999.0;

// keido
double JSON_Data::longitude_max=0.0;
double JSON_Data::longitude_min=9999.0;

uint32_t JSON_Data::update_num = 0;
std::list<String> JSON_Data::now_blocked;

// void way_node::Print(){
// 	std::cout<<std::fixed;
// 	std::cout<<nodeID<<std::endl;
// 	std::cout<<"-----------------------------------------------------"<<std::endl;
// 	std::cout<<"Coordinate:["<<std::setprecision(7)<<Coordinate.first<<", "
// 			<<std::setprecision(7)<<Coordinate.second<<"]"<<std::endl;
// 	std::cout<<"Affiliation_way:";
// 	for(auto itr = wayIDs.begin();itr != wayIDs.end();++itr){
// 		String ID = *itr;
// 		if(itr==wayIDs.begin()) std::cout<<ID;
// 		else std::cout<<","<<ID;
// 	}
// 	std::cout<<std::endl;
// 	std::cout<<"intersection:";
// 	if(isCrossing) std::cout<<"true"<<std::endl;
// 	else std::cout<<"false"<<std::endl;

// 	if(isCrossing){
// 		std::cout<<"neighbor_node:"<<std::endl;
// 		for(auto itr = neighbor_node_coordinate.begin();itr != neighbor_node_coordinate.end();++itr){
// 			std::pair<double,double> coord = itr->first;
// 			double dst = itr->second;
// 			std::cout<<"<"<<coord.first<<","<<coord.second<<">:"<<dst<<"m"<<std::endl;
// 		}
// 	}


// 	std::cout<<"Blocked:";
// 	if(isBlocked) std::cout<<"true"<<std::endl;
// 	else std::cout<<"false"<<std::endl;

// 	std::cout<<"-----------------------------------------------------"<<std::endl;
// }

void way_info::Print(){
	//std::cout<<"============================="<<w_id<<"=========================="<<std::endl;
	//std::cout<<"properties :"<<properties<<std::endl;
	for(auto itr = w_nodes.begin();itr != w_nodes.end();++itr){
		way_node it =*itr;
		// it.Print();
	}
//	std::cout<<"----------------------------------------"<<std::endl;
//	for(auto itr = ws.begin();itr != ws.end();++itr){
//		way_node it =itr->second;
//		it.Print();
//	}
//	for(auto itr = ws1.begin();itr != ws1.end();++itr){
//		way_node it =itr->second;
//		it.Print();
//	}
	//std::cout<<"length:"<<w_length<<"m , node_num :"<<w_nodes.size()<<std::endl;
//	std::cout<<"===================================================================="<<std::endl;
}


void JSON_Data::SetGraph(way_node node){
    	way_node wno=nodes.at(node.GetCoordinate());

    	decltype(s_graph)::iterator it = s_graph.find(node.GetCoordinate());

    	if(it != s_graph.end()) {
    		return;
    	}else{
    		s_graph.insert(make_pair(node.GetCoordinate(),wno));
    	}

    	std::map<std::pair<double,double>,double> n_node_map = wno.GetNeighborNodes();
    	for(auto itr2 = n_node_map.begin();itr2 != n_node_map.end();++itr2){
    		std::pair<double,double> it2 = itr2->first;
    		way_node n_wno = nodes.at(it2);
    		SetGraph(n_wno);
    	}
}


void JSON_Data::json_input(){
	const std::string pathToJSON_field = "./data/" + MyBuilding::GetFieldName() + "/" + MyBuilding::GetFieldName() + ".geojson";
	// const std::string pathToJSON_field = "./data/itukaichi.geojson";
	std::cout<<pathToJSON_field<<std::endl;
    std::ifstream ifs(pathToJSON_field.c_str());
    if (ifs.good())
    {
        ifs >> j;
//        for (const auto &elem : j.items())
//        {
//            std::cout << elem.value() << std::endl;
//
//        }
//        std::cout << j[nl::json::json_pointer("/features/1/geometry/coordinates")]<< std::endl;
        int i =0;
        while(1){
          String s = "/features";
        	s += "/" + std::to_string(i);
        	if(j[json::json_pointer(s)].is_null()){
        		break;
        	}
        	features.push_back(j[json::json_pointer(s)]);
//        	json_print(s);
        	i++;
        }

        for(auto itr = features.begin();itr != features.end();++itr){
        	json feature = *itr;


        	if(feature[json::json_pointer("/geometry/type")]=="Point"){
        		points.push_back(feature);
				String s = "/geometry/coordinates";
				way_node wn;
				wn.SetCoordinate(feature[json::json_pointer(s)]);

				decltype(nodes)::iterator its = nodes.find(wn.GetCoordinate());
				if(its == nodes.end()){
//					nodes.push_back(wn);
					nodes.insert(std::make_pair(wn.GetCoordinate(),wn));
				}

				// std::cout<<feature<<std::endl;
        	}else if(feature[json::json_pointer("/geometry/type")]=="LineString"){
        		String s = "/geometry/coordinates";
        		String wayid = feature[json::json_pointer("/id")];
        		way_info wi;
        		double dst=0.0;

        		wi.SetProperties(feature[json::json_pointer("/properties")]);
        		wi.SetID(wayid);
//        		linestrings.push_back(feature);
        		i=0;
        		std::pair<double,double> p_coord(std::make_pair(0.0,0.0));

				while(1){
					String s = "/geometry/coordinates";
					way_node wn;
					s += "/" + std::to_string(i);
					if(feature[json::json_pointer(s)].is_null()){
						break;
					}
					std::pair<double,double> coord = std::make_pair(feature[json::json_pointer(s)][json::json_pointer("/0")],feature[json::json_pointer(s)][json::json_pointer("/1")]);

					if(p_coord==std::make_pair(0.0,0.0)){
//						std::cout<<"tes"<<std::endl;
						p_coord=coord;
					}else{
//						std::cout<<"p_coord:("<<p_coord.first<<","<<p_coord.second<<")"<<std::endl;
//						std::cout<<"coord:("<<coord.first<<","<<coord.second<<")"<<std::endl;
						dst = dst + calculate_distance(p_coord,coord);
						p_coord=coord;
//						std::cout<<"dst:"<<dst<<"m"<<std::endl;
					}


					decltype(nodes)::iterator itr = nodes.find(coord);

					// If a node has already been added
					if(itr != nodes.end()){
						way_node it = itr->second;
						wn = it;
						nodes.erase(coord);
						wn.SetWayIDs(wayid);
						wn.SetCrossingflag(true);
						wi.SetNodes(wn);
						nodes.insert(std::make_pair(wn.GetCoordinate(),wn));

						std::list<String> wn_ID_list = wn.GetWayIDs();
						for(auto itr2 = wn_ID_list.begin();itr2 != wn_ID_list.end();++itr2){
								String it1 = *itr2;
								decltype(linestrings)::iterator itr3 = linestrings.find(it1);
								if(itr3 != linestrings.end()){
									way_info it2 = itr3->second;
									std::list<way_node> wnl=it2.GetNodes();
									std::list<way_node> swap_wnl;
									for(auto itr4 = wnl.begin();itr4 != wnl.end();++itr4){
										way_node it2 = *itr4;
										if(it2.GetCoordinate()==wn.GetCoordinate()){
											swap_wnl.push_back(wn);
										}else swap_wnl.push_back(it2);
									}
									linestrings.at(it1).SetNodeList(swap_wnl);
								}else{

								}
						}


					}else{
						//交差点以外の処理？
						//全ノード交差点として設定する
						wn.SetWayNode(feature[json::json_pointer(s)],wayid,false,false,false);
						wi.SetNodes(wn);

						nodes.insert(std::make_pair(wn.GetCoordinate(),wn));
					}
					i++;
				}
//				std::cout<<wayid<<"---";
//				std::cout<<"dst(m):"<<dst<<std::endl;
				wi.SetWayLength(dst);
				linestrings.insert(std::make_pair(wayid,wi));
//				linestrings.at(wayid).Print();
        	}
        }


        // neighbor nodes info set
        for(auto itr = linestrings.begin();itr != linestrings.end();++itr){
        	way_info it = itr->second;
//        	std::cout<<"===="<<it.GetID()<<"===="<<std::endl;
//        	it.Print();
        	std::list<way_node> way_n = it.GetNodes();

        	way_node wn = way_n.front();
        	way_node p_wn=wn;
//        	auto it_b = way_n.begin();

        	double neighbor_dist=0.0;
        	for(auto itr_wnl = ++way_n.begin();itr_wnl != way_n.end();++itr_wnl){
        		way_node it2 = *itr_wnl;
        		double node_dist=0.0;

//        		std::cout<<"p_coord:("<<p_wn.GetCoordinate().first<<","<<p_wn.GetCoordinate().second<<")"<<std::endl;
//				std::cout<<"neighbor_coord:("<<wn.GetCoordinate().first<<","<<wn.GetCoordinate().second<<")"<<std::endl;
//				std::cout<<"coord:("<<it2.GetCoordinate().first<<","<<it2.GetCoordinate().second<<")"<<std::endl;

        		Hubeny H;
        		H.hubeny_distance(it2.GetCoordinate().first,it2.GetCoordinate().second,p_wn.GetCoordinate().first,p_wn.GetCoordinate().second,node_dist);

//        		std::cout<<"dst:"<<node_dist<<"m"<<std::endl;
        		neighbor_dist = neighbor_dist + node_dist;
        		p_wn=it2;



        		if(it2.GetCrossingflag()||itr_wnl==--way_n.end()){
//        			std::cout<<"-----neighbor_dist-----:"<<neighbor_dist<<"m"<<std::endl;
        			wn.SetNeighborNode(it2.GetCoordinate(),neighbor_dist);
        			nodes.at(wn.GetCoordinate()).SetNeighborNode(it2.GetCoordinate(),neighbor_dist);
//        			nodes.at(wn.GetCoordinate()).Print();

        			it2.SetNeighborNode(wn.GetCoordinate(),neighbor_dist);
        			nodes.at(it2.GetCoordinate()).SetNeighborNode(wn.GetCoordinate(),neighbor_dist);
//        			nodes.at(it2.GetCoordinate()).Print();

        			neighbor_dist = 0.0;
        			wn=it2;


        		}else{

        		}
//        	itr->second.SetNodeList(way_n);
//        	itr->second.Print();
        	}
        }

        int cross_nodes=0;

        std::list<std::pair<double,double>> erase_nodes;
        for(auto itr = nodes.begin();itr != nodes.end();++itr){
        	// Road information updates
        	std::pair<double,double> node = itr->first;
        	way_node wno=itr->second;

			std::list<String> wn_ID_list = wno.GetWayIDs();
			for(auto itr = wn_ID_list.begin();itr != wn_ID_list.end();++itr){
					String it = *itr;
					decltype(linestrings)::iterator itr2 = linestrings.find(it);
					way_info it2 = itr2->second;
					std::list<way_node> wnl = it2.GetNodes();
					for(auto itr3 = wnl.begin();itr3 != wnl.end();++itr3){
						way_node it3=*itr3;
						if(wno.GetCoordinate()==it3.GetCoordinate()){
							itr3=wnl.erase(itr3);
							wnl.insert(itr3,wno);
							linestrings.at(it).SetNodeList(wnl);
//							linestrings.at(it).Print();

							break;
						}
					}
			}



        	if(node.first>longitude_max) longitude_max = node.first;
        	if(node.first<longitude_min) longitude_min = node.first;

        	if(node.second>latitude_max) latitude_max = node.second;
        	if(node.second<latitude_min) latitude_min = node.second;



        	if(wno.GetCrossingflag()) {
        		SetGraph(wno);
        		if(graph.empty()){
        			graph = s_graph;
        		}else if(graph.size()<s_graph.size()){
//        			for(auto itr = graph.begin();itr != graph.end();++itr){
//        				way_node it = itr->second;
//        				std::list<String> st = it.GetWayIDs();
//        				for(auto itr2 = st.begin();itr2 != st.end();++itr2){
//        					String it2 = *itr2;
//        					way_info wi = linestrings.at(it2);
//        					std::list<way_node> wn = wi.GetNodes();
//        					for(auto itr3 = wn.begin();itr3 != wn.end();++itr3){
//        						std::pair<double,double> it3 = itr3->GetCoordinate();
//        						nodes.erase(it3);
//        					}
//        					linestrings.erase(it2);
//        				}
//
//        			}

        			graph = s_graph;
        		}else{
//        			for(auto itr = s_graph.begin();itr != s_graph.end();++itr){
//        				way_node it = itr->second;
//        				std::list<String> st = it.GetWayIDs();
//        				for(auto itr2 = st.begin();itr2 != st.end();++itr2){
//        					String it2 = *itr2;
//        					way_info wi = linestrings.at(it2);
//        					std::list<way_node> wn = wi.GetNodes();
//        					for(auto itr3 = wn.begin();itr3 != wn.end();++itr3){
//        						std::pair<double,double> it3 = itr3->GetCoordinate();
//        						nodes.erase(it3);
//        					}
//        					linestrings.erase(it2);
//        				}
//
//        			}
        		}
        		s_graph.clear();
//        		graph.insert(make_pair(node,wno));
        		cross_nodes++;
        	}
        }

        std::list<String> erase_way;
        for(auto itr = linestrings.begin();itr != linestrings.end();++itr){
        	std::list<way_node> it = itr->second.GetNodes();
        	String id = itr->first;
        	bool flag=false;
        	for(auto itr2 = it.begin();itr2 != it.end();++itr2){
        		std::pair<double,double> it2 = itr2->GetCoordinate();
        		if(nodes.at(it2).GetCrossingflag()){
        			flag=true;
        			decltype(graph)::iterator it3 = graph.find(it2);
        			if(it3 != graph.end()){
        				break;
        			}else{
        				erase_way.push_back(id);
        			}
        		}
        	}
        	if(!flag){
        		erase_way.push_back(id);
        	}
        }

        erase_way.unique();
        for(auto itr = erase_way.begin();itr != erase_way.end();++itr){
        	String id = *itr;
        	// std::cout<<"way_id:"<<id<<std::endl;
        }

        for(auto itr = erase_way.begin();itr != erase_way.end();++itr){
        	String id = *itr;
//        	std::cout<<"way_id:"<<id<<std::endl;
        	std::list<way_node> it = linestrings.at(id).GetNodes();
        	for(auto itr2 = it.begin();itr2 != it.end();++itr2){
        		std::pair<double,double> it2 = itr2->GetCoordinate();
        		nodes.erase(it2);
        	}
        	linestrings.erase(id);
        }
		double time = MyBuilding::GetUpdateMapinfoInterval();
		// std::cout<<"time："<<time<<std::endl;
        UpdateMapinfoTimer(Seconds(time));

//         std::cout<<"features:"<<features.size()<<std::endl;
//         std::cout<<"linestrings:"<<linestrings.size()<<std::endl;
//         std::cout<<"points:"<<points.size()<<std::endl;
//         std::cout<<"nodes:"<<nodes.size()<<std::endl;

// //        std::cout<<"crossing:"<<cross_nodes<<std::endl;
//         std::cout<<"crossing:"<<graph.size()<<std::endl;

        std::cout<<"longitude_max:"<<std::to_string(longitude_max)<<std::endl;
        std::cout<<"longitude_min:"<<std::to_string(longitude_min)<<std::endl;
        std::cout<<"latitude_max:"<<std::to_string(latitude_max)<<std::endl;
        std::cout<<"latitude_min:"<<std::to_string(latitude_min)<<std::endl;

//         std::cout<<"x:"<<json_x(longitude_max)<<std::endl;
//         std::cout<<"y:"<<json_y(latitude_max)<<std::endl;




//        std::cout<<"x:"<<MyBuilding::GetFieldWidth()<<std::endl;
//        std::cout<<"y:"<<MyBuilding::GetFieldHight()<<std::endl;

    }
    else{
    	std::cout << "Not found data" << std::endl;
    }


    const std::string pathToJSON_hinanjo = MyBuilding::GethinanjojsonPath();
    std::ifstream ifs1(pathToJSON_hinanjo.c_str());
    if (ifs1.good()){
    	json js;
    	ifs1 >> js;
    	int i=0;

		/*
		P20_002	避難施設の名称	文字列型
		P20_003	避難施設の住所	文字列型
		P20_004	避難施設の種類	文字列型
		P20_005	収容可能人数	整数値型
		P20_006	施設規模(面積)	文字列型

		P20_007	地震災害	真偽値型
		P20_008	津波災害	真偽値型
		P20_009	水害		真偽値型
		P20_010	火山災害	真偽値型
		P20_011	その他		真偽値型
		P20_012	指定無し	真偽値型
		今回指定無しはすべての災害に対応として扱う
		*/

        while(1){
        	String s = "/features";
        	s += "/" + std::to_string(i);
        	if(js[json::json_pointer(s)].is_null()){
        		break;
        	}
        	double x = js[json::json_pointer(s)][json::json_pointer("/geometry/coordinates/0")];
        	double y = js[json::json_pointer(s)][json::json_pointer("/geometry/coordinates/1")];
        	if(x<longitude_max&&x>longitude_min&&y<latitude_max&&y>latitude_min){
				String a = js[json::json_pointer(s)][json::json_pointer("/properties/P20_004")];

				
				if(a != "福祉避難所"){ //福祉避難所は災害時避難できないので除外
					uint32_t capa = js[json::json_pointer(s)][json::json_pointer("/properties/P20_005")]; //収容人数を取得
					// std::list<bool> Disaster_Category;
					// Disaster_Category.push_back(js[json::json_pointer(s)][json::json_pointer("/properties/P20_007")]);
					// Disaster_Category.push_back(js[json::json_pointer(s)][json::json_pointer("/properties/P20_008")]);
					// Disaster_Category.push_back(js[json::json_pointer(s)][json::json_pointer("/properties/P20_009")]);
					// Disaster_Category.push_back(js[json::json_pointer(s)][json::json_pointer("/properties/P20_010")]);
					// Disaster_Category.push_back(js[json::json_pointer(s)][json::json_pointer("/properties/P20_011")]);
					// Disaster_Category.push_back(js[json::json_pointer(s)][json::json_pointer("/properties/P20_012")]);

					if(MyBuilding::GetUseDisasterTypeFlag()){
						//////////災害の対応状況を取得////////////////
						std::map<uint32_t, uint32_t> Disaster_Category_Map;
						Disaster_Category_Map.insert(std::make_pair(1, js[json::json_pointer(s)][json::json_pointer("/properties/P20_007")])); //地震
						Disaster_Category_Map.insert(std::make_pair(2, js[json::json_pointer(s)][json::json_pointer("/properties/P20_008")])); //津波
						Disaster_Category_Map.insert(std::make_pair(3, js[json::json_pointer(s)][json::json_pointer("/properties/P20_009")])); //水害
						Disaster_Category_Map.insert(std::make_pair(4, js[json::json_pointer(s)][json::json_pointer("/properties/P20_010")])); //火山
						Disaster_Category_Map.insert(std::make_pair(5, js[json::json_pointer(s)][json::json_pointer("/properties/P20_011")])); //その他
						Disaster_Category_Map.insert(std::make_pair(6, js[json::json_pointer(s)][json::json_pointer("/properties/P20_012")])); //指定なし(すべての災害に対応)

						uint32_t disaster_type = MyBuilding::GetDisasterType();
					 	//6がtrueの場合はすべての災害に対応していることとする
						//1 = true, 0 = false　みたいなもの
						if(Disaster_Category_Map.at(disaster_type) == 1 || Disaster_Category_Map.at(6) == 1){
							hinanjo.push_back(std::make_pair(x,y));
							capacity.insert(std::make_pair(hinanjo.back(),capa)); //座標と収容人数のペア
						}
					}else{
						hinanjo.push_back(std::make_pair(x,y));
						capacity.insert(std::make_pair(hinanjo.back(),capa)); //座標と収容人数のペア
					}						
					//MyBuilding::SetExitInfo(std::make_pair(x,y), Disaster_Category); //座標と避難できる災害の種類をmapに保存
				}
            	//std::cout<<js[json::json_pointer(s)][json::json_pointer("/properties/P20_002")]<<":["<<x<<","<<y<<"]"<<std::endl;
				//std::cout<<"収容可能人数："<<js[json::json_pointer(s)][json::json_pointer("/properties/P20_005")]<<std::endl;
        	}
        	i++;
        }
    }
    else{
    	std::cout << "Not found data" << std::endl;
    }

}

void JSON_Data::update_mapinfo(){

	// std::cout<<"update_mapinfo"<<std::endl;
	update_num++;
	if(MyBuilding::GetBlockFlag()){
		MyBuilding::SetBlockedNodes(update_num);
	}

	// std::list<String> erase_nodes = MyBuilding::GetEraseNodes();
	std::list<String> erase_nodes;
	std::list<String> newblocked = MyBuilding::GetBlockedNodes();

	if(MyBuilding::GetBlockFlag() && MyBuilding::GetBlockeraseFlag() && update_num > 1 && now_blocked.size() > 0){
		// MyBuilding::EraseBlockedNodes(update_num);
		String erasenode = now_blocked.back();
		std::cout<<"削除予定ノード："<<erasenode<<std::endl;
		erase_nodes.push_back(erasenode);
		now_blocked.pop_back();//末尾の要素削除
	}



	for(auto itr = newblocked.begin();itr != newblocked.end();++itr){
		String it = *itr;
		std::pair<double,double> coord = GetCoordinatefromID(it);
		way_node node;
		try{
			 node = nodes.at(coord);
		}
		catch(std::out_of_range& oor){
			break;
		}

		if(!node.GetBlockedflag()){
			node.SetBlockedflag(true);
			Update_ClusterView(it, "block");
		}
		// std::cout<<"毎回実行されてる？"<<std::endl;
		now_blocked.push_back(it);
		nodes.erase(coord);
		nodes.insert(std::make_pair(coord,node));
//		nodes.at(it).Print();
	}

	for(auto a : erase_nodes){
		// std::cout<<"実行されてる？"<<std::endl;
		std::pair<double,double> coord = GetCoordinatefromID(a);
		way_node node;
		try{
			 node = nodes.at(coord);
		}
		catch(std::out_of_range& oor){
			std::cout<<"エラー起きてるかもよ？"<<std::endl;
			break;
		}

		if(node.GetBlockedflag()){
			node.SetBlockedflag(false);
			Update_ClusterView(a, "erase");
		}

		nodes.erase(coord);
		nodes.insert(std::make_pair(coord,node));
		std::cout<<"ブロックを解除しました："<< a <<std::endl;

//		nodes.at(it).Print();
	}
	double time = get_rand_range(50,250);
	std::cout<<"次の解除は"<<time<<"秒あとです．"<<std::endl;;

	UpdateMapinfoTimer(Seconds(time));
}

uint32_t JSON_Data::get_rand_range(uint32_t min, uint32_t max){
	static std::mt19937_64 mt32(MyBuilding::GetSeed());
	std::uniform_int_distribution<uint64_t> get_rand_uni_int(min, max);
	return get_rand_uni_int(mt32);
}

void JSON_Data::UpdateMapinfoTimer (Time delay)
{
	updTimer.SetFunction(&JSON_Data::update_mapinfo);
	updTimer.Remove();
	updTimer.Schedule(delay);
}

void JSON_Data::Update_ClusterView(String id, String status){
	Time now_t = Simulator::Now();
	int64_t time = now_t.GetInteger();
	int64_t roughTime = time / 1000000000;
	int64_t splitSecond = time % 1000000000;
	String mode="ON";

	if(RoutingProtocol::GetMode()){
		mode="ON";
	}else{
		mode="OFF";
	}

	String quickhello="QuickHello_ON";
	if(RoutingProtocol::GetUseQuickHelloflag()){
		quickhello="QuickHello_ON";
	}else{
		quickhello="QuickHello_OFF";
	}	

	String eraseblock = "EraseBlock_ON";
	if(RoutingProtocol::GetUseEraseInfoflag() == true){
		eraseblock="EraseBlock_ON";
	}else{
		eraseblock="EraseBlock_OFF";
	}

	std::pair<double,double> coord = GetCoordinatefromID(id);

	char filename[100];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	std::string str = "Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterViewer/ClusterViewer_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
	str.copy(filename,str.size());
	filename[str.size()] = '\0';


	// output to file
	std::ofstream fout;
	fout.open(filename,std::ios::app);
 	String flg="FALSE";
 	if(nodes.at(coord).GetCrossingflag()) flg="TRUE";
	if(status == "block"){
		fout << "NI,"
				<< roughTime << "." << splitSecond << ", "	// time [s]
				<<NodeIDstoi(id)
				<<", "<<simple::JSON_Data::json_x(coord.first)<<", "<<simple::JSON_Data::json_y(coord.second)<<", 0, BLOCKED, 0, 0, " <<flg<< std::endl;
	}else if(status == "erase"){
		fout << "NI,"
				<< roughTime << "." << splitSecond << ", "	// time [s]
				<<NodeIDstoi(id)
				<<", "<<simple::JSON_Data::json_x(coord.first)<<", "<<simple::JSON_Data::json_y(coord.second)<<", 0, "<<flg<<", 0, 0,"<< std::endl;		
	}
//	nodenum++;

	fout.close();
}

void JSON_Data::SetNodeID(int id,std::pair<double,double> coord){
//	uint32_t num = nodenum;
	String ID = "node/" + std::to_string(id);
	nodes.at(coord).SetNodeID(ID);
//	for(auto itr = nodes.rbegin();itr!=nodes.rend();++itr){
//		String id = "node/" + std::to_string(num);
//		itr->second.SetNodeID(id);
	id_to_coordinate.insert(std::make_pair(ID,coord));
//		itr->second.Print();
////		std::cout<<NodeIDstoi(id)<<std::endl;
//		num--;
//	}
}

uint32_t JSON_Data::NodeIDstoi(String ID){
	String separator = "/";
	int separator_length = separator.length();

	auto list = std::vector<String>();

	if (separator_length == 0) {
	  list.push_back(ID);
	} else {
	  auto offset = std::string::size_type(0);
	  while (1) {
	    auto pos = ID.find(separator, offset);
	    if (pos == std::string::npos) {
	      list.push_back(ID.substr(offset));
	      break;
	    }
	    list.push_back(ID.substr(offset, pos - offset));
	    offset = pos + separator_length;
	  }
	}

	for(auto itr = list.begin();itr!=list.end();++itr){
		String it = *itr;
//		std::cout<<it<<std::endl;
	}
	int r = stoi(list[1]);
//	std::cout<<r<<std::endl;

	return r;
}


double JSON_Data::json_x(double lo){
	double x;
	Hubeny h;

	h.hubeny_distance(longitude_min,latitude_min,lo,latitude_min,x);

	return x;
}

double JSON_Data::json_y(double la){
	double y;
	Hubeny h;

	h.hubeny_distance(longitude_min,latitude_max,longitude_min,la,y); //（始点の経度，始点の緯度，終点の経度，終点の緯度）

	return y;
}

double JSON_Data::json_y_size(double la){
	double y;
	Hubeny h;

	h.hubeny_distance(longitude_min,latitude_min,longitude_min,la,y); //（始点の経度，始点の緯度，終点の経度，終点の緯度）

	return y;
}

double JSON_Data::calculate_distance(std::pair<double,double> first,std::pair<double,double> second){
	double dst;
	Hubeny h;

	h.hubeny_distance(first.first,first.second,second.first,second.second,dst);

	return dst;
}


void JSON_Data::json_print(String s){
    // std::cout << j[json::json_pointer(s)]<< std::endl;
}

//void JSON_Data::output_node_coordinates(std::ofstream fout, int nid){
//	int i = nid;
//
//    for(auto itr = nodes.begin();itr != nodes.end();++itr){
//    	json node = *itr;
//
//    	fout << "NI, 0, "<<i<<", "<< (double)json_x(node[json::json_pointer("/0")])<<", "<<(double)json_y(node[json::json_pointer("/1")])<<", 0, READY, 0, 0"<<std::endl;
//
//    	i++;
//    }
//
//}




}
}


