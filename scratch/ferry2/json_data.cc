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
#include <ctime>
#include <cstdlib>

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
std::list<String> JSON_Data::now_rescue;
std::list<String> JSON_Data::now_congestion;
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

// -------------------------------------------------------------------------
// Helper functions required for pre-calculation
// -------------------------------------------------------------------------


// Pos alias is used locally here for readability, but internally everything uses std::pair<double, double>.
using Pos = std::pair<double, double>;
using Weight = double;
const Weight inf = std::numeric_limits<Weight>::max();


void dijkstra_pair(Pos start, const std::map<Pos, way_node>& nodes_map, 
                    std::map<Pos, Weight>& dist, std::map<Pos, Pos>& parent) {
    
    // Initialize distance and predecessor maps
    dist.clear(); parent.clear();
    for (const auto& pair : nodes_map) {
        // Initialize distance to INF and predecessor to a dummy value
        dist[pair.first] = inf; parent[pair.first] = {-1.0, -1.0}; 
    }
    dist[start] = 0.0;
    
    // priority_queue: {Distance, Coordinate}
    using QElement = std::pair<Weight, Pos>;
    std::priority_queue<QElement, std::vector<QElement>, std::greater<QElement>> pq;
    pq.push({0.0, start});

    while (!pq.empty()) {
        Weight d = pq.top().first; Pos u_coord = pq.top().second; pq.pop();

        // Check if the coordinate exists and if a shorter path has already been found
        if (dist.find(u_coord) == dist.end() || d > dist.at(u_coord)) continue;

        // Graph access: The graph is implicitly defined by GetNeighborNodes() within the way_node
        const way_node& u_node = nodes_map.at(u_coord); 
        
        for (const auto& edge : u_node.GetNeighborNodesforUAV()) {
            Pos v_coord = edge.first; Weight weight = edge.second; // Neighbor coordinate and edge weight

            if (dist.find(v_coord) == dist.end()) continue; // Check if neighbor exists in the map

            if (dist.at(u_coord) + weight < dist.at(v_coord)) {
                dist[v_coord] = dist.at(u_coord) + weight;
                parent[v_coord] = u_coord; // Record predecessor coordinate for path reconstruction
                pq.push({dist[v_coord], v_coord});
            }
        }
    }
}

/**
 * @brief Path Reconstruction: Coordinate-based.
 */
std::vector<std::pair<double, double>> reconstruct_path_pair(Pos start, Pos target, const std::map<Pos, Pos>& parent) {
    std::vector<std::pair<double, double>> path;
    Pos current = target;
    Pos dummy_parent = {-1.0, -1.0};

    // Trace back from target to start
    while (current != dummy_parent && current != start) {
        path.push_back(current);
        if (parent.count(current) == 0) return {}; // Check for disconnected graph/error
        current = parent.at(current);
    }
    
    if (current == start) {
        path.push_back(start);
        std::reverse(path.begin(), path.end()); // Reverse to get start -> target order
    } else {
        return {}; // Unreachable or path error
    }
    return path;
}

// -------------------------------------------------------------------------
// 2. Main Pre-calculation Program Body
// -------------------------------------------------------------------------


