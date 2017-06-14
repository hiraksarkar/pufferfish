#ifndef PUFFER_GRAPH_H
#define PUFFER_GRAPH_H

#include <algorithm>
#include <map>
#include "Util.hpp"
#include "sparsepp/spp.h"
#include <iostream>

namespace pufg{

  enum class EdgeType : uint8_t { PLUS_PLUS=0, PLUS_MINUS=1, MINUS_PLUS=2, MINUS_MINUS=3};

  inline bool fromSign_(EdgeType et) {
    return (et == EdgeType::PLUS_PLUS or et == EdgeType::PLUS_MINUS);
  }
  inline bool toSign_(EdgeType et) {
    return (et == EdgeType::PLUS_PLUS or et == EdgeType::MINUS_PLUS);
  }

  inline EdgeType typeFromBools_(bool sign, bool toSign) {
    if (sign and toSign) {
      return EdgeType::PLUS_PLUS;
    } else if (sign and !toSign) {
      return EdgeType::PLUS_MINUS;
    } else if (!sign and toSign) {
      return EdgeType::MINUS_PLUS;
    } else {
      return EdgeType::MINUS_MINUS;
    }
  }

	struct edgetuple{
		edgetuple(bool fSign,std::string cId, bool tSign):
      contigId(cId) {
      t = typeFromBools_(fSign, tSign);
    }

		edgetuple() {}

    bool baseSign() {
      return (t == EdgeType::PLUS_PLUS or t == EdgeType::PLUS_MINUS);
    }
    bool neighborSign() {
      return (t == EdgeType::PLUS_PLUS or t == EdgeType::MINUS_PLUS);
    }

    EdgeType t;
		std::string contigId;
	};

  inline bool operator==(const edgetuple& e1, const edgetuple& e2) {
    return (e1.t == e2.t and e1.contigId == e2.contigId);
  }

  inline bool operator!=(const edgetuple& e1, const edgetuple& e2) {
    return !(e1 == e2);
  }

	class Node {
    template <typename E>
    constexpr typename std::underlying_type<E>::type to_index(E e) {
      return static_cast<typename std::underlying_type<E>::type>(e);
    }
    int8_t getCountPlus_(std::vector<edgetuple>& elist) {
      int8_t r{0};
      for (auto& et : elist) {
        r += (et.baseSign() ? 1 : 0);
      }
      return r;
    }
    int8_t getCountMinus_(std::vector<edgetuple>& elist) {
      int8_t r{0};
      for (auto& et : elist) {
        r += (!et.baseSign() ? 1 : 0);
      }
      return r;
    }

	public:
    Node() {}
    Node(std::string idIn) {id = idIn ;}

    /*
			Node(std::string idIn) {
				id = idIn ;
				indegp = 0 ;
				outdegp = 0 ;
				indegm = 0 ;
				outdegm = 0 ;
			}
    */

    int8_t getIndegP() {
      return getCountPlus_(in_edges);
    }

    int8_t getOutdegP() {
      return getCountPlus_(out_edges);
    }

    int8_t getIndegM() {
      return getCountMinus_(in_edges);
    }

    int8_t getOutdegM() {
      return getCountMinus_(out_edges);
    }

    /*
      int8_t getIndegP() {return outdegp ;}
      int8_t getOutdegP() {return outdegp ;}
    int8_t getIndegM() {return indegm ;}
    int8_t getOutdegM() {return outdegm ;}
    */

			size_t getRealOutdeg() {
        // outgoing + and incoming - 
        spp::sparse_hash_set<std::string> distinctNeighbors;
        for (auto& e : in_edges) {
          if (!e.baseSign()) {
            std::string name = e.contigId + (e.neighborSign() ? "-" : "+");
            distinctNeighbors.insert(name);
          }
        }
        for (auto& e : out_edges) {
          if (e.baseSign()) {
            std::string name = e.contigId + (e.neighborSign() ? "+" : "-");
            distinctNeighbors.insert(name);
          }
        }
        return distinctNeighbors.size();
      }
			size_t getRealIndeg() {
        // outgoing - and incoming + 
        spp::sparse_hash_set<std::string> distinctNeighbors;
        for (auto& e : in_edges) {
          if (e.baseSign()) {
            std::string name = e.contigId + (e.neighborSign() ? "+" : "-");
            distinctNeighbors.insert(name);
          }
        }
        for (auto& e : out_edges) {
          if (!e.baseSign()) {
            std::string name = e.contigId + (e.neighborSign() ? "-" : "+");
            distinctNeighbors.insert(name);
          }
        }
        return distinctNeighbors.size();
      }

			edgetuple& getOnlyRealIn() {
				if (getIndegP() > 0) {
					for (auto& e : in_edges)
            if (e.baseSign()) {
									return e;
							}
				} else {
					for (auto& e : out_edges)
            if (!e.baseSign()) {
									return e;
							}
				}
        // should not get here
        return in_edges.front();
			}

