//
// Created by MarvelLi on 2024/3/12.
//

#pragma once
#include <stack>
#include <queue>
#include "CoreMinimal.h"

// Algorithms and classes for graph theory related
namespace MechEngine::Algorithm::GraphTheory
{
	template <typename T>
	struct DirectionalEdge
	{
		uint StartNodeID;
		uint EndNodeID;
		T EdgeContent;
	};

	template <typename T0, typename T1>
	struct Node
	{
		uint ID;
		T0 NodeContent;

		TSet<DirectionalEdge<T1>*> InEdge; // Save the edge that points to this node
		TSet<DirectionalEdge<T1>*> OutEdge;// Save the edge that starts from this node
	};


	// A graph contains a set of nodes and edges, nodes are stored by uquie ID
	template <class NodeContentType = int, class EdgeContentType = int>
	class Graph
	{
		using NodeType = Node<NodeContentType, EdgeContentType>;
		using EdgeType = DirectionalEdge<EdgeContentType>;
	public:

		Graph() = default;

		// Copy constructors, deep copy the graph
		Graph(const Graph&);

		~Graph();

		FORCEINLINE bool IsNodeValid(uint NodeID) const;

		FORCEINLINE int NodeNum() const;
		/**
		 * Get the node by ID
		 * @param NodeID Unique ID of the node
		 * @return Reference to the node
		 */
		FORCEINLINE NodeType& GetNode(uint NodeID);

		/**
		 * Add a node to the graph
		 * @param NodeID Unique ID of the node
		 * @param Content Optional content of the node
		 * @return true if the node is added successfully, false if the node already exists
		 */
		FORCEINLINE bool AddNode(uint NodeID, NodeContentType Content = NodeContentType());

		/**
		 * Remove a node from the graph
		 * @param NodeID
		 * @return true if the node is removed successfully, false if the node does not exist
		 */
		bool RemoveNode(uint NodeID);

		/**
		 * Add a directional edge to the graph, will create the nodes if they do not exist
		 * @param StartNodeID Start node ID
		 * @param EndNodeID End node ID
		 * @param Weight Optional weight of the edge
		 */
		void AddDirectionalEdge(uint StartNodeID, uint EndNodeID, EdgeContentType Content = EdgeContentType());
		void AddBidirectionalEdge(uint StartNodeID, uint EndNodeID, EdgeContentType Content = EdgeContentType());

		/**
		 * Remove an edge from the graph
		 * @param Edge Edge to remove
		 */
		void RemoveEdge(EdgeType* Edge);

		/**
		 * BFS traversal of the graph
		 * @param StartNodeID Start node ID
		 * @param Visit Function to call when a node is visited
		 */
		void BFS(uint StartNodeID, const TFunction<void(uint)>& Visit);

		/**
		 * DFS traversal of the graph
		 * @param StartNodeID Start node ID
		 * @param Visit Function to call when a node is visited
		 */
		void DFS(uint StartNodeID, const TFunction<void(uint)>& Visit);

		/**
		 * Calculate the connected components of the graph, each component is a list of node IDs
		 * Assume the graph is undirected(AKA. all edges are bidirectional)
		 * O(V + E)
		 * @return List of connected components
		 */
		TArray<TArray<uint>> ConnectedComponents();


		/**
		 * @see https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
		 * Calc the strongly connected components of the graph
		 * Assume the graph is directed(unidirectional graph could also work)
		 * @return List of strongly connected components
		 * O(V + E)
		 */
		TArray<TArray<uint>> StronglyConnectedComponents();

		/**
		 * @see https://codeforces.com/blog/entry/71146
		 * Calc the Cut vertex(articulation points) of the graph
		 * Assume the graph is undirected
		 * @return List of Cut vertex
		 * O(V + E)
		 */
		TArray<uint> CutVertex();

	protected:
		// Id to Node
		THashMap<uint, NodeType> Nodes;

		void DFSInternal(uint NodeID, const TFunction<void(uint)>& Visit, THashSet<uint>& VisitFlag);

		void StronglyConnectedComponentsInternal(uint NodeID,
		 THashMap<uint, int>& DFN, THashMap<uint, int>& Low, THashSet<uint>& InStack,
		 std::stack<uint>& Stack, int& Time, TArray<TArray<uint>>& Result);

		void CutVertexInternal(uint NodeID, uint              RootID,
			THashMap<uint, int>&    DFN, THashMap<uint, int>& Low, int& Time, TSet<uint>& Result);
	};

	template <class NodeContentType, class EdgeContentType>
	Graph<NodeContentType, EdgeContentType>::Graph(const Graph& Other)
	{
		for (auto& Node : Other.Nodes)
		{
			AddNode(Node.first, Node.second.NodeContent);
		}

		for (auto& Node : Other.Nodes)
		{
			for (auto& Edge : Node.second.OutEdge)
			{
				AddDirectionalEdge(Edge->StartNodeID, Edge->EndNodeID, Edge->EdgeContent);
			}
		}
	}

