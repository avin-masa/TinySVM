#include "q_matrix.h"

// $Id: q_matrix.cc,v 1.5 2001/08/14 09:09:04 taku-ku Exp $;

namespace TinySVM {

QMatrix::QMatrix (const BaseExample &example, const Param & param):
Kernel (example, param)
{
  cache_binary        = 0;
  cache_normal        = 0;
  binary_kernel_cache = 0;

  miss = hit = size = 0;
  cache_size = param.cache_size;

  // making cache
  if (feature_type == BINARY_FEATURE) {
    binary_kernel_cache = new int[pack_d + 1];
    for (int i = 0; i < (pack_d + 1); i++)
      binary_kernel_cache[i] = (int) (this->getKernel ((double) i));

    if (pack_d <= 255) {
      cache_binary = new Cache <unsigned char> (l, 0.9 * cache_size);
      cache_normal = new Cache <double>        (l, 0.1 * cache_size);
      size = cache_binary->size + cache_normal->size;
      _getQ = &QMatrix::_getQ_binary_char;
    } else {
      cache_normal = new Cache <double>(l, cache_size);
      size = cache_normal->size;
      _getQ = &QMatrix::_getQ_binary_double;
    }
  } else {
    cache_normal = new Cache <double>(l, cache_size);
    size = cache_normal->size;
    _getQ = &QMatrix::_getQ_normal;
  }
}

QMatrix::~QMatrix ()
{
  delete [] binary_kernel_cache;
  delete cache_binary;
  delete cache_normal;
}

void 
QMatrix::rebuildCache(int active_size)
{
  if (cache_binary && cache_normal) {
    delete cache_binary;
    delete cache_normal;
    cache_binary = new Cache <unsigned char> (active_size,  0.9 * cache_size);
    cache_normal = new Cache <double>        (active_size,  0.1 * cache_size);
    size = cache_binary->size + cache_normal->size;
  } else if (cache_normal) {
    delete cache_normal;
    cache_normal = new Cache <double>(active_size, cache_size);
    size = cache_normal->size;
  }
}

void 
QMatrix::swap_index(const int i, const int j)
{
  if (cache_normal) {
    cache_normal->swap_index  (i, j);
    cache_normal->delete_index(j);
  }

  if (cache_binary) {
    cache_binary->swap_index   (i, j); 
    cache_binary->delete_index (j);
  }
}

void 
QMatrix::update(const int active_size)
{
  size = 0;
 
 if (cache_normal) {
    cache_normal->update (active_size);
    size += cache_normal->size;
 }

  if (cache_binary) {
    cache_binary->update (active_size);
    size += cache_binary->size;
  }
}

void
QMatrix::delete_index(const int i)
{
  if (cache_normal) cache_normal->delete_index(i);
  if (cache_binary) cache_binary->delete_index(i);
}

// get_Q for SVM
double *
QMatrix::_getQ_normal (const int i, const int active_size)
{
  double *data;

  if (cache_normal->getData (i, &data)) {
    hit++;
  } else {
    for (int j = 0; j < active_size; j++) 
      data[j] = y[i] * y[j] * getKernel(x[i], x[j]);
    miss++;
  }

  return data;
}

double *
QMatrix::_getQ_binary_char (const int i, const int active_size)
{
  double *data1;

  if (cache_normal->getData (i, &data1)) {
    hit++;
  } else {
    unsigned char *data2;
    if (cache_binary->getData (i, &data2)) {
      for (int j = 0; j < active_size; j++) 
	data1[j] = y[i] * y[j] * binary_kernel_cache[data2[j]];
      hit++;
    } else {
      for (int j = 0; j < active_size; j++) {
	data2[j] = (unsigned char) dot_binary (x[i], x[j]);
	data1[j] = y[i] * y[j] * binary_kernel_cache[data2[j]];
      }
      miss++;
    }
  }

  return data1;
}

double *
QMatrix::_getQ_binary_double (const int i, const int active_size)
{
  double *data;

  if (cache_normal->getData (i, &data)) {
    hit++;
  } else {
    for (int j = 0; j < active_size; j++) 
      data[j] =	(y[i] * y[j] * binary_kernel_cache[dot_binary (x[i], x[j])]);
    miss++;
  }

  return data;
}

}
