#include "GraphDistance.h"
using namespace Structure;

GraphDistance::GraphDistance(Structure::Graph *graph)
{
    this->g = new Structure::Graph(*graph);
	this->isReady = false;
}

void GraphDistance::computeDistances( Vector3 startingPoint, double resolution )
{
	std::vector<Vector3> pnts;
	pnts.push_back(startingPoint);
	computeDistances(pnts, resolution);
}

void GraphDistance::computeDistances( std::vector<Vector3> startingPoints, double resolution )
{
	this->isReady = false;

	clear();

	typedef std::map< int, std::pair<int,double> > CloseMap;
	CloseMap closestStart;
	foreach(Vector3 p, startingPoints) 
		closestStart[closestStart.size()] = std::make_pair(-1,DBL_MAX);

	int globalID = 0;

	// Setup control points adjacency lists
	foreach(Node * node, g->nodes)
	{
		int i = 0;

		nodesMap[node] = std::vector<GraphDistanceNode>();
		samplePoints[node] = std::vector<Vector3>();

		std::vector<Vector3> pointList;
		std::vector< std::vector<Vector3> > discretization = node->discretizedPoints( resolution );
		nodeCount[node] = std::make_pair(discretization.size(), discretization.front().size());

		foreach(std::vector<Vector3> pts, discretization){
			foreach(Vector3 p, pts){
				pointList.push_back(p);
			}
		}

		foreach(Vector3 p, pointList)
		{
			// Check if its close to a start
			for(int s = 0; s < (int)startingPoints.size(); s++)
			{
				Scalar dist = (p - startingPoints[s]).norm();
				if(dist < closestStart[s].second)
					closestStart[s] = std::make_pair(globalID, dist);
			}

			samplePoints[node].push_back(p);
			allPoints.push_back(p);
			adjacency_list.push_back( std::vector<neighbor>() );
			correspond.push_back(node);
			nodesMap[node].push_back( GraphDistanceNode(p, node, i++, globalID++) );
		}
	}

	// Compute neighbors and distances at each node
	foreach(Node * node, g->nodes)
	{			
		int gid = nodesMap[node].front().gid;

		if(node->type() == Structure::CURVE)
		{
			int N = nodesMap[node].size();

			for(int i = 0; i < N; i++)
			{
				std::set<int> adj;
				adj.insert(qRanged(0, i - 1, N - 1));
				adj.insert(qRanged(0, i + 1, N - 1));
				adj.erase(i);

				// Add neighbors
				foreach(int nei, adj){
					double weight = (samplePoints[node][i] - samplePoints[node][nei]).norm();
					adjacency_list[gid + i].push_back(neighbor(gid + nei, weight));
				}
			}
		}

		if(node->type() == Structure::SHEET)
		{
			Sheet * n = (Sheet*)node;

			int numU = nodeCount[n].first, numV = nodeCount[n].second;

			for(int u = 0; u < numU; u++)
			{
				for(int v = 0; v < numV; v++)
				{
					std::set<int> adj;

					int idx = u*numV + v;

					// Figure out the set of neighbors
					for(int i = -1; i <= 1; i++)
					{
						for(int j = -1; j <= 1; j++)
						{
							int ni = qRanged(0, u + i, numU - 1);
							int nj = qRanged(0, v + j, numV - 1);
							adj.insert(ni*numV + nj);
						}
					}
					adj.erase(idx);

					// Add its neighbors
					foreach(int nei, adj){
						double weight = (samplePoints[node][idx] - samplePoints[node][nei]).norm();
						adjacency_list[gid + idx].push_back(neighbor(gid + nei, weight));
					}
				}
			}
		}
	}

	// Connect between nodes
	foreach(Link e, g->edges)
	{
		int gid1 = nodesMap[e.n1].front().gid;
		int gid2 = nodesMap[e.n2].front().gid;

		// Get positions
		Vector3 pos1(0),pos2(0);
		e.n1->get(e.coord[0], pos1);
		e.n2->get(e.coord[1], pos2);

		QMap<double,int> dists1, dists2;

		// Find closest on node 1
		for(int i = 0; i < (int)samplePoints[e.n1].size(); i++) 
			dists1[(pos1-samplePoints[e.n1][i]).norm()] = i;
		int id1 = dists1.values().first();

		// Find closest on node 2
		for(int i = 0; i < (int)samplePoints[e.n2].size(); i++) 
			dists2[(pos2-samplePoints[e.n2][i]).norm()] = i;
		int id2 = dists2.values().first();

		// Connect them
		double weight = (samplePoints[e.n1][id1] - samplePoints[e.n2][id2]).norm();
		adjacency_list[gid1 + id1].push_back(neighbor(gid2 + id2, weight));
		adjacency_list[gid2 + id2].push_back(neighbor(gid1 + id1, weight));

		// Keep record
		jumpPoints.insert(std::make_pair(gid1 + id1, gid2 + id2));
	}

	// Create and connect starting points to rest of graph
	int start_point = globalID;
	adjacency_list.push_back(std::vector<neighbor>());
	allPoints.push_back(Vector3(0));
	correspond.push_back(NULL);

	// Last element's adjacency list contains links to all starting points
	for(CloseMap::iterator it = closestStart.begin(); it != closestStart.end(); it++){
		adjacency_list.back().push_back(neighbor(it->second.first, 0));
	}

	// Compute distances
	DijkstraComputePaths(start_point, adjacency_list, min_distance, previous);

	// Find maximum
	double max_dist = -DBL_MAX;
	for(int i = 0; i < (int)allPoints.size(); i++)
		max_dist = qMax(max_dist, min_distance[i]);

	// Normalize
	for(int i = 0; i < (int)allPoints.size(); i++)
		dists.push_back(min_distance[i] / max_dist);

	isReady = true;
}

