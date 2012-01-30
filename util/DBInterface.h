#ifndef DB_INTERFACE_H

class VOI;

class DBInterface {

 protected:
  DBInterface() {}
  virtual ~DBInterface() {}

 public:

  virtual void updateVOI (VOI *voi) = 0;
};

#endif
