#include "utility.h"
#include <string>
#include <algorithm>

void* new2d(int h, int w, int size)
{
    register int i;
	register int j;
    void **p;

    p = (void**)new char[h*sizeof(void*) + h*w*size];

    for(i = 0; i < h; i++)
    {
        p[i] = ((char *)(p + h)) + i*w*size;
    }

    return p;
}
/*
void* new3d(int h, int w, int z, int size)
{
    register int i;
    void ***p;

    p = (void***)new char[h*sizeof(void*)*sizeof(void*) + h*w*sizeof(void*) + h*w*z*size];

    for(i = 0; i < h; i++)
    {
		p[i] = ((char *)(p + h)) + i*w*size;	
		for(j = 0; j<w; j++)
			p[i] = ((char *)(p + h)) + i*w*size;	
    }

    return p;
}
*/

float* read_volume(string file, Grid &g)  //read volume data
{

	int size;
		FILE* in;
		int data_type;

		char temp[100];
		char endian[100];
		char type[100];

		size_t found,found2;
		//string file=fname;	
		found = file.find_last_of("/\\");
		g.data.path = file.substr(0,found+1);
		found2 = file.find_last_of(".");
		g.data.dataname = file.substr(found+1,found2-found-1);		

		in = fopen(file.c_str(), "r");
		if (in == NULL) {
			//MOD-BY-TzuHsuan 10/08/2012-BEGIN
			printf(" can't open header %s\n", file.c_str());
			//MOD-BY-TzuHsuan 10/08/2012-END
			exit(0);
		}

		while(fscanf(in,"%s",temp)!=EOF)
		{
			if(strcmp(temp,"sizes:")==0)
			{
				fscanf(in,"%d",&g.data.xdim);
				fscanf(in,"%d",&g.data.ydim);
				fscanf(in,"%d",&g.data.zdim);
				
			}
			if(strcmp(temp,"endian:")==0)
			    fscanf(in,"%s",endian);

			if(strcmp(temp,"type:")==0)
				fscanf(in,"%s",type);

			if(strcmp(temp,"spacings:")==0)
			{
				fscanf(in,"%f",&g.x_space);
				fscanf(in,"%f",&g.y_space);
				fscanf(in,"%f",&g.z_space);
			}

			if(strcmp(temp,"dimension:")==0)
				fscanf(in,"%d",&g.dim);
		}
		

		if(strcmp(type,"uchar")==0)
			data_type = 1;
		else if(strcmp(type,"short")==0)
			data_type = 2;
		else if((strcmp(type,"int")==0) || (strcmp(type,"float")==0))
			data_type = 4;

		fclose(in);

		file.replace(file.find_last_of(".")+1,4,"raw");

		in = fopen(file.c_str(), "rb");
		if (in == NULL) {
			file.replace(file.find_last_of(".")+1,3,"dat");
			in = fopen(file.c_str(), "rb");
			if(in == NULL)
			{
				printf(" can't open file %s\n", file.c_str());
				exit(0);
			}
		}
		
		size = g.data.xdim*g.data.ydim*g.data.zdim;
		g.data.size = size;

		g.xdim = g.data.xdim*g.x_space;
		g.ydim = g.data.ydim*g.y_space;
		g.zdim = g.data.zdim*g.z_space;
		g.size = g.xdim*g.ydim*g.zdim;

		float* V = new float[size];
		unsigned char *p1;
		float *p2;
		unsigned short *p3;
		//void *V = new void[size];
			
		switch(data_type)
		{
		case 1:
			p1 = new unsigned char[size];
			fread(p1, data_type, size, in);
			for (int i=0; i<size; i++) 
			{
				//V[i] = (float) p1[i];
				V[i] = p1[i];
			}
			delete [] p1;
			
			break;
		case 2:
			if(strcmp(endian,"big")==0)
			{
				unsigned char *cptr,tmp;
				
				p3 = new unsigned short[1];

				for(int i=0;i<size;i++)
				{				
					fread(p3, data_type, 1, in);
					cptr = (unsigned char *)p3;
					tmp = cptr[0];
					cptr[0] = cptr[1];
					cptr[1] =tmp;
					V[i] = (float)(*p3);
				}

				delete [] p3;
				
			}
			else
			{
				p3 = new unsigned short[size];
			    fread(p3, data_type, size, in);

				for (int i=0; i<size; i++) 
				{
					V[i] = (float) p3[i];
				}
			
				delete [] p3;
				
			}
			
			break;

		case 4:
			if(strcmp(endian,"big")==0)
			{
				float retVal;
				char *cptr;
				char *cptr2 = (char*) &retVal;

				p2 = new float[1];

				for(int i=0;i<size;i++)
				{				
					fread(p2, data_type, 1, in);
					cptr = (char *) p2;

					cptr2[3] = cptr[0];
					cptr2[2] = cptr[1];
					cptr2[1] = cptr[2];
					cptr2[0] = cptr[3];
			
					V[i] = retVal;
				}

				delete [] p2;
			}
			else
			{
				p2 = new float[size];
				fread(p2, data_type, size, in);
				for (int i=0; i<size; i++) 
				{
					V[i] = p2[i];
					//V[i] = V[i]>0 ? V[i]:0;
				}

			}
			delete [] p2;
			
			break;
		}
		//for (int i=0; i<size; i++) 
		//		V[i] = 100*V[i];
	
		g.data.minvalue = *min_element(V,V + g.data.size);
		g.data.maxvalue = *max_element(V,V + g.data.size);
		return(V);
}

