#ifndef _BASE_EXAMPLE_H
#define _BASE_EXAMPLE_H
#include "misc.h"
#include <stdio.h>

// $Id: base_example.h,v 1.19 2001/08/29 14:49:04 taku-ku Exp $;

namespace TinySVM {
   
extern feature_node *str2feature_node   (const char *);
extern feature_node *copy_feature_node  (const feature_node *);
extern feature_node *fix_feature_node   (feature_node *);

class BaseExample
{
private:
  char *stre;
  int   strl;
  int   alloc_l;

public:
  int          l;	       // number of SVs
  int          d;              // dimension of SVs
  int          pack_d;         // packed dimension of SVs
  double       *y;	       // class label,  
  feature_node **x;	       // training data
  int          feature_type;   // feature type (bin or double)
  int          class_type;     // class type (bin or double)
  double       *alpha;         // alpha
  double       *G;             // gradient
  int          svindex_size;   // number of Support Vectors;

  int add (const double, feature_node *);
  int add (const double, const char *);
  int add (const char *);
  int set (int i, const double, feature_node *);
  int set (int i, const double, const char *);
  int set (int i, const char *);
  int get (int i, double &, feature_node *&);
  const char *get (int i);
  int remove (int i);
   
  int clear ();
  int size () { return l; };

  virtual int read   (const char *, const char *mode = "r", const int offset = 0) = 0;
  virtual int write  (const char *, const char *mode = "w", const int offset = 0) = 0;
  char    *readLine (FILE *);

  int readSVindex  (const char *, const char *mode = "r", const int offset = 0);
  int writeSVindex (const char *, const char *mode = "w", const int offset = 0);

  BaseExample& operator=(BaseExample &);
  BaseExample ();
  virtual ~BaseExample();
};

};
#endif