			edgetuple& getOnlyRealOut() {
				if (getOutdegP() > 0) {
					for (auto& e : out_edges)
						if (e.baseSign()) {
							return e;
						}
				} else { // The real outgoing edge should be an incoming edge to negative if it's not an outgoing edge from positive
					for (auto& e : in_edges)
						if (!e.baseSign()) {
								return e;
						}
				}
        // should not get here
        return out_edges.front();
			}
    
			std::string& getId() {return id;}

			void insertNodeTo(std::string nodeId, bool sign, bool toSign) {
        edgetuple ekey = {sign, nodeId, toSign};
        if (std::find(out_edges.begin(), out_edges.end(), ekey) == out_edges.end()) {
          out_edges.emplace_back(ekey);
        }
			}

    void removeEdgeTo(std::string nodeId) {
      out_edges.erase(std::remove_if(out_edges.begin(),
                                    out_edges.end(),
                                    [&nodeId](edgetuple& etup) -> bool {
                                      return etup.contigId == nodeId;
                                    }
                                    ));
      /*
      for (auto it = out_edges.begin(); it != out_edges.end();) {
        auto& edge = *it;
        if (edge.contigId == nodeId) {
          it = out_edges.erase(it);
        } else {
          it++;
        }
      }
      */
    }

			void insertNodeFrom(std::string nodeId, bool sign, bool fromSign){
        edgetuple ekey = {sign, nodeId, fromSign};
        if (std::find(in_edges.begin(), in_edges.end(), ekey) == in_edges.end()) {
          in_edges.emplace_back(ekey);
        }
			}


    void removeEdgeFrom(std::string nodeId) {
      in_edges.erase(std::remove_if(in_edges.begin(),
                                    in_edges.end(),
                                    [&nodeId](edgetuple& etup) -> bool {
                                      return etup.contigId == nodeId;
                                    }
                                    ));
      /*
      for (auto it=in_edges.begin(); it!=in_edges.end();) {
        auto& edge = *it;
        if (edge.contigId == nodeId) {
          if (edge.baseSign()) {
            indegp--;
          } else {
            indegm--;
          }
          it = in_edges.erase(it);
        } else {
          it++;
        }
      }	
      */
    }

    bool checkExistence(bool bSign, std::string toId, bool toSign){
      edgetuple ekey = {bSign, toId, toSign};
      return (std::find(out_edges.begin(), out_edges.end(), ekey) != out_edges.end());
      /*
      for(auto& i : out_edges){
        if(i.baseSign() == bSign and i.neighborSign() == toSign){
          if(i.contigId == toId)
            return true ;
        }
      }
      return false ;
      */
    }

    std::vector<edgetuple>& getPredecessors() {
      return in_edges;
    }
    std::vector<edgetuple>& getSuccessors() {
      return out_edges;
    }

    /*
    std::vector<edgetuple>& getIn() {return in;}
		 std::vector<edgetuple>& getOut() {return out;}

    void removeEdgeFrom_orig(std::string nodeId) {
			for (std::vector<edgetuple>::iterator it=in.begin(); it!=in.end();) {
        auto& edge = *it;
        if (edge.contigId == nodeId) {
          if (edge.baseSign()) {
            indegp--;
            std::string key = nodeId + (edge.neighborSign()?"+":"-");
            if (distinctRealIn.contains(key)) {
              distinctRealIn[key] -= 1;
              if (distinctRealIn[key] == 0)
                distinctRealIn.erase(key);
            }
          }
          else {
            indegm--;
            std::string key = nodeId + (edge.neighborSign()?"-":"+");
            if (distinctRealOut.contains(key)) {
              distinctRealOut[key] -= 1;
              if (distinctRealOut[key] == 0)
                distinctRealOut.erase(key);
            }
          }
          it = in.erase(it);
        }
        else it++;
      }					

			}
    void insertNodeFrom_orig(std::string nodeId, bool sign, bool fromSign){
			if(sign) {
        indegp++ ;
        std::string key = nodeId + (fromSign?"+":"-");
        if (distinctRealIn.contains(key))
          distinctRealIn[key] += 1;
        else distinctRealIn[key] = 1;
      }
      else {
        indegm++ ;
        std::string key = nodeId + (fromSign?"-":"+");
        if (distinctRealOut.contains(key))
          distinctRealOut[key] += 1;
        else distinctRealOut[key] = 1;
      }
      in.emplace_back(sign, nodeId, fromSign) ;
			}
    void insertNodeTo_orig(std::string nodeId, bool sign, bool toSign){
      if(sign) {
        outdegp++;
        std::string key = nodeId + (toSign?"+":"-");
        if (distinctRealOut.contains(key))
          distinctRealOut[key] += 1;
        else distinctRealOut[key] = 1;
      }
      else {
        outdegm++;
        std::string key = nodeId + (toSign?"-":"+");
        if (distinctRealIn.contains(key))
          distinctRealIn[key] += 1;
        else distinctRealIn[key] = 1;
      }

      out.emplace_back(sign, nodeId, toSign);
			}

    void removeEdgeTo_orig(std::string nodeId) {
			for (std::vector<edgetuple>::iterator it=out.begin(); it!=out.end();) {
        auto& edge = *it;
        if (edge.contigId == nodeId) {
          if (edge.baseSign()) {
            outdegp--;
            std::string key = nodeId + (edge.neighborSign()?"+":"-");
            if (distinctRealOut.contains(key)) {
              distinctRealOut[key] -= 1;
              if (distinctRealOut[key] == 0)
                distinctRealOut.erase(key);
            }
          }
          else {
            outdegm--;
            std::string key = nodeId + (edge.neighborSign()?"-":"+");
            if (distinctRealIn.contains(key)) {
              distinctRealIn[key] -= 1;
              if (distinctRealIn[key] == 0)
                distinctRealIn.erase(key);
            }
          }
          it = out.erase(it);
        }
        else it++;
      }
    }
    */