float* read_raw_volume(string file, Data &data, int xdim, int ydim, int zdim)  //read raw data
{

		int size;
		FILE* in;
		int data_type;

		char temp[100];
		char endian[100];
		char type[100];

		size_t found,found2;
		//string file=fname;	
		found = file.find_last_of("/\\");
		data.path = file.substr(0,found+1);
		found2 = file.find_last_of(".");
		data.dataname = file.substr(found+1,found2-found-1);
		data.xdim=xdim;
		data.ydim=ydim;
		data.zdim=zdim;

		in = fopen(file.c_str(), "rb");
		if (in == NULL) {
			printf(" can't open file %s\n", file.c_str());
			exit(0);
		}
		
		size = data.xdim*data.ydim*data.zdim;
		data.size = size;

		float* V = new float[size];
		//void *V = new void[size];	

		fread(V, sizeof(float), size, in);

		data.minvalue = *min_element(V,V + data.size);
		data.maxvalue = *max_element(V,V + data.size);

		return(V);
}

myvector* read_vector_volume(string file, VectorData &vec_data, int vec_dim)  //read vector data
{

		int size;
		FILE* in;
		int data_type;

		char temp[100];
		char endian[100];
		char type[100];

		size_t found,found2;
		//string file=fname;	
		found = file.find_last_of("/\\");
		vec_data.path = file.substr(0,found+1);
		found2 = file.find_last_of(".");
		vec_data.dataname = file.substr(found+1,found2-found-1);

		in = fopen(file.c_str(), "rb");
		if (in == NULL) {
			printf(" can't open file %s\n", file.c_str());
			exit(0);
		}
		
		fread(&vec_data.xdim,sizeof(int),1,in);
		fread(&vec_data.ydim,sizeof(int),1,in);
		fread(&vec_data.zdim,sizeof(int),1,in);
		
		size = vec_data.xdim*vec_data.ydim*vec_data.zdim;
		vec_data.size = size;

		float* V = new float[size*vec_dim];
		fread(V, sizeof(float), size*vec_dim, in);
		myvector* V1 = new myvector[size];

		vec_data.minvalue.x = V[0];
		vec_data.minvalue.y = V[1];
		vec_data.minvalue.z = V[2];
		vec_data.maxvalue.x = V[0];
		vec_data.maxvalue.y = V[1];
		vec_data.maxvalue.z = V[2];

		for(int i=0;i<size;i++)
		{
			V1[i].x = V[i*vec_dim];
			V1[i].y = V[i*vec_dim+1];
			V1[i].z = V[i*vec_dim+2];
			if(V1[i].x>vec_data.maxvalue.x)
				vec_data.maxvalue.x = V1[i].x;
			if(V1[i].y>vec_data.maxvalue.y)
				vec_data.maxvalue.y = V1[i].y;
			if(V1[i].z>vec_data.maxvalue.z)
				vec_data.maxvalue.z = V1[i].y;

			if(V1[i].x<vec_data.minvalue.x)
				vec_data.minvalue.x = V1[i].x;
			if(V1[i].x<vec_data.minvalue.y)
				vec_data.minvalue.y = V1[i].y;
			if(V1[i].x<vec_data.minvalue.z)
				vec_data.minvalue.z = V1[i].z;
		}

		delete [] V;

		return(V1);
}