void JSON_Data::precalculate_shortest_paths() {

	std::srand(1);

	size_t total_nodes = nodes.size();
	size_t crossing_nodes = 0;
	for (const auto& pair : nodes) {
		if (pair.second.GetCrossingflag()) {
			crossing_nodes++;
		}
	}
	std::cout << "DEBUG: Total Nodes in Map: " << total_nodes << std::endl;
	std::cout << "DEBUG: Only Intersection Nodes (Calculated Starts): " << crossing_nodes << std::endl;

	// exit(1);
	
	std::map<String, uint32_t> escape = simple::MyBuilding::GetExitNodes();
	std::list<Pos> exit;
	for(auto a : escape){
		exit.push_back(GetCoordinatefromID(a.first));
	}

	int matched_shelters = 0;
	for (const Pos& s_coord : exit) {
		if (nodes.count(s_coord)) {
			matched_shelters++;
		} else {
			std::cerr << "DEBUG: Shelter coord (" << s_coord.first << ", " << s_coord.second << ") NOT found in nodes map!" << std::endl;
		}
	}
	std::cout << "DEBUG: Total Shelters to process: " << exit.size() << ". Matched in Graph: " << matched_shelters << std::endl;
    
    // Check if nodes or shelters (hinanjo) are defined
    if (nodes.empty() || exit.empty()) { 
        std::cerr << "JSON_Data::precalculate_shortest_paths: Nodes or shelters are not defined. Skipping." << std::endl;
        return;
    }

    std::cout << "--- Starting All-Pairs Shortest Paths Pre-calculation ---" << std::endl;

    // Loop through all nodes as the starting point (u_coord)
    // nodes is a static member (JSON_Data::nodes)
    for (auto& node_pair : nodes) {
        Pos u_coord = node_pair.first;
        way_node& u_node = node_pair.second; // Reference to update the node data

		// if (!u_node.GetCrossingflag()) { //交差点でなければスキップ
        //     continue; 
        // }

        // (1) Calculate shortest distance and predecessors from u_coord (Assuming no initial blockades)
        std::map<Pos, Weight> dist;
        std::map<Pos, Pos> parent;
        dijkstra_pair(u_coord, nodes, dist, parent);

        // (2) Multiple Goals Consideration: Identify the nearest shelter
        //Pos target_shelter_coord = {-1.0, -1.0};
        //Weight min_dist = inf;

        // hinanjo is the list of shelter coordinates
        //for (const Pos& s_coord : exit) { 
            // Check if the shelter is reachable and closer
            //if (dist.count(s_coord) && dist.at(s_coord) < min_dist) {
                //min_dist = dist.at(s_coord);
                //target_shelter_coord = s_coord;
            //}
        //}

		// (2) Multiple Goals Consideration: Choose randomly from nearest 3 shelters
		Pos target_shelter_coord = {-1.0, -1.0};

		std::vector<std::pair<Weight, Pos>> shelter_candidates;

		// hinanjo is the list of shelter coordinates
		for (const Pos& s_coord : exit) {
			if (dist.count(s_coord)) {
				shelter_candidates.push_back(std::make_pair(dist.at(s_coord), s_coord));
			}
		}

		if (!shelter_candidates.empty()) {
			std::sort(shelter_candidates.begin(), shelter_candidates.end(),
				[](const std::pair<Weight, Pos>& a, const std::pair<Weight, Pos>& b) {
					return a.first < b.first;
				});

			int candidate_num = std::min(3, static_cast<int>(shelter_candidates.size()));
			int random_index = std::rand() % candidate_num;

			target_shelter_coord = shelter_candidates[random_index].second;
		}
        
        // (3) Store the shortest path if it exists
        if (target_shelter_coord.first != -1.0) {
            
            // Reconstruct the path
            std::vector<std::pair<double, double>> shortest_path = reconstruct_path_pair(u_coord, target_shelter_coord, parent);
            
            // Save results to the way_node object
            u_node.SetShortestPath(shortest_path);
            u_node.SetNearestShelter(target_shelter_coord);

        } else {
            // If unreachable, set empty/dummy values
            u_node.SetShortestPath({});
            u_node.SetNearestShelter({-1.0, -1.0});
        }
    }

    std::cout << "--- Pre-calculation Finished ---" << std::endl;
	DisplayAllShortestPaths(); //print
}

// -------------------------------------------------------------------------
// 3. Suggested Calling Point
// -------------------------------------------------------------------------
/*
 * This precalculate_shortest_paths function should be called once, after all nodes,
 * edges, and shelter information have been successfully loaded within JSON_Data::json_input().
 * * Example: Add the following line at the end of JSON_Data::json_input():
 * JSON_Data::precalculate_shortest_paths();
 */


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
	const std::string pathToJSON_field = "./obayashiIOFiles/data/" + MyBuilding::GetFieldName() + "/" + MyBuilding::GetFieldName() + ".geojson";
	// const std::string pathToJSON_field = "./obayashiIOFiles/data/itukaichi.geojson";
	std::cout<<pathToJSON_field<<std::endl;
    std::ifstream ifs(pathToJSON_field.c_str());
    if (ifs.good())
    {
        ifs >> j;
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
						wn.SetWayNode(feature[json::json_pointer(s)],wayid,false,false,false,false,false);
						wi.SetNodes(wn);

						nodes.insert(std::make_pair(wn.GetCoordinate(),wn));
					}
					
					i++;
				}
				//避難所を交差点とすることで経路の検索を可能とする．
