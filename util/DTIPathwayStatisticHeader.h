#ifndef PATHWAY_STAT_HEADER_H
#define PATHWAY_STAT_HEADER_H

// number of stats
// highest unique ID

// * statheaders*
//
// then:
// 
// number of paths
//
// * paths *

struct DTIPathwayStatisticHeader {
 public:
  int _is_luminance_encoding; /* is this statistic good as a luminance encoding for paths?*/
  int _is_computed_per_point; /* is this statistic stored per point, also? */
  int _is_viewable_stat;      /* is this statistic something that shows up 
				  in the stat panel, by default? */
  char _aggregate_name[255];
  char _local_name[255];
  int _unique_id;
};

#endif