void read_header(char* fname, Grid &g)  //read volume data
{

	int size;
		FILE* in;
		int data_type;

		char temp[100];
		char endian[100];
		char type[100];

		size_t found,found2;
		string file=fname;	
		found = file.find_last_of("/\\");
		g.data.path = file.substr(0,found+1);
		found2 = file.find_last_of(".");
		g.data.dataname = file.substr(found+1,found2-found-1);		

		in = fopen(file.c_str(), "r");
		if (in == NULL) {
			//MOD-BY-TzuHsuan 10/08/2012-BEGIN
			printf(" can't open header %s\n", file.c_str());
			//MOD-BY-TzuHsuan 10/08/2012-END
			exit(0);
		}

		while(fscanf(in,"%s",temp)!=EOF)
		{
			if(strcmp(temp,"sizes:")==0)
			{
				fscanf(in,"%d",&g.data.xdim);
				fscanf(in,"%d",&g.data.ydim);
				fscanf(in,"%d",&g.data.zdim);
				
			}
			if(strcmp(temp,"endian:")==0)
			    fscanf(in,"%s",endian);

			if(strcmp(temp,"type:")==0)
				fscanf(in,"%s",type);

			if(strcmp(temp,"spacings:")==0)
			{
				fscanf(in,"%f",&g.x_space);
				fscanf(in,"%f",&g.y_space);
				fscanf(in,"%f",&g.z_space);
			}

			if(strcmp(temp,"dimension:")==0)
				fscanf(in,"%d",&g.dim);

			if(strcmp(temp,"minimum:")==0)
				fscanf(in,"%f",&g.data.minvalue);

			if(strcmp(temp,"maximum:")==0)
				fscanf(in,"%f",&g.data.maxvalue);
		}

		if(strcmp(type,"uchar")==0)
			data_type = 1;
		else if(strcmp(type,"short")==0)
			data_type = 2;
		else if((strcmp(type,"int")==0) || (strcmp(type,"float")==0))
			data_type = 4;

		fclose(in);
		
		size = g.data.xdim*g.data.ydim*g.data.zdim;
		g.data.size = size;

		g.xdim = g.data.xdim*g.x_space;
		g.ydim = g.data.ydim*g.y_space;
		g.zdim = g.data.zdim*g.z_space;
		g.size = g.xdim*g.ydim*g.zdim;
}

