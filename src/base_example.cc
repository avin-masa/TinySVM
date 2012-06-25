#include "base_example.h"
#include "common.h"

// $Id: base_example.cc,v 1.21 2001/08/24 13:07:52 taku-ku Exp $;

// misc function
namespace TinySVM {

int
inc_refcount_feature_node (feature_node *f)
{
   int i;
   for (i = 0; f[i].index >= 0; i++);
   return --f[i].index;
}

int
dec_refcount_feature_node (feature_node *f)
{
   int i;
   for (i = 0; f[i].index >= 0; i++);
   return ++f[i].index;
}

int
comp_feature_node (const void *x1, const void *x2)
{
  feature_node *p1 = (feature_node *) x1;
  feature_node *p2 = (feature_node *) x2;
  return (p1->index > p2->index);
}

feature_node *
copy_feature_node (const feature_node * f)
{
  int i;
  for (i = 0; f[i].index >= 0; i++);

  try {
    feature_node *r = new feature_node[i + 1];
    for (i = 0; f[i].index >= 0; i++) {
      r[i].index = f[i].index;
      r[i].value = f[i].value;
    }
    r[i].index = -1;
    return r;
  }

  catch (...) {
    fprintf (stderr, "copy_feature_node(): Out of memory\n");
    exit (EXIT_FAILURE);
  }
}

feature_node *
str2feature_node (const char *s)
{
  int elmnum = 0;
  int len = strlen (s);

  for (int i = 0; i < len; i++) if (s[i] == ':') elmnum++;

  try {
    feature_node *_x = new feature_node[elmnum + 1];

    int j = 0;
    for (int i = 0; j < elmnum && i < len;) {
      while (i < len && isspace (s[i]))	i++;
      _x[j].index = atoi (s + i);
      while (i + 1 < len && s[i] != ':') i++;
      _x[j].value = atof (s + i + 1);
      j++;
      while (i < len && !isspace (s[i])) i++;
    }

    // dumy index
    _x[j].index = -1;
    _x[j].value = 0;
    return _x;
  }

  catch (...) {
    fprintf (stderr, "str2feature_node(): Out of memory\n");
    exit (EXIT_FAILURE);
  }
}

feature_node *
fix_feature_node (feature_node * _x)
{
  register int i;
  register int cindex = -1;
  register int sorted = 1;

  // check sort
  for (i = 0; _x[i].index >= 0; i++) {
     if (cindex >= _x[i].index) sorted = 0;
     cindex = _x[i].index;
  }
   
  // sort
  if (!sorted) qsort ((void *) _x, i, sizeof (feature_node), comp_feature_node);
  return _x;
}

BaseExample::BaseExample ()
{
  l = d = pack_d = strl = 0;
  stre = 0;
  x = 0;
  y = 0;
  alpha = 0;
  G  = 0;
  feature_type = class_type = BINARY_FEATURE;
}

BaseExample::~BaseExample ()
{
  for (int i = 0; i < l; i++) {
     if (x && dec_refcount_feature_node(x[i]) == -1) delete [] x[i];
  }

  delete [] x;
  delete [] y;
  delete [] alpha;
  delete [] G;
  delete [] stre;
}

// copy constructor
BaseExample &
BaseExample::operator =(BaseExample & e) 
{
  if (this != &e) {
    clear ();
    for (int i = 0; i < e.l; i++) {
       inc_refcount_feature_node (e.x[i]);
       add (e.y[i], e.x[i]);
    }
    l = e.l;
    pack_d = e.pack_d;
    svindex_size = e.svindex_size;
    if (svindex_size) {
	clone (alpha, e.alpha, svindex_size);
	clone (G,     e.G,     svindex_size);
    }
  }

  return *this;
}

int
BaseExample::clear ()
{
  for (int i = 0; i < l; i++) {
     if (x && dec_refcount_feature_node(x[i]) == -1) delete [] x[i];
  }

  delete [] x;
  delete [] y;
  delete [] alpha;
  delete [] G;

  l = d = pack_d = 0;
  x = 0;
  y = 0;
  alpha = 0;
  G = 0;
  return 0;
}
   

char *
BaseExample::readLine (FILE * fp)
{
  long len;
  int c;
  char *tstr;

  try {
    if (! stre) {
      strl = MAXLEN;
      stre = new char[strl];
    }

    len = 0;
    tstr = stre;

    while (1) {
      if (len >= strl) {
	tstr = resize (tstr, strl, strl + MAXLEN, (char)0);
        strl += MAXLEN;
	stre = tstr;
      }

      c = fgetc (fp);
      if (c == '\n' || c == '\r') {
        tstr[len] = '\0';
        break;
      }

      if (c == EOF && feof (fp)) {
        tstr[len] = '\0';
        if (feof (fp) && len == 0) tstr = 0;
        break;
      }

      tstr[len++] = c;
    }

    return tstr;
  }

  catch (...) {
    fprintf (stderr, "BaseExample::readLine(): Out of memory\n");
    exit (EXIT_FAILURE);
  }
}
   
int
BaseExample::remove (int i)
{
   if (i < 0 || i >= l || ! x || ! y) {
      fprintf(stderr, "BaseExample::set (): Out of range\n");
      return 0;
   }

  if (dec_refcount_feature_node(x[i]) == -1) delete [] x[i];
  for (int j = i+1; j < l; j++) {
     x[j-1] = x[j];
     y[j-1] = y[j];
  }
     
  return --l;
}
     
int 
BaseExample::set (int i, const double _y, feature_node *_x)
{
   if (i < 0 || i >= l || ! x || ! y) {
      fprintf(stderr, "BaseExample::set (): Out of range\n");
      return 0;
   }
   
  if (dec_refcount_feature_node(x[i]) == -1) delete [] x[i];
  _x = fix_feature_node(_x);
  inc_refcount_feature_node(_x);
   
  x[i] = _x;
  y[i] = _y;
  return 1;
}
   
int 
BaseExample::set (int i, const double _y, const char *s)
{
   return set(i, _y, (feature_node *) str2feature_node (s));
}

int 
BaseExample::set (int _i, const char *s)
{
  double _y = 0;
  int len = strlen (s);

  int i;
  for (i = 0; i < len;) {
    while (isspace (s[i])) i++;
    _y = atof (s + i);
    while (i < len && !isspace (s[i])) i++;
    while (i < len && isspace (s[i]))  i++;
    break;
  }

  return set (_i, _y, (const char *) (s + i));
}
   

int
BaseExample::add (const double _y, feature_node * _x)
{
  int fnum = 0;

  try {
    feature_node *node = fix_feature_node ((feature_node *) _x);
     
    // check contents
    for (int i = 0; (node + i)->index >= 0; i++) {
      if ((node + i)->value != 1) feature_type = DOUBLE_FEATURE; // check feature type
      d = max (d, (node + i)->index);	// save max dimension
      fnum++;
    }
     
    // incriment refcount
    inc_refcount_feature_node (node);

    pack_d = max (fnum, pack_d);
    if (! fnum) return 0; // empty node

    // check class type
    if (_y != +1 && _y != -1) class_type = DOUBLE_FEATURE;

    // resize
    x = append (x, l, node, (feature_node*)0);
    y = append (y, l, _y, 0.0);
    l++;

    return 1;
  }

  catch (...) {
    fprintf (stderr, "BaseExample::add(): Out of memory\n");
    exit (EXIT_FAILURE);
  }
}

int
BaseExample::add (const double _y, const char *s)
{
  return add (_y, (feature_node *) str2feature_node (s));
}

int
BaseExample::add (const char *s)
{
  double _y = 0;
  int len = strlen (s);

  int i;
  for (i = 0; i < len;) {
    while (isspace (s[i])) i++;
    _y = atof (s + i);
    while (i < len && !isspace (s[i])) i++;
    while (i < len && isspace (s[i]))  i++;
    break;
  }

  return add (_y, (const char *) (s + i));
}

int
BaseExample::writeSVindex (const char *filename, const char *mode, const int offset)
{
  if (!alpha || !G) return 0;

  FILE *fp = fopen (filename, mode);
  if (!fp) return 0;

  for (int i = 0; i < svindex_size; i++)
    fprintf (fp, "%.16g %.16g\n", alpha[i], G[i]);

  fclose (fp);
  return 1;
}

int
BaseExample::readSVindex (const char *filename, const char *mode, const int offset)
{
  if (l == 0) {
    fprintf(stderr, "Fatal: size == 0, Read model/example file before reading .idx file\n");
    return 0;
  }

  FILE *fp = fopen (filename, mode);
  if (!fp) return 0;

  delete [] alpha;
  delete [] G;

  int _l = 0;
  char *buf;

  while ((buf = readLine (fp)) != NULL) {
    double _alpha, _G;
    if (2 != sscanf (buf, "%lf %lf\n", &_alpha, &_G)) {
      fprintf(stderr, "Fatal: Format error %s, line %d\n", filename, _l);
      fclose (fp);
      return 0;
    }

    alpha = append (alpha, _l, _alpha, 0.0);
    G     = append (G,     _l, _G,     0.0);
    _l++;
  }

  fclose (fp);

  //  check size of idx file
  if (l < _l) {
    fprintf(stderr, "Fatal: model/example size (%d) < idx size (%d)\n", l, _l);
    delete [] alpha;
    delete [] G;
    alpha = 0;
    G = 0;
    return 0;
  }

  svindex_size = _l;
  return 1;
}
}