	template <class NodeContentType, class EdgeContentType>
	Graph<NodeContentType, EdgeContentType>::~Graph()
	{
		auto NodesCopy = Nodes;
		for (auto& Node : NodesCopy)
		{
			RemoveNode(Node.first);
		}
	}

	template <class NodeContentType, class EdgeContentType>
	FORCEINLINE bool Graph<NodeContentType, EdgeContentType>::IsNodeValid(uint NodeID) const
	{
		return Nodes.contains(NodeID);
	}

	template <class NodeContentType, class EdgeContentType>
	int Graph<NodeContentType, EdgeContentType>::NodeNum() const
	{
		return Nodes.size();
	}

	template <class NodeContentType, class EdgeContentType>
	FORCEINLINE typename Graph<NodeContentType, EdgeContentType>::NodeType& Graph<NodeContentType, EdgeContentType>::GetNode(uint NodeID)
	{
		return Nodes[NodeID];
	}

	template <class NodeContentType, class EdgeContentType>
	bool Graph<NodeContentType, EdgeContentType>::AddNode(uint NodeID, NodeContentType Content)
	{
		if (IsNodeValid(NodeID)) return false;
		Nodes[NodeID] = NodeType{ NodeID, Content };
		return true;
	}

	template <class NodeContentType, class EdgeContentType>
	bool Graph<NodeContentType, EdgeContentType>::RemoveNode(uint NodeID)
	{
		if( Nodes.contains(NodeID) )
		{
			auto OutEdgeCopy = Nodes[NodeID].OutEdge;
			for (auto& Edge : OutEdgeCopy)
			{
				RemoveEdge(Edge);
			}
			auto InEdgeCopy = Nodes[NodeID].InEdge;
			for (auto& Edge : InEdgeCopy)
			{
				RemoveEdge(Edge);
			}
			Nodes.erase(NodeID);
			return true;
		}
		return false;
	}

	template <class NodeContentType, class EdgeContentType>
	void Graph<NodeContentType, EdgeContentType>::AddDirectionalEdge(uint StartNodeID, uint EndNodeID, EdgeContentType Content)
	{
		if(!IsNodeValid(StartNodeID))
		{
			AddNode(StartNodeID);
		}
		if(!IsNodeValid(EndNodeID))
		{
			AddNode(EndNodeID);
		}

		auto* NewEdge = new EdgeType{ StartNodeID, EndNodeID, Content };
		Nodes[StartNodeID].OutEdge.insert(NewEdge);
		Nodes[EndNodeID].InEdge.insert(NewEdge);
	}

	template <class NodeContentType, class EdgeContentType>
	void Graph<NodeContentType, EdgeContentType>::AddBidirectionalEdge(uint StartNodeID, uint EndNodeID, EdgeContentType Content)
	{
		AddDirectionalEdge(StartNodeID, EndNodeID, Content);
		AddDirectionalEdge(EndNodeID, StartNodeID, Content);
	}

	template <class NodeContentType, class EdgeContentType>
	void Graph<NodeContentType, EdgeContentType>::RemoveEdge(EdgeType* Edge)
	{
		if( IsNodeValid(Edge->StartNodeID) )
		{
			Nodes[Edge->StartNodeID].OutEdge.erase(Edge);
		}
		if (IsNodeValid(Edge->EndNodeID))
		{
			Nodes[Edge->EndNodeID].InEdge.erase(Edge);
		}
		delete Edge;
	}

	template <class NodeContentType, class EdgeContentType>
	void Graph<NodeContentType, EdgeContentType>::BFS(uint StartNodeID, const TFunction<void(uint)>& Visit)
	{
		THashSet<uint>       VisitedNode;
		std::queue<uint> Queue;
		Queue.push(StartNodeID);
		VisitedNode.insert(StartNodeID);
		while (!Queue.empty())
		{
			uint CurrentNodeID = Queue.front();
			Visit(CurrentNodeID);
			for (auto& Edge : Nodes[CurrentNodeID].OutEdge)
			{
				auto NextID = Edge->EndNodeID;
				if (VisitedNode.contains(NextID)) continue;
				Queue.push(NextID);
				VisitedNode.insert(NextID);
			}
			Queue.pop();
		}
	}

	template <class NodeContentType, class EdgeContentType>
	void Graph<NodeContentType, EdgeContentType>::DFS(uint StartNodeID, const TFunction<void(uint)>& Visit)
	{
		THashSet<uint> Visited;
		DFSInternal(StartNodeID, Visit, Visited);
	}