void read_GMM(char* fname, Grid &g)  //read volume data
{

		int size,distrib_num;
		FILE* in;
		int data_type;

		char temp[100];
		char endian[100];
		char type[100];

		size_t found,found2;
		string file=fname;	
		found = file.find_last_of("/\\");
		g.data.path = file.substr(0,found+1);
		found2 = file.find_last_of(".");
		g.data.dataname = file.substr(found+1,found2-found-1);		

		in = fopen(file.c_str(), "r");
		if (in == NULL) {
			//MOD-BY-TzuHsuan 10/08/2012-BEGIN
			printf(" can't open header %s\n", file.c_str());
			//MOD-BY-TzuHsuan 10/08/2012-END
			exit(0);
		}

		while(fscanf(in,"%s",temp)!=EOF)
		{
			if(strcmp(temp,"sizes:")==0)
			{
				fscanf(in,"%d",&g.data.xdim);
				fscanf(in,"%d",&g.data.ydim);
				fscanf(in,"%d",&g.data.zdim);
				
			}
			if(strcmp(temp,"endian:")==0)
			    fscanf(in,"%s",endian);

			if(strcmp(temp,"type:")==0)
				fscanf(in,"%s",type);

			if(strcmp(temp,"spacings:")==0)
			{
				fscanf(in,"%f",&g.x_space);
				fscanf(in,"%f",&g.y_space);
				fscanf(in,"%f",&g.z_space);
			}

			if(strcmp(temp,"dimension:")==0)
				fscanf(in,"%d",&g.dim);

			if(strcmp(temp,"distribsize:")==0)
				fscanf(in,"%d",&distrib_num);
		}
		file.replace(file.find_last_of(".")+1,4,"raw");

		if(strcmp(type,"uchar")==0)
			data_type = 1;
		else if(strcmp(type,"short")==0)
			data_type = 2;
		else if((strcmp(type,"int")==0) || (strcmp(type,"float")==0))
			data_type = 4;

		fclose(in);

		in = fopen(file.c_str(), "rb");
		if (in == NULL) {
			printf(" can't open file %s\n", file.c_str());
			exit(0);
		}
		
		size = g.data.xdim*g.data.ydim*g.data.zdim;
		g.data.size = size;

		g.xdim = g.data.xdim*g.x_space;
		g.ydim = g.data.ydim*g.y_space;
		g.zdim = g.data.zdim*g.z_space;
		g.size = g.xdim*g.ydim*g.zdim;

		int newsize = size*distrib_num*3;

		float* V = new float[newsize];
		unsigned char *p1;
		float *p2;
		unsigned short *p3;
		//void *V = new void[size];
			
		switch(data_type)
		{
		case 1:
			p1 = new unsigned char[newsize];
			fread(p1, data_type, newsize, in);
			for (int i=0; i<newsize; i++) 
			{
				//V[i] = (float) p1[i];
				V[i] = p1[i];
			}
			delete [] p1;
			
			break;
		case 2:
			if(strcmp(endian,"big")==0)
			{
				unsigned char *cptr,tmp;
				
				p3 = new unsigned short[1];

				for(int i=0;i<newsize;i++)
				{				
					fread(p3, data_type, 1, in);
					cptr = (unsigned char *)p3;
					tmp = cptr[0];
					cptr[0] = cptr[1];
					cptr[1] =tmp;
					V[i] = (float)(*p3);
				}

				delete [] p3;
				
			}
			else
			{
				p3 = new unsigned short[newsize];
			    fread(p3, data_type, newsize, in);

				for (int i=0; i<newsize; i++) 
				{
					V[i] = (float) p3[i];
				}
			
				delete [] p3;
				
			}
			
			break;

		case 4:
			if(strcmp(endian,"big")==0)
			{
				float retVal;
				char *cptr;
				char *cptr2 = (char*) &retVal;

				p2 = new float[1];

				for(int i=0;i<newsize;i++)
				{				
					fread(p2, data_type, 1, in);
					cptr = (char *) p2;

					cptr2[3] = cptr[0];
					cptr2[2] = cptr[1];
					cptr2[1] = cptr[2];
					cptr2[0] = cptr[3];
			
					V[i] = retVal;
				}

				delete [] p2;
			}
			else
			{
				p2 = new float[newsize];
				fread(p2, data_type, newsize, in);
				for (int i=0; i<newsize; i++) 
				{
					V[i] = p2[i];
					//V[i] = V[i]>0 ? V[i]:0;
				}

			}
			delete [] p2;
			
			break;
		}

		for(int i=0;i<size;i++)
		{
			g.distribution.gmm[i].mixtured_numer=distrib_num;
			for(int j=0;j<distrib_num;j++)
			{
				int index = i*distrib_num*3+j*3;
				g.distribution.gmm[i].parameter.push_back(V[index]);
				g.distribution.gmm[i].parameter.push_back(V[index+1]);
				g.distribution.gmm[i].parameter.push_back(V[index+2]);
			}
		}

		delete [] V;

}

