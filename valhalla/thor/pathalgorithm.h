#ifndef VALHALLA_THOR_PATHALGORITHM_H_
#define VALHALLA_THOR_PATHALGORITHM_H_

#include <vector>
#include <map>
#include <unordered_map>
#include <utility>

#include <valhalla/baldr/graphid.h>
#include <valhalla/baldr/graphreader.h>
#include <valhalla/baldr/pathlocation.h>
#include <valhalla/sif/dynamiccost.h>
#include <valhalla/sif/edgelabel.h>
#include <valhalla/sif/hierarchylimits.h>
#include <valhalla/thor/adjacencylist.h>
#include <valhalla/thor/astarheuristic.h>
#include <valhalla/thor/edgestatus.h>
#include <valhalla/thor/pathinfo.h>

namespace valhalla {
namespace thor {

/**
 * Algorithm to create shortest path.
 */
class PathAlgorithm {
 public:
  /**
   * Constructor.
   */
  PathAlgorithm();

  /**
   * Destructor
   */
  virtual ~PathAlgorithm();

  /**
   * Form path between and origin and destination location using the supplied
   * costing method.
   * @param  origin  Origin location
   * @param  dest    Destination location
   * @param  graphreader  Graph reader for accessing routing graph.
   * @param  costing  Costing method.
   * @return  Returns the path edges (and elapsed time/modes at end of
   *          each edge).
   */
  std::vector<PathInfo> GetBestPath(const baldr::PathLocation& origin,
          const baldr::PathLocation& dest, baldr::GraphReader& graphreader,
          const std::shared_ptr<sif::DynamicCost>& costing);

  /**
   * Clear the temporary information generated during path construction.
   */
  void Clear();

 protected:
  // Allow transitions (set from the costing model)
  bool allow_transitions_;

  // Current travel mode
  sif::TravelMode mode_;

  // Hierarchy limits.
  std::vector<sif::HierarchyLimits> hierarchy_limits_;

  // A* heuristic
  AStarHeuristic astarheuristic_;

  // List of edge labels
  uint64_t edgelabel_index_;
  std::vector<sif::EdgeLabel> edgelabels_;

  // Adjacency list
  AdjacencyList* adjacencylist_;

  // Edge status
  EdgeStatus* edgestatus_;

  // Map of edges in the adjacency list. Keep this map so we do not have
  // to search to find an entry that is already in the adjacency list
  std::unordered_map<baldr::GraphId, uint32_t> adjlistedges_;

  // Destinations, id and cost
  std::unordered_map<baldr::GraphId, sif::Cost> destinations_;

  // Destination that was last found with its true cost + partial cost
  std::pair<uint32_t, sif::Cost> best_destination_;

  /**
   * Initializes the hierarch limits, A* heuristic, and adjacency list.
   * @param  origll  Lat,lng of the origin.
   * @param  destll  Lat,lng of the destination.
   * @param  costing Dynamic costing method.
   */
  void Init(const PointLL& origll, const PointLL& destll,
      const std::shared_ptr<sif::DynamicCost>& costing);

  /**
   * Handle transition edges. Will add any that are allowed to the
   * adjacency list.
   * @param level      Current hierarchy level
   * @param edge       Directed edge (a transition edge)
   * @param pred       Predecessor information
   * @param predindex  Predecessor index in the edge labels.
   */
  void HandleTransitionEdge(const uint32_t level,const baldr::GraphId& edgeid,
                      const baldr::DirectedEdge* edge,
                      const sif::EdgeLabel& pred, const uint32_t predindex);

  /**
   * Add edges at the origin to the adjacency list
   */
  void SetOrigin(baldr::GraphReader& graphreader, const baldr::PathLocation& origin,
      const std::shared_ptr<sif::DynamicCost>& costing, const baldr::GraphId& loop_edge);

  /**
   * Set the destination edge(s).
   */
  void SetDestination(baldr::GraphReader& graphreader, const baldr::PathLocation& dest,
     const std::shared_ptr<sif::DynamicCost>& costing);

  /**
   * Return a valid edge id if we've found the destination edge
   * @param edge_label_index     edge label to be tested for destination
   * @return bool                true if we've found the destination
   */
  bool IsComplete(const uint32_t edge_label_index);

  /**
   * Form the path from the adjacency list.
   * @param   dest  Index in the edge labels of the destination edge.
   * @param   graphreader  Graph tile reader
   * @param   loop   GraphId representing the loop edge (invalid if none)
   * @return  Returns the path info, a list of GraphIds representing the
   *          directed edges along the path - ordered from origin to
   *          destination - along with travel modes and elapsed time.
   */
  std::vector<PathInfo> FormPath(const uint32_t dest,
                                 baldr::GraphReader& graphreader,
                                 const baldr::GraphId& loop);

  /**
   * TODO - are we keeping these?
   */
  baldr::GraphId GetStartNode(baldr::GraphReader& graphreader,
                              const baldr::DirectedEdge* directededge);
  std::vector<baldr::GraphId> FormLocalPath(const uint32_t dest,
                              baldr::GraphReader& graphreader);
  baldr::GraphId RecoverShortcut(baldr::GraphReader& graphreader,
                                 const baldr::GraphId& startnode,
                                 const baldr::GraphId& endnode,
                                 const baldr::DirectedEdge* shortcutedge,
                                 std::vector<baldr::GraphId>& edgesonpath);

  /**
   * Gets the edge label for an edge that is in the adjacency list.
   * @param  edgeid   Edge Id
   * @return  Returns the index in the edge labels for the edge id.
   */
  uint32_t GetPriorEdgeLabel(const baldr::GraphId& edgeid) const;

  /*
   * Remove the edge label from the map of edges in the adjacency list
   * @param  edgeid  Edge Id to remove from adjacency list map.
   */
  void RemoveFromAdjMap(const baldr::GraphId& edgeid);
};

}
}

#endif  // VALHALLA_THOR_PATHALGORITHM_H_