double GraphDistance::distanceTo( Vector3 point, std::vector<Vector3> & path )
{
	// Find closest destination point
	int closest = 0;
	double minDist = DBL_MAX;

	for(int i = 0; i < (int)allPoints.size(); i++)
	{
		double dist = (point - allPoints[i]).norm();
		if(dist < minDist){
			minDist = dist;
			closest = i;
		}
	}

	// Retrieve path 
	std::list<vertex_t> shortestPath = DijkstraGetShortestPathTo(closest, previous);
	foreach(vertex_t v, shortestPath) 
		path.push_back( allPoints[v] );

	// Remove first node and reverse order
	path.erase( path.begin() );
	std::reverse(path.begin(), path.end());
	
	// Return distance
	return dists[closest];
}

void GraphDistance::clear()
{
	adjacency_list.clear();
	min_distance.clear();
	previous.clear();
	nodesMap.clear();
	samplePoints.clear();
	nodeCount.clear();
	allPoints.clear();
	dists.clear();
	correspond.clear();
	jumpPoints.clear();
}

void GraphDistance::draw()
{
	if(!isReady) return;

	glDisable(GL_LIGHTING);

	// Draw edges [slow!]
	/*glLineWidth(4);
	glBegin(GL_LINES);
	int v1 = 0;
	foreach(std::vector<neighbor> adj, adjacency_list)
	{
		foreach(neighbor n, adj)
		{
			int v2 = n.target;

			glVector3(allPoints[v1]);
			glVector3(allPoints[v2]);
		}
		v1++;
	}
	glEnd();*/

	// Visualize resulting distances
	glPointSize(15);
	glBegin(GL_POINTS);
	for(int i = 0; i < (int)allPoints.size(); i++)
	{
		glColorQt( qtJetColorMap(dists[i]) );
		glVector3( allPoints[i] );
	}
	glEnd();

	glEnable(GL_LIGHTING);
}

Structure::Node * GraphDistance::closestNeighbourNode( Vector3 to, double resolution )
{
	computeDistances(to, resolution);

	// Get start point appointed
	int startID = adjacency_list.back().back().target;

	Node * closestNode = NULL;
	double minDist = DBL_MAX;
	typedef std::pair<int,int> PairIds;

	// Compare distance at jump points
	foreach(PairIds i, jumpPoints)
	{
		double dist = dists[i.first];

		if(dist < minDist){
			minDist = dist;
			closestNode = correspond[i.first] == correspond[startID] ? correspond[i.second] : correspond[i.first];
		}
	}
	return closestNode;
}