float* read_GMM2(char* fname, Grid &g)  //read volume data
{

		int size,distrib_num;
		FILE* in;
		int data_type;

		char temp[100];
		char endian[100];
		char type[100];

		size_t found,found2;
		string file=fname;	
		found = file.find_last_of("/\\");
		g.data.path = file.substr(0,found+1);
		found2 = file.find_last_of(".");
		g.data.dataname = file.substr(found+1,found2-found-1);		

		in = fopen(file.c_str(), "r");
		if (in == NULL) {
			//MOD-BY-TzuHsuan 10/08/2012-BEGIN
			printf(" can't open header %s\n", file.c_str());
			//MOD-BY-TzuHsuan 10/08/2012-END
			exit(0);
		}

		while(fscanf(in,"%s",temp)!=EOF)
		{
			if(strcmp(temp,"sizes:")==0)
			{
				fscanf(in,"%d",&g.data.xdim);
				fscanf(in,"%d",&g.data.ydim);
				fscanf(in,"%d",&g.data.zdim);
				
			}
			if(strcmp(temp,"endian:")==0)
			    fscanf(in,"%s",endian);

			if(strcmp(temp,"type:")==0)
				fscanf(in,"%s",type);

			if(strcmp(temp,"spacings:")==0)
			{
				fscanf(in,"%f",&g.x_space);
				fscanf(in,"%f",&g.y_space);
				fscanf(in,"%f",&g.z_space);
			}

			if(strcmp(temp,"dimension:")==0)
				fscanf(in,"%d",&g.dim);

			if(strcmp(temp,"distribsize:")==0)
				fscanf(in,"%d",&distrib_num);
		}
		file.replace(file.find_last_of(".")+1,4,"raw");

		if(strcmp(type,"uchar")==0)
			data_type = 1;
		else if(strcmp(type,"short")==0)
			data_type = 2;
		else if((strcmp(type,"int")==0) || (strcmp(type,"float")==0))
			data_type = 4;

		fclose(in);

		in = fopen(file.c_str(), "rb");
		if (in == NULL) {
			printf(" can't open file %s\n", file.c_str());
			exit(0);
		}
		
		size = g.data.xdim*g.data.ydim*g.data.zdim;
		g.data.size = size;

		g.xdim = g.data.xdim*g.x_space;
		g.ydim = g.data.ydim*g.y_space;
		g.zdim = g.data.zdim*g.z_space;
		g.size = g.xdim*g.ydim*g.zdim;
		int newsize = size*distrib_num*3;

		float* V = new float[newsize];
		unsigned char *p1;
		float *p2;
		unsigned short *p3;
		//void *V = new void[size];
			
		switch(data_type)
		{
		case 1:
			p1 = new unsigned char[newsize];
			fread(p1, data_type, newsize, in);
			for (int i=0; i<newsize; i++) 
			{
				//V[i] = (float) p1[i];
				V[i] = p1[i];
			}
			delete [] p1;
			
			break;
		case 2:
			if(strcmp(endian,"big")==0)
			{
				unsigned char *cptr,tmp;
				
				p3 = new unsigned short[1];

				for(int i=0;i<newsize;i++)
				{				
					fread(p3, data_type, 1, in);
					cptr = (unsigned char *)p3;
					tmp = cptr[0];
					cptr[0] = cptr[1];
					cptr[1] =tmp;
					V[i] = (float)(*p3);
				}

				delete [] p3;
				
			}
			else
			{
				p3 = new unsigned short[newsize];
			    fread(p3, data_type, newsize, in);

				for (int i=0; i<newsize; i++) 
				{
					V[i] = (float) p3[i];
				}
			
				delete [] p3;
				
			}
			
			break;

		case 4:
			if(strcmp(endian,"big")==0)
			{
				float retVal;
				char *cptr;
				char *cptr2 = (char*) &retVal;

				p2 = new float[1];

				for(int i=0;i<newsize;i++)
				{				
					fread(p2, data_type, 1, in);
					cptr = (char *) p2;

					cptr2[3] = cptr[0];
					cptr2[2] = cptr[1];
					cptr2[1] = cptr[2];
					cptr2[0] = cptr[3];
			
					V[i] = retVal;
				}

				delete [] p2;
			}
			else
			{
				p2 = new float[newsize];
				fread(p2, data_type, newsize, in);
				for (int i=0; i<newsize; i++) 
				{
					V[i] = p2[i];
					//V[i] = V[i]>0 ? V[i]:0;
				}

			}
			delete [] p2;
			
			break;
		}
		return V;

}

