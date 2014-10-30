#pragma once

#include <string>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

#define EPSINON		0.01
#define M_PI	3.1415926

enum Type
{
	UNSIGNED_CHAR,
	UNSIGNED_SHORT,
    INTEGER
};

struct myvector{
	double x;
	double y;
	double z;

	myvector()
	{}

	myvector(double _x, double _y, double _z)
	{
		x = _x;		y = _y;		z = _z;
	}

	myvector& operator=(myvector const& e)
	{
		x = e.x;		y = e.y;		z = e.z;

		return *this;
	}
};
struct myvector4{
	double x;
	double y;
	double z;
	double w;

	myvector4()
	{}

	myvector4(double _x, double _y, double _z, double _w)
	{
		x = _x;		y = _y;		z = _z;	 w = _w;
	}

	myvector4& operator=(myvector4 const& e)
	{
		x = e.x;		y = e.y;		z = e.z;		w = e.w;

		return *this;
	}
};

struct GMM
{
	int mixtured_numer;
	vector<float> parameter;
	float *para;
};

struct Isosurface
{
	float isovalue;
	float *vertices;
	int *triangle;
	int vertices_num;
	int triangle_num;
};
struct Histogram
{
	float *pdf;
	int bin_num;
	float bin_width;
	
};
struct Data
{
	float* volume;		
	float maxvalue;
	float minvalue;
	int xdim,ydim,zdim;
	int size;	
	string path;
	string dataname;
	string extension;
	Isosurface isosurface;
};

struct VectorData
{
	myvector* volume;		
	myvector maxvalue;
	myvector minvalue;
	int xdim,ydim,zdim;
	int size;	
	string path;
	string dataname;
	string extension;
};

struct Distribution //histogram on grid
{
	float** histogram;
	GMM* gmm;
	GMM gmm2;
	//myvector *block;
	int xdim,ydim,zdim;
	int blocksize;
	int bin_number;
	double binWidth;
};

struct Grid
{	
	int dim;
	int xdim,ydim,zdim;
	int size;
	float x_space,y_space,z_space;

	//float *gradientMag;
	//float *CurvMag;
	//float **Hessian;
	//myvector *gradient;
	//myvector *Min_Curv_Dir;
	//myvector *Max_Curv_Dir;

	Data data;
	Histogram global_histogram;
	Distribution distribution; //ignore this
};

template <class T>
inline T abs(const T &x) { return (x > 0 ? x : -x); };

template <class T>
inline int sign(const T &x) { return (x >= 0 ? 1 : -1); };

template <class T>
inline T square(const T &x) { return x*x; };

template <class T>
inline T bound(const T &x, const T &min, const T &max) {
  return (x < min ? min : (x > max ? max : x));
}



void* new2d(int h, int w, int size);
float* read_volume(string file, Grid &g);
float* read_raw_volume(string file, Data &data, int xdim, int ydim, int zdim);
myvector* read_vector_volume(string file, VectorData &vec_data, int vec_dim);
void read_header(char* fname, Grid &g);
void read_GMM(char* fname, Grid &g);
float* read_GMM2(char* fname, Grid &g);
void init_Grid(Grid &G);
void destroy_Grid(Grid &G);
void init_Data(Data &data);
void init_Isosurface(Isosurface &isosurf);
void init_VectorData(VectorData &vec_data);
void destroy_Isosurface(Isosurface &isosurf);
void destroy_Data(Data &data);
void destroy_VectorData(VectorData &vec_data);
void convert_spherical_coord(float & theta, float & phi, float & r, float x, float y, float z);