	template <class NodeContentType, class EdgeContentType>
	TArray<TArray<uint>> Graph<NodeContentType, EdgeContentType>::ConnectedComponents()
	{
		uint ComponentID = 0;
		THashSet<uint> Visited;
		TArray<TArray<uint>> Result;

		for (auto& AllNode : Nodes)
		{
			auto TNodeID = AllNode.first;
			if (Visited.contains(TNodeID)) continue;
			Result.emplace_back();
			BFS(TNodeID, [&](uint NodeID)
			{
				Result[ComponentID].push_back(NodeID);
				Visited.insert(NodeID);
			});
			ComponentID++;
		}
		return Result;
	}

	template <class NodeContentType, class EdgeContentType>
	TArray<TArray<uint>> Graph<NodeContentType, EdgeContentType>::StronglyConnectedComponents()
	{
		THashMap<uint, int> DFN, Low;
		THashSet<uint>      InStack;
		std::stack<uint>    Stack;
		int                 Time = 0;
		TArray<TArray<uint>> Result;
		for (auto& Node : Nodes)
		{
			auto NodeID = Node.first;
			if (DFN.contains(NodeID)) continue;
			StronglyConnectedComponentsInternal(NodeID, DFN, Low, InStack, Stack, Time, Result);
		}
		return Result;
	}

	template <class NodeContentType, class EdgeContentType>
	TArray<uint> Graph<NodeContentType, EdgeContentType>::CutVertex()
	{
		THashMap<uint, int> DFN, Low;
		THashSet<uint>      InStack;
		std::stack<uint>    Stack;
		int                 Time = 0;
		TSet<uint>			ResultSet;
		TArray<uint>		Result;
		for (auto& Node : Nodes)
		{
			auto NodeID = Node.first;
			if (DFN.contains(NodeID)) continue;
			CutVertexInternal(NodeID, NodeID, DFN, Low, Time, ResultSet);
		}
		std::ranges::copy(ResultSet, std::back_inserter(Result));
		return Result;
	}

	template <class NodeContentType, class EdgeContentType>
	void Graph<NodeContentType, EdgeContentType>::DFSInternal(uint NodeID, const TFunction<void(uint)>& Visit, THashSet<uint>& VisitFlag)
	{
		Visit(NodeID);
		VisitFlag.insert(NodeID);
		for (auto& Edge : Nodes[NodeID].OutEdge)
		{
			auto NextID = Edge->EndNodeID;
			if (VisitFlag.contains(NextID)) continue;
			DFSInternal(NextID, Visit, VisitFlag);
		}
	}

	template <class NodeContentType, class EdgeContentType>
	void Graph<NodeContentType, EdgeContentType>::StronglyConnectedComponentsInternal(uint NodeID, THashMap<uint, int>& DFN, THashMap<uint, int>& Low, THashSet<uint>& InStack, std::stack<uint>& Stack, int& Time, TArray<TArray<uint>>& Result)
	{
		DFN[NodeID] = Low[NodeID] = ++Time;
		Stack.push(NodeID);
		InStack.insert(NodeID);
		for (auto& Edge : Nodes[NodeID].OutEdge)
		{
			auto NextID = Edge->EndNodeID;
			if (!DFN.contains(NextID))
			{
				StronglyConnectedComponentsInternal(NextID, DFN, Low, InStack, Stack, Time, Result);
				Low[NodeID] = std::min(Low[NodeID], Low[NextID]);
			}
			else if (InStack.contains(NextID))
			{
				Low[NodeID] = std::min(Low[NodeID], DFN[NextID]);
			}
		}
		if (DFN[NodeID] == Low[NodeID])
		{
			Result.emplace_back();
			uint CurrentID;
			do
			{
				CurrentID = Stack.top();
				Stack.pop();
				InStack.erase(CurrentID);
				Result.back().push_back(CurrentID);
			} while (CurrentID != NodeID);
		}
	}

	template <class NodeContentType, class EdgeContentType>
	void Graph<NodeContentType, EdgeContentType>::CutVertexInternal(uint NodeID, uint RootID, THashMap<uint, int>& DFN, THashMap<uint, int>& Low, int& Time, TSet<uint>& Result)
	{
		DFN[NodeID] = Low[NodeID] = ++Time;
		int SonCount = 0;
		for (auto& Edge : Nodes[NodeID].OutEdge)
		{
			auto NextID = Edge->EndNodeID;
			if (!DFN.contains(NextID))
			{
				SonCount++;
				CutVertexInternal(NextID, RootID, DFN, Low, Time, Result);
				Low[NodeID] = std::min(Low[NodeID], Low[NextID]);
				if (NodeID != RootID && Low[NextID] >= DFN[NodeID])
					Result.insert(NodeID);
			}
			else
				Low[NodeID] = std::min(Low[NodeID], DFN[NextID]);
		}
		if (NodeID == RootID && SonCount > 1)
		{
			Result.insert(NodeID);
		}
	}

}