void init_Grid(Grid &G)
{
 /*
	G.gradientMag = NULL;
	G.CurvMag = NULL;
	G.Hessian = NULL;
	G.gradient = NULL;
	G.Min_Curv_Dir = NULL;
	G.Max_Curv_Dir = NULL;
	*/
	G.data.volume = NULL;
	G.global_histogram.pdf = NULL;	
	G.distribution.histogram = NULL;	
	G.distribution.gmm = NULL;
	G.distribution.gmm2.para=NULL;

}
void destroy_Grid(Grid &G)
{
	/*
	if(G.gradientMag!=NULL)
		delete [] G.gradientMag;
	if(G.CurvMag!=NULL)
		delete [] G.CurvMag;
	if(G.Hessian!=NULL)
		delete [] G.Hessian;
	if(G.gradient!=NULL)
		delete [] G.gradient;
	if(G.Min_Curv_Dir!=NULL)
		delete [] G.Min_Curv_Dir;
	if(G.Max_Curv_Dir!=NULL)
		delete [] G.Max_Curv_Dir;	
		*/
	if(G.distribution.histogram!=NULL)
		delete [] G.distribution.histogram;
	if(G.distribution.gmm!=NULL)
		delete [] G.distribution.gmm;
	if(G.distribution.gmm2.para)
		delete [] G.distribution.gmm2.para;
	if(G.data.volume!=NULL)
		delete [] G.data.volume;
	if(G.global_histogram.pdf!=NULL)
		delete [] G.global_histogram.pdf;
}

void init_Data(Data &data)
{
	data.volume=NULL;
	data.isosurface.vertices = NULL;
}

void init_VectorData(VectorData &vec_data)
{
	vec_data.volume = NULL;
}

void destroy_Data(Data &data)
{
	if(data.volume!=NULL)
		delete [] data.volume;
	if(data.isosurface.vertices!=NULL)
		delete [] data.isosurface.vertices;
}

void destroy_VectorData(VectorData &vec_data)
{
	if(vec_data.volume!=NULL)
		delete [] vec_data.volume;
		
}

void init_Isosurface(Isosurface &isosurf)
{
	isosurf.vertices = NULL;
	isosurf.triangle = NULL;
}

void destroy_Isosurface(Isosurface &isosurf)
{
	if(isosurf.vertices!=NULL)
		delete [] isosurf.vertices;
	if(isosurf.triangle!=NULL)
		delete [] isosurf.triangle;
}

void convert_spherical_coord(float & theta, float & phi, float & r, float x, float y, float z)
{
	const float EPSILON = 1e-12;
	r = sqrtf(x*x+y*y+z*z);

	if(r>EPSILON)
	{
		theta = acos(z/r);
	}
	else
		theta = 0;

	if(fabs(x)<EPSILON)
	{
		if(fabs(y)<EPSILON)
			phi = 0;
		else if(y>0)
			phi = M_PI/2;
		else if(y<0)
			phi = M_PI + M_PI/2.0;

	}
	else
	{
		if(x>0 && y>0)
			phi = atan(y/x);
		else if(x>0 && y<0)
			phi = 2*M_PI + atan(y/x);
		else if(x<0)
			phi = M_PI + atan(y/x);

	}
}