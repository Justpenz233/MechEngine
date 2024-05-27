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
		int StartNodeID;
		int EndNodeID;
		T EdgeContent;
	};

	template <typename T0, typename T1>
	struct Node
	{
		int ID;
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

		FORCEINLINE bool IsNodeValid(int NodeID) const;

		FORCEINLINE int NodeNum() const;
		/**
		 * Get the node by ID
		 * @param NodeID Unique ID of the node
		 * @return Reference to the node
		 */
		FORCEINLINE NodeType& GetNode(int NodeID);

		/**
		 * Add a node to the graph
		 * @param NodeID Unique ID of the node
		 * @param Content Optional content of the node
		 * @return true if the node is added successfully, false if the node already exists
		 */
		FORCEINLINE bool AddNode(int NodeID, NodeContentType Content = NodeContentType());

		/**
		 * Remove a node from the graph
		 * @param NodeID
		 * @return true if the node is removed successfully, false if the node does not exist
		 */
		bool RemoveNode(int NodeID);

		/**
		 * Add a directional edge to the graph, will create the nodes if they do not exist
		 * @param StartNodeID Start node ID
		 * @param EndNodeID End node ID
		 * @param Content Optional content of the edge
		 */
		void AddDirectionalEdge(int StartNodeID, int EndNodeID, EdgeContentType Content = EdgeContentType());
		void AddBidirectionalEdge(int StartNodeID, int EndNodeID, EdgeContentType Content = EdgeContentType());

		/**
		 * Remove an edge from the graph
		 * @param Edge Edge to remove
		 */
		void RemoveEdge(EdgeType* Edge);

		/**
		 * BFS traversal of the graph
		 * @param StartNodeID Start node ID
		 * @param Visit Function to call when a node is visited, if the function return false, the traversal will not continue from this node
		 */
		void BFS(int StartNodeID, const TFunction<bool(int)>& Visit) const;

		/**
		 * DFS traversal of the graph
		 * @param StartNodeID Start node ID
		 * @param Visit Function to call when a node is visited
		 */
		void DFS(int StartNodeID, const TFunction<bool(int)>& Visit) const;

		/**
		 * Calculate the connected components of the graph, each component is a list of node IDs
		 * Assume the graph is undirected(AKA. all edges are bidirectional)
		 * O(V + E)
		 * @return List of connected components
		 */
		TArray<TArray<int>> ConnectedComponents();


		/**
		 * @see https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm
		 * Calc the strongly connected components of the graph
		 * Assume the graph is directed(unidirectional graph could also work)
		 * @return List of strongly connected components
		 * O(V + E)
		 */
		TArray<TArray<int>> StronglyConnectedComponents();

		/**
		 * @see https://codeforces.com/blog/entry/71146
		 * Calc the Cut vertex(articulation points) of the graph
		 * Assume the graph is undirected
		 * @return List of Cut vertex
		 * O(V + E)
		 */
		TArray<int> CutVertex();

	protected:
		// Id to Node
		THashMap<int, NodeType> Nodes;

		void DFSInternal(int NodeID, const TFunction<bool(int)>& Visit, THashSet<int>& VisitFlag);

		void StronglyConnectedComponentsInternal(int NodeID,
		 THashMap<int, int>& DFN, THashMap<int, int>& Low, THashSet<int>& InStack,
		 std::stack<int>& Stack, int& Time, TArray<TArray<int>>& Result);

		void CutVertexInternal(int NodeID, int              RootID,
			THashMap<int, int>&    DFN, THashMap<int, int>& Low, int& Time, TSet<int>& Result);
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
	FORCEINLINE bool Graph<NodeContentType, EdgeContentType>::IsNodeValid(int NodeID) const
	{
		return Nodes.contains(NodeID);
	}

	template <class NodeContentType, class EdgeContentType>
	int Graph<NodeContentType, EdgeContentType>::NodeNum() const
	{
		return Nodes.size();
	}

	template <class NodeContentType, class EdgeContentType>
	FORCEINLINE typename Graph<NodeContentType, EdgeContentType>::NodeType& Graph<NodeContentType, EdgeContentType>::GetNode(int NodeID)
	{
		return Nodes[NodeID];
	}

	template <class NodeContentType, class EdgeContentType>
	bool Graph<NodeContentType, EdgeContentType>::AddNode(int NodeID, NodeContentType Content)
	{
		if (IsNodeValid(NodeID)) return false;
		Nodes[NodeID] = NodeType{ NodeID, Content };
		return true;
	}

	template <class NodeContentType, class EdgeContentType>
	bool Graph<NodeContentType, EdgeContentType>::RemoveNode(int NodeID)
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
	void Graph<NodeContentType, EdgeContentType>::AddDirectionalEdge(int StartNodeID, int EndNodeID, EdgeContentType Content)
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
	void Graph<NodeContentType, EdgeContentType>::AddBidirectionalEdge(int StartNodeID, int EndNodeID, EdgeContentType Content)
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
	void Graph<NodeContentType, EdgeContentType>::BFS(int StartNodeID, const TFunction<bool(int)>& Visit) const
	{
		THashSet<int>   VisitedNode;
		std::queue<int> Queue;
		Queue.push(StartNodeID);
		VisitedNode.insert(StartNodeID);
		while (!Queue.empty())
		{
			int CurrentNodeID = Queue.front(); Queue.pop();
			if(!Visit(CurrentNodeID)) continue;
			for (auto& Edge : Nodes.at(CurrentNodeID).OutEdge)
			{
				auto NextID = Edge->EndNodeID;
				if (VisitedNode.contains(NextID)) continue;
				Queue.push(NextID);
				VisitedNode.insert(NextID);
			}
		}
	}

	template <class NodeContentType, class EdgeContentType>
	void Graph<NodeContentType, EdgeContentType>::DFS(int StartNodeID, const TFunction<bool(int)>& Visit) const
	{
		THashSet<int> Visited;
		DFSInternal(StartNodeID, Visit, Visited);
	}

	template <class NodeContentType, class EdgeContentType>
	TArray<TArray<int>> Graph<NodeContentType, EdgeContentType>::ConnectedComponents()
	{
		int ComponentID = 0;
		THashSet<int> Visited;
		TArray<TArray<int>> Result;

		for (auto& AllNode : Nodes)
		{
			auto TNodeID = AllNode.first;
			if (Visited.contains(TNodeID)) continue;
			Result.emplace_back();
			BFS(TNodeID, [&](int NodeID)
			{
				Result[ComponentID].push_back(NodeID);
				Visited.insert(NodeID);
			});
			ComponentID++;
		}
		return Result;
	}

	template <class NodeContentType, class EdgeContentType>
	TArray<TArray<int>> Graph<NodeContentType, EdgeContentType>::StronglyConnectedComponents()
	{
		THashMap<int, int> DFN, Low;
		THashSet<int>      InStack;
		std::stack<int>    Stack;
		int                 Time = 0;
		TArray<TArray<int>> Result;
		for (auto& Node : Nodes)
		{
			auto NodeID = Node.first;
			if (DFN.contains(NodeID)) continue;
			StronglyConnectedComponentsInternal(NodeID, DFN, Low, InStack, Stack, Time, Result);
		}
		return Result;
	}

	template <class NodeContentType, class EdgeContentType>
	TArray<int> Graph<NodeContentType, EdgeContentType>::CutVertex()
	{
		THashMap<int, int> DFN, Low;
		THashSet<int>      InStack;
		std::stack<int>    Stack;
		int                 Time = 0;
		TSet<int>			ResultSet;
		TArray<int>		Result;
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
	void Graph<NodeContentType, EdgeContentType>::DFSInternal(int NodeID, const TFunction<bool(int)>& Visit, THashSet<int>& VisitFlag)
	{
		VisitFlag.insert(NodeID);
		if(!Visit(NodeID)) return;
		for (auto& Edge : Nodes[NodeID].OutEdge)
		{
			auto NextID = Edge->EndNodeID;
			if (VisitFlag.contains(NextID)) continue;
			DFSInternal(NextID, Visit, VisitFlag);
		}
	}

	template <class NodeContentType, class EdgeContentType>
	void Graph<NodeContentType, EdgeContentType>::StronglyConnectedComponentsInternal(int NodeID, THashMap<int, int>& DFN, THashMap<int, int>& Low, THashSet<int>& InStack, std::stack<int>& Stack, int& Time, TArray<TArray<int>>& Result)
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
			int CurrentID;
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
	void Graph<NodeContentType, EdgeContentType>::CutVertexInternal(int NodeID, int RootID, THashMap<int, int>& DFN, THashMap<int, int>& Low, int& Time, TSet<int>& Result)
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