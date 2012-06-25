#ifndef _EXAMPLE_H
#define _EXAMPLE_H
#include "param.h"
#include "model.h"
#include "base_example.h"

// $Id: example.h,v 1.11 2001/07/14 15:28:15 taku-ku Exp $;

namespace TinySVM {

class Example: public BaseExample
{
public:
  int read  (const char *, const char *mode = "r", const int offset = 0);
  int write (const char *, const char *mode = "w", const int offset = 0);
  int rebuildSVindex (Model *);

  Model *learn(const Param &);
  Model *learn(const Param *p) { return this->learn(*p); };
  Model *learn(const char *s) 
  {
    Param p;
    if (p.set(s)) return this->learn(p);
    else return NULL;
  }    
};

};
#endif