	private:
    std::string id ;
    // Make the actual node IDs into numbers instead of strings
    //uint64_t id_;
    std::vector<edgetuple> out_edges;
    std::vector<edgetuple> in_edges;
    //std::vector<edgetuple> in;
    //std::vector<edgetuple> out ;
			// We use these two just to count total number of incoming edges or outgoing edges and they literaly have no other usecases!!
			//spp::sparse_hash_map<std::string, short> distinctRealIn;
			//spp::sparse_hash_map<std::string, short> distinctRealOut;

	};

	class Graph{
	private:
		spp::sparse_hash_map<std::string,Node> Vertices ;
    //std::vector<Node> Vertices;
    //std::vector<std::string> NodeNames;
		//std::vector<std::pair<Node,Node> > Edges ;

	public:
		//case where I do
		spp::sparse_hash_map<std::string, Node>& getVertices(){
			return Vertices;
		}

		bool getNode(std::string nodeId){
      //return (Vertices.find(nodeId) == Vertices.end());
			if(Vertices.find(nodeId) == Vertices.end())
				return true;
			else
				return false;
		}

		bool addEdge(std::string fromId, bool fromSign, std::string toId, bool toSign){
			//case 1 where the from node does not exist
			//None of the nodes exists
			if(Vertices.find(fromId) == Vertices.end()){
				Node newNode(fromId) ;
				Vertices[fromId] = newNode ;
			}
			if(Vertices.find(toId) == Vertices.end()){
				Node newNode(toId) ;
				Vertices[toId] = newNode ;
			}
			auto& fromNode = Vertices[fromId] ;
			auto& toNode = Vertices[toId] ;

			if(!fromNode.checkExistence(fromSign,toNode.getId(),toSign)){
				fromNode.insertNodeTo(toNode.getId(),fromSign,toSign) ;
				toNode.insertNodeFrom(fromNode.getId(),toSign,fromSign) ;
				return true ;
			}

			return false ;


			//Edges.emplace_back(fromNode,toNode) ;
		}


		bool removeNode(std::string id) {
			if (Vertices.find(id) == Vertices.end()) return false;
			Node& n = Vertices[id];
      for (auto& in : n.getPredecessors()) {
        for (auto& out : n.getSuccessors()) {
          addEdge(in.contigId, in.neighborSign(), out.contigId, out.neighborSign());
        }
      }
			for (auto& in : n.getPredecessors()) {
				Node& from = Vertices[in.contigId];
				from.removeEdgeTo(id);
			}
			for (auto& out : n.getSuccessors()) {
				Node& to = Vertices[out.contigId];
				to.removeEdgeFrom(id);
			}
      return false;
    }

    /*
		bool removeNode_orig(std::string id) {
			if (Vertices.find(id) == Vertices.end()) return false;
			Node& n = Vertices[id];
			for (auto & in : n.getIn()) {
				for (auto & out : n.getOut()) {
					addEdge(in.contigId, in.neighborSign(), out.contigId, out.neighborSign());					
				}
			}
			for (auto& in : n.getIn()) {
				Node& from = Vertices[in.contigId];
				from.removeEdgeTo(id);
			}
			for (auto& out : n.getOut()) {
				Node& to = Vertices[out.contigId];
				to.removeEdgeFrom(id);
			}
      return false;
	}
    */

		/*
		void buildGraph(std::string gfa_file){
			std::ifstream file(gfa_file);
			while(std::get_line(file, ln)){
				char firstC = ln[0];
				if(firstC != 'L') continue ;

			}
		}*/

	};
}

#endif //end puffer