//				std::cout<<wayid<<"---";
//				std::cout<<"dst(m):"<<dst<<std::endl;
				wi.SetWayLength(dst);
				linestrings.insert(std::make_pair(wayid,wi));
//				linestrings.at(wayid).Print();
        	}
        }

		//隣接ノード距離計算・登録（全ノード）今はドローンのためだけに全ノード計算、将来的にはユーザ移動もこちらで
		for(auto itr = linestrings.begin();itr != linestrings.end();++itr){
			
			std::list<way_node> way_n = itr->second.GetNodes(); // way_info©çm[hXgðæ¾

			if (way_n.empty()) continue;

			way_node p_wn = way_n.front(); // ¼OÌ_ (Previous point)

			for(auto itr_wnl = ++way_n.begin();itr_wnl != way_n.end();++itr_wnl){
				way_node it2 = *itr_wnl; // »ÝÌ_ (Current point)
				double node_dist = calculate_distance(p_wn.GetCoordinate(), it2.GetCoordinate());

				nodes.at(p_wn.GetCoordinate()).SetNeighborNodeforUAV(it2.GetCoordinate(), node_dist);
				nodes.at(it2.GetCoordinate()).SetNeighborNodeforUAV(p_wn.GetCoordinate(), node_dist);

				p_wn = it2;
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

		//2026/07/01
		double shelter_edge_margin_rate = 0.20;
		double shelter_margin_lon = (longitude_max - longitude_min) * shelter_edge_margin_rate;
		double shelter_margin_lat = (latitude_max - latitude_min) * shelter_edge_margin_rate;

		uint32_t shelter_total_in_map = 0;
		uint32_t shelter_removed_edge = 0;
		uint32_t shelter_used = 0;

        while(1){
        	String s = "/features";
        	s += "/" + std::to_string(i);
        	if(js[json::json_pointer(s)].is_null()){
        		break;
        	}
        	double x = js[json::json_pointer(s)][json::json_pointer("/geometry/coordinates/0")];
        	double y = js[json::json_pointer(s)][json::json_pointer("/geometry/coordinates/1")];
        	//if(x<longitude_max&&x>longitude_min&&y<latitude_max&&y>latitude_min){
			//kokokara
			if(x < longitude_max && x > longitude_min &&
				y < latitude_max  && y > latitude_min){

					shelter_total_in_map++;

					if(x >= longitude_max - shelter_margin_lon ||
					x <= longitude_min + shelter_margin_lon ||
					y >= latitude_max - shelter_margin_lat ||
					y <= latitude_min + shelter_margin_lat){

						shelter_removed_edge++;
						i++;
						continue;
					}

					//kokomade
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
							shelter_used++;
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
		std::cout << "DEBUG: Shelters in map = " << shelter_total_in_map << std::endl;
		std::cout << "DEBUG: Shelters removed near edge = " << shelter_removed_edge << std::endl;
		std::cout << "DEBUG: Shelters used = " << shelter_used << std::endl;
    }
    else{
    	std::cout << "Not found data" << std::endl;
    }

	// JSON_Data::precalculate_shortest_paths(); //way_nodeに避難所までの最短経路をもたせる

}

void JSON_Data::update_mapinfo(){

	// std::cout<<"update_mapinfo"<<std::endl;
	update_num++;

	std::list<String> erase_rescue;
	std::list<String> erase_congestion;
	std::list<String> newrescue;
	std::list<String> newcongestion;

	if(MyBuilding::GetBlockFlag()){
		MyBuilding::SetBlockedNodes(update_num);
	}
	if(MyBuilding::GetRescueFlag()){
		MyBuilding::SetRescueNode(update_num);
		newrescue = MyBuilding::GetRescueNode();
	}
	if(MyBuilding::GetCongestionFlag()){
		MyBuilding::SetCongestionNode(update_num);
		newcongestion = MyBuilding::GetCongestionNode();
	}
	if(MyBuilding::GetEraseRescueFlag() && now_rescue.size() > 0 && update_num > 1){
		MyBuilding::EraseRescueNode(update_num);
		erase_rescue = MyBuilding::GetEraseRescueNode();
	}
	if(MyBuilding::GetEraseCongestionFlag() && now_congestion.size() > 0 && update_num > 1){
		MyBuilding::EraseCongestionNode(update_num);
		erase_congestion = MyBuilding::GetEraseCongestionNode();
	}

	std::list<String> erase_nodes;
	std::list<String> newblocked = MyBuilding::GetBlockedNodes();



	// std::list<String> erase_nodes = MyBuilding::GetEraseNodes();
	double time = MyBuilding::GetUpdateMapinfoInterval();
	if(MyBuilding::GetBlockFlag() && MyBuilding::GetRandomBlockeraseFlag() && update_num > 1 && now_blocked.size() > 0){
		// MyBuilding::EraseBlockedNodes(update_num);
		String erasenode = now_blocked.back();
		std::cout<<"削除予定ノード："<<erasenode<<std::endl;
		erase_nodes.push_back(erasenode);
		now_blocked.pop_back();//末尾の要素削除
		time = get_rand_range(50,250);
		std::cout<<"次の解除は"<<time<<"秒あとです．"<<std::endl;
	}

	if(MyBuilding::GetBlockFlag() && MyBuilding::GetBlockeraseFlag() && update_num > 1 && now_blocked.size() > 0){
		MyBuilding::EraseBlockedNodes(update_num);
		erase_nodes = MyBuilding::GetEraseNodes();
		for(auto a : erase_nodes){
			now_blocked.remove(a);
		}
		time = MyBuilding::GetUpdateMapinfoInterval();
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
	}

	for(auto a : newrescue){
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

		if(!node.GetRescueflag()){
			node.SetRescueflag(true);
			Update_ClusterView(a, "set_rescue");
		}

		now_rescue.push_back(a);
		nodes.erase(coord);
		nodes.insert(std::make_pair(coord,node));
		std::cout<<"要救助者ノードを設定しました："<< a <<std::endl;
	}

	for(auto a : newcongestion){
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

		if(!node.GetCongestionflag()){
			node.SetCongestionflag(true);
			Update_ClusterView(a, "set_congestion");
		}

		now_congestion.push_back(a);
		nodes.erase(coord);
		nodes.insert(std::make_pair(coord,node));
		std::cout<<"混雑箇所を設定しました："<< a <<std::endl;
	}

	for(auto a : erase_rescue){
		std::pair<double,double> coord = GetCoordinatefromID(a);
		way_node node;
		try{
			 node = nodes.at(coord);
		}
		catch(std::out_of_range& oor){
			std::cout<<"エラー起きてるかもよ？"<<std::endl;
			break;
		}

		if(node.GetRescueflag()){
			node.SetRescueflag(false);
			Update_ClusterView(a, "erase_rescue");
		}

		now_rescue.remove(a);
		nodes.erase(coord);
		nodes.insert(std::make_pair(coord,node));
		std::cout<<"要救助者を削除しました："<< a <<std::endl;
	}

	for(auto a : erase_congestion){
		std::pair<double,double> coord = GetCoordinatefromID(a);
		way_node node;
		try{
			 node = nodes.at(coord);
		}
		catch(std::out_of_range& oor){
			std::cout<<"エラー起きてるかもよ？"<<std::endl;
			break;
		}		

		if(node.GetCongestionflag()){
			node.SetCongestionflag(false);
			Update_ClusterView(a, "erase_congestion");
		}

		now_congestion.remove(a);
		nodes.erase(coord);
		nodes.insert(std::make_pair(coord,node));
		std::cout<<"混雑箇所を解除："<< a <<std::endl;
	}

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

	char filename[1000];
	std::ostringstream oss_run;
	oss_run << RngSeedManager::GetRun();
	std::string str = "obayashiIOFiles/Log/obayashi/" + mode + "/" + quickhello + "/" + eraseblock + "/" + std::to_string(simple::MyBuilding::GetNumUsers()) + "/ClusterViewer/ClusterViewer_" +std::to_string(simple::MyBuilding::GetRun()) + ".txt";
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
				<<", "<<simple::JSON_Data::json_x(coord.first)<<", "<<simple::JSON_Data::json_y(coord.second)<<", 0, BLOCKED, 0, 0, " << flg << std::endl;
	}else if(status == "erase"){
		fout << "NI,"
				<< roughTime << "." << splitSecond << ", "	// time [s]
				<<NodeIDstoi(id)
				<<", "<<simple::JSON_Data::json_x(coord.first)<<", "<<simple::JSON_Data::json_y(coord.second)<<", 0, "<<flg<<", 0, 0,"<< std::endl;		
	}else if(status == "set_rescue"){
		fout << "NI,"
				<< roughTime << "." << splitSecond << ", "	// time [s]
				<<NodeIDstoi(id)
				<<", "<<simple::JSON_Data::json_x(coord.first)<<", "<<simple::JSON_Data::json_y(coord.second)<<", 0, RESCUE, 0, 0, "<< flg << std::endl;			
	}else if(status == "set_congestion"){
		fout << "NI,"
				<< roughTime << "." << splitSecond << ", "	// time [s]
				<<NodeIDstoi(id)
				<<", "<<simple::JSON_Data::json_x(coord.first)<<", "<<simple::JSON_Data::json_y(coord.second)<<", 0, CONGESTION, 0, 0, "<< flg << std::endl;	
	}else if(status == "erase_congestion"){
		fout << "NI,"
				<< roughTime << "." << splitSecond << ", "	// time [s]
				<<NodeIDstoi(id)
				<<", "<<simple::JSON_Data::json_x(coord.first)<<", "<<simple::JSON_Data::json_y(coord.second)<<", 0, "<<flg<<", 0, 0,"<< std::endl;			
	}else if(status == "erase_rescue"){
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

void JSON_Data::DisplayAllShortestPaths() {
    
    if (nodes.empty()) {
        std::cout << "--- Error: Node data (JSON_Data::nodes) is empty. Cannot display paths. ---" << std::endl;
        return;
    }

    std::cout << "\n=======================================================" << std::endl;
    std::cout << "         PRE-CALCULATED SHORTEST PATHS DISPLAY"           << std::endl;
    std::cout << "=======================================================" << std::endl;

    int path_count = 0;
    
    // Iterate over all way_nodes stored in the static map JSON_Data::nodes
    for (const auto& node_pair : nodes) {
        
        // Get the starting coordinate and the way_node object
        const Pos& start_coord = node_pair.first;
        const way_node& u_node = node_pair.second;
        
        // Retrieve the pre-calculated path data
        const std::vector<std::pair<double, double>>& path = u_node.GetShortestPath();
        const Pos& shelter_coord = u_node.GetNearestShelter();

        if (!path.empty()) {
            path_count++;
            
            std::cout << "\n--- Path #" << path_count << " ---" << std::endl;
            
            // Output Start/End Information
            std::cout << std::fixed << std::setprecision(7);
            std::cout << "  START: (" << start_coord.first << ", " << start_coord.second << ")" << std::endl;
            std::cout << "  SHELTER: (" << shelter_coord.first << ", " << shelter_coord.second << ")" << std::endl;
            std::cout << "  LENGTH (Nodes): " << path.size() << std::endl;
            
            // Output Path Sequence
            std::cout << "  ROUTE: [";
            for (size_t i = 0; i < path.size(); ++i) {
                if (i > 0) std::cout << " -> ";
                
                // Use a short, formatted output for coordinates
                std::cout << "(" << path[i].first << ", " << path[i].second << ")";
            }
            std::cout << "]" << std::endl;
        }
    }

    if (path_count > 0) {
         std::cout << "\n--- Total " << path_count << " shortest paths displayed. ---" << std::endl;
    } else {
         std::cout << "\n--- No reachable shortest paths were found and stored. ---" << std::endl;
    }
}


}
}


