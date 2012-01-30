#ifndef PATHWAY_ALGORITHM_HEADER_H
#define PATHWAY_ALGORITHM_HEADER_H

struct DTIPathwayAlgorithmHeader {
 public:
  char _algorithm_name[255];
  char _comments[255];
  int _unique_id;
};

#endif
