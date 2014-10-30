/*
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

// Simple 3D volume renderer

#ifndef _VOLUMERENDER_KERNEL_CU_
#define _VOLUMERENDER_KERNEL_CU_

#include <helper_cuda.h>
#include <helper_math.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "utility.h"
#include <time.h>
#include <thrust/random/normal_distribution.h>
#include <thrust/random/linear_congruential_engine.h>
#include <thrust/random.h>

typedef unsigned int  uint;
typedef unsigned char uchar;

cudaArray *d_volumeArray = 0;
cudaArray *d_normalArray = 0;
cudaArray *d_transferFuncArray1;
cudaArray *d_transferFuncArray2;

typedef unsigned char VolumeType;
//typedef unsigned short VolumeType;
//typedef float4 VolumeType;

//texture<VolumeType, 3, cudaReadModeNormalizedFloat> tex;         // 3D texture
texture<float, 3, cudaReadModeElementType> tex_volume;         // 3D texture
texture<float, 3, cudaReadModeElementType> tex_normal;         // 3D texture
texture<float4, 1, cudaReadModeElementType>         transferTex; // 1D transfer function texture
texture<float4, 1, cudaReadModeElementType>         transferTex_color; // 1D transfer function texture
texture<float, 1, cudaReadModeElementType>         transferTex_alpha; // 1D transfer function texture

typedef struct
{
    float4 m[3];
} float3x4;

__constant__ float3x4 c_invViewMatrix;  // inverse view matrix

struct Ray
{
    float3 o;   // origin
    float3 d;   // direction
};

__device__
int intersectBox(Ray r, float3 boxmin, float3 boxmax, float *tnear, float *tfar)
{
    // compute intersection of ray with all six bbox planes
    float3 invR = make_float3(1.0f) / r.d;
    float3 tbot = invR * (boxmin - r.o);
    float3 ttop = invR * (boxmax - r.o);

    // re-order intersections to find smallest and largest on each axis
    float3 tmin = fminf(ttop, tbot);
    float3 tmax = fmaxf(ttop, tbot);

    // find the largest tmin and the smallest tmax
    float largest_tmin = fmaxf(fmaxf(tmin.x, tmin.y), fmaxf(tmin.x, tmin.z));
    float smallest_tmax = fminf(fminf(tmax.x, tmax.y), fminf(tmax.x, tmax.z));

    *tnear = largest_tmin;
    *tfar = smallest_tmax;

    return smallest_tmax > largest_tmin;
}

// transform vector by matrix (no translation)
__device__
float3 mul(const float3x4 &M, const float3 &v)
{
    float3 r;
    r.x = dot(v, make_float3(M.m[0]));
    r.y = dot(v, make_float3(M.m[1]));
    r.z = dot(v, make_float3(M.m[2]));
    return r;
}

// transform vector by matrix with translation
__device__
float4 mul(const float3x4 &M, const float4 &v)
{
    float4 r;
    r.x = dot(v, M.m[0]);
    r.y = dot(v, M.m[1]);
    r.z = dot(v, M.m[2]);
    r.w = 1.0f;
    return r;
}

__device__ uint rgbaFloatToInt(float4 rgba)
{
    rgba.x = __saturatef(rgba.x);   // clamp to [0.0, 1.0]
    rgba.y = __saturatef(rgba.y);
    rgba.z = __saturatef(rgba.z);
    rgba.w = __saturatef(rgba.w);
    return (uint(rgba.w*255)<<24) | (uint(rgba.z*255)<<16) | (uint(rgba.y*255)<<8) | uint(rgba.x*255);
}

__global__ void
d_render(uint *d_output, uint imageW, uint imageH,
         float density, float brightness,
         float transferOffset, float transferScale, cudaExtent volumeSize, float min_value, float max_value, 
		 float gridScale_X, float gridScale_Y, float gridScale_Z, float tstep)
{
    const int maxSteps = 450;
    const float opacityThreshold = 0.99f;
    const float3 boxMin = make_float3(-1.0*gridScale_X, -1.0*gridScale_Y, -1.0*gridScale_Z);
    const float3 boxMax = make_float3(1.0*gridScale_X, 1.0*gridScale_Y, 1.0*gridScale_Z);
	//const float3 boxMin = make_float3(-1.0f, -1.0f, -1.0f);
    //const float3 boxMax = make_float3(1.0f, 1.0f, 1.0f);

    uint x = blockIdx.x*blockDim.x + threadIdx.x;
    uint y = blockIdx.y*blockDim.y + threadIdx.y;
    
	if ((x >= imageW) || (y >= imageH)) 
		return;

	float W = (float) imageW * gridScale_X;
	float H = (float) imageH * gridScale_Y;	

    float u = (x / (float) imageW)*2.0f*gridScale_X-gridScale_X;
    float v = (y / (float) imageH)*2.0f*gridScale_Y-gridScale_Y;
	//float u = (x / (float) W)*2.0f-1.0f;
    //float v = (y / (float) H)*2.0f-1.0f;
	//float w = (z / (float) imageH)*2.0f-1.0f;

    // calculate eye ray in world space
    Ray eyeRay;
    eyeRay.o = make_float3(mul(c_invViewMatrix, make_float4(0.0f, 0.0f, 0.0f, 1.0f)));
    eyeRay.d = normalize(make_float3(u, v, -2.0f));
    eyeRay.d = mul(c_invViewMatrix, eyeRay.d);

    // find intersection with box
    float tnear, tfar;
    int hit = intersectBox(eyeRay, boxMin, boxMax, &tnear, &tfar);

    if (!hit) return;

    if (tnear < 0.0f) tnear = 0.0f;     // clamp to near plane

    // march along ray from front to back, accumulating color
    float4 sum = make_float4(0.0f);
    float t = tnear;
    float3 pos = eyeRay.o + eyeRay.d*tnear;
    float3 step = eyeRay.d*tstep;
	float sample;
	float value_range = max_value - min_value;
	float temp_range = 1.0f/value_range;

    for (int i=0; i<maxSteps; i++)
    {
        // read from 3D texture
        // remap position to [0, 1] coordinates
        //sample = tex3D(tex, pos.x*0.5f+0.5f, pos.y*0.5f+0.5f, pos.z*0.5f+0.5f);
		sample = tex3D(tex_volume, (pos.x/(2.0*gridScale_X)+0.5)*(volumeSize.width-1), (pos.y/(2.0*gridScale_Y)+0.5)*(volumeSize.height-1), (pos.z/(2.0*gridScale_Z)+0.5)*(volumeSize.depth-1));
		
        sample = (sample-min_value)*temp_range;    // rmap scale to [0 , 1]
	
        // lookup in transfer function texture
        
		//float4 col = tex1D(transferTex, (sample-transferOffset)*transferScale);
		
		float4 col = tex1D(transferTex_color, (sample-transferOffset)*transferScale);
		float alpha = tex1D(transferTex_alpha, (sample-transferOffset)*transferScale);
		col.w = alpha;
		
		col.w *= density;
		
        // "under" operator for back-to-front blending
        //sum = lerp(sum, col, col.w);

        // pre-multiply alpha
        col.x *= col.w;
        col.y *= col.w;
        col.z *= col.w;

        // "over" operator for front-to-back blending
        sum = sum + col*(1.0f - sum.w);

        // exit early if opaque
        if (sum.w > opacityThreshold)
            break;

        t += tstep;

        if (t > tfar) break;

        pos += step;
    }

    sum *= brightness;

    // write output color
    d_output[y*imageW + x] = rgbaFloatToInt(sum);
}
__global__ void
gmm_kernel(thrust::random::uniform_real_distribution<float> dist, thrust::minstd_rand rng, float *gmm_V, float* gmm_P, int xdim, int ydim, int zdim, int distib_num, unsigned int m_seed2)
{
	uint x = blockIdx.x*blockDim.x + threadIdx.x;
    uint y = blockIdx.y*blockDim.y + threadIdx.y;
	uint z = blockIdx.y*blockDim.z + threadIdx.z;
	uint index = z*xdim*ydim + y*xdim + x;

	float sum,uni_rand,mean,stdev;
	bool flag=false;
	int gaussian_index;
	
	sum=gmm_P[index+2]; //weighted of first gaussian 
//	uni_rand = dist(rng);
/*	
	for(int j=0;j<distib_num-1;j++)
	{
		if(uni_rand<sum)
		{
			gmm_V[index] = j;
			flag = true;
			break;
		}
		else
			sum += gmm_P[index+(j+2)*3-1];
	}
	
	if(!flag)
		gmm_V[index] = distib_num-1;

	mean = gmm_P[(int)(index+gmm_V[index]*3)];
	stdev = gmm_P[(int)(index+gmm_V[index]*3+1)];
	
	thrust::minstd_rand rng2;
	rng2.seed(m_seed2);
	thrust::random::experimental::normal_distribution<float> dist2(mean,stdev);
	gmm_V[index] = dist2(rng2);
	*/
}

#if 0
__global__ void
Fuzzy_IsoSusrface(uint *v_output, uint xdim, uint ydim, uint zdim, float iso, int block_size, int binWidth,float minvalue)
{
	int index;
	int above,under;
	int offset = block_size*0.5;
	int iso_location = ((iso-minvalue)/binWidth)+0.5;
	int iter =0;

	float *block_data;
	float *hist;

	float score;

	uint x = blockIdx.x*blockDim.x + threadIdx.x;
    uint y = blockIdx.y*blockDim.y + threadIdx.y;
	uint z = blockIdx.z*blockDim.z + threadIdx.z;

	if(x>xdim || y>ydim || z>zdim)
		return;

	under = 0;
	above = 0;

	////////////////////////////////////////////////
	int z_lowerbond = (z-offset)>=0 ? z-offset:0;
	int y_lowerbond = (y-offset)>=0 ? y-offset:0;
	int x_lowerbond = (x-offset)>=0 ? x-offset:0;

	int z_upperbond = (z+offset)<zdim ? z+offset:zdim-1;
	int y_upperbond = (y+offset)<ydim ? y+offset:ydim-1;
	int x_upperbond = (x+offset)<xdim ? x+offset:xdim-1;
	
	cudaExtent size = make_cudaExtent(x_upperbond-x_lowerbond+1, y_upperbond-y_lowerbond+1, z_upperbond-z_lowerbond+1);
	cudaArray *d_blockArray = 0;
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float>();
    checkCudaErrors(cudaMalloc3DArray(&d_blockArray, &channelDesc, size));

	for(int z=z_lowerbond;z<=z_upperbond;z++)
		for(int y=y_lowerbond;y<=y_upperbond;y++)
			for(int x=x_lowerbond;x<=x_upperbond;x++)
			{
				int idx = x + y*xdim + z*xdim*ydim;
				d_blockArray[iter++] = d_volumeArray[idx];							
			}

////////////////////////////////////////////////////////////
				block_data = &tempdata[0];

				hist = info->histogram(block_data,tempdata.size(),binWidth,minvalue,maxvalue);

				for(int iter=0;iter<iso_location;iter++)
				{
					if(hist[iter]>0)
						under++;
				}

				for(int iter=iso_location;iter<info->hist_gridsize;iter++)
				{
					if(hist[iter]>0)
						above++;
				}
				if(under==0)
					score = above;
				else if(above==0)
					score = under;
				else
					score = (float)above/(float)under - (float)under/(float)above;
				/*
				if(under==0 || above==0)
					score = 1;
				else					
					score = above>=under ? (1-(float)under/(float)above):(1-(float)above/(float)under);
					*/
				V[index] = (score);	

	checkCudaErrors(cudaFreeArray(d_blockArray));
}
#endif

extern "C"
void setTextureFilterMode(bool bLinearFilter)
{
    tex_volume.filterMode = bLinearFilter ? cudaFilterModeLinear : cudaFilterModePoint;
	tex_normal.filterMode = bLinearFilter ? cudaFilterModeLinear : cudaFilterModePoint;
}

extern "C"
void init_volume_Cuda(float *h_volume, cudaExtent volumeSize)
{
    // create 3D array
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float>();
    checkCudaErrors(cudaMalloc3DArray(&d_volumeArray, &channelDesc, volumeSize));

    // copy data to 3D array
    cudaMemcpy3DParms copyParams = {0};
    copyParams.srcPtr   = make_cudaPitchedPtr(h_volume, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    copyParams.dstArray = d_volumeArray;
    copyParams.extent   = volumeSize;
    copyParams.kind     = cudaMemcpyHostToDevice;
    checkCudaErrors(cudaMemcpy3D(&copyParams));

    // set texture parameters
    tex_volume.normalized = false;                      // access with normalized texture coordinates
    tex_volume.filterMode = cudaFilterModeLinear;      // linear interpolation
	//tex.filterMode = cudaFilterModePoint;
	tex_volume.channelDesc = channelDesc;
    tex_volume.addressMode[0] = cudaAddressModeClamp;  // clamp texture coordinates
    tex_volume.addressMode[1] = cudaAddressModeClamp;
	tex_volume.addressMode[2] = cudaAddressModeClamp;

    // bind array to 3D texture
    checkCudaErrors(cudaBindTextureToArray(tex_volume, d_volumeArray, channelDesc));

}

extern "C"
void init_normal_Cuda(myvector4 *h_volume, cudaExtent volumeSize)
{
    // create 3D array
    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float4>();
    checkCudaErrors(cudaMalloc3DArray(&d_normalArray, &channelDesc, volumeSize,1));

    // copy data to 3D array
    cudaMemcpy3DParms copyParams = {0};
    copyParams.srcPtr   = make_cudaPitchedPtr(h_volume, volumeSize.width*4*sizeof(float), volumeSize.width, volumeSize.height);
    copyParams.dstArray = d_normalArray;
    copyParams.extent   = volumeSize;
    copyParams.kind     = cudaMemcpyHostToDevice;
    checkCudaErrors(cudaMemcpy3D(&copyParams));

    // set texture parameters
    tex_normal.normalized = false;                      // access with normalized texture coordinates
    tex_normal.filterMode = cudaFilterModeLinear;      // linear interpolation
	//tex.filterMode = cudaFilterModePoint;
	tex_normal.channelDesc = channelDesc;
    tex_normal.addressMode[0] = cudaAddressModeClamp;  // clamp texture coordinates
    tex_normal.addressMode[1] = cudaAddressModeClamp;
	tex_normal.addressMode[2] = cudaAddressModeClamp;

    // bind array to 3D texture
    checkCudaErrors(cudaBindTextureToArray(tex_normal, d_normalArray, channelDesc));

}

extern "C"
void TransferFunc(float *transf_color, int num_color, float *transf_alpha, int num_alpha)
{
	
	float4 *t_color = new float4[num_color];
	for(int i=0;i<num_color;i++)
	{
		t_color[i].x = transf_color[i*4];
		t_color[i].y = transf_color[i*4+1];
		t_color[i].z = transf_color[i*4+2];
		t_color[i].w = transf_color[i*4+3];
	}
	
    cudaChannelFormatDesc channelDesc2 = cudaCreateChannelDesc<float4>();
	
    checkCudaErrors(cudaMallocArray(&d_transferFuncArray1, &channelDesc2, num_color, 1));
	checkCudaErrors(cudaMemcpyToArray(d_transferFuncArray1, 0, 0, t_color, num_color*4*sizeof(float), cudaMemcpyHostToDevice));

	transferTex_color.filterMode = cudaFilterModeLinear;
    transferTex_color.normalized = true;    // access with normalized texture coordinates
    transferTex_color.addressMode[0] = cudaAddressModeClamp;   // wrap texture coordinates

    // Bind the array to the texture
    checkCudaErrors(cudaBindTextureToArray(transferTex_color, d_transferFuncArray1, channelDesc2));


	cudaChannelFormatDesc channelDesc3 = cudaCreateChannelDesc<float>();
	
    checkCudaErrors(cudaMallocArray(&d_transferFuncArray2, &channelDesc3, num_alpha, 1));
	checkCudaErrors(cudaMemcpyToArray(d_transferFuncArray2, 0, 0, transf_alpha, num_alpha*sizeof(float), cudaMemcpyHostToDevice));

	transferTex_alpha.filterMode = cudaFilterModeLinear;
    transferTex_alpha.normalized = true;    // access with normalized texture coordinates
    transferTex_alpha.addressMode[0] = cudaAddressModeClamp;   // wrap texture coordinates

    // Bind the array to the texture
    checkCudaErrors(cudaBindTextureToArray(transferTex_alpha, d_transferFuncArray2, channelDesc3));
	delete [] t_color;
}
extern "C"
void freeCudaVolumeBuffers()
{
    checkCudaErrors(cudaFreeArray(d_volumeArray));
	checkCudaErrors(cudaFreeArray(d_normalArray));
    
}

extern "C"
void freeCudaTransferFuncBuffers()
{    
    checkCudaErrors(cudaFreeArray(d_transferFuncArray1));
	checkCudaErrors(cudaFreeArray(d_transferFuncArray2));
}


extern "C"
void render_kernel(dim3 gridSize, dim3 blockSize, uint *d_output, uint imageW, uint imageH,
                   float density, float brightness, float transferOffset, float transferScale, 
				   cudaExtent volumeSize, float min_value, float max_value, float gridScale_X, float gridScale_Y, float gridScale_Z, float tstep)
				   
{
    d_render<<<gridSize, blockSize>>>(d_output, imageW, imageH, density,
                                      brightness, transferOffset, transferScale, volumeSize, min_value, max_value,
									  gridScale_X, gridScale_Y, gridScale_Z, tstep);
}

extern "C"
void copyInvViewMatrix(float *invViewMatrix, size_t sizeofMatrix)
{
    checkCudaErrors(cudaMemcpyToSymbol(c_invViewMatrix, invViewMatrix, sizeofMatrix));
}


extern "C"
void gmm_compute_kernel(dim3 gridSize, dim3 blockSize,int size, float *gmm_V,float *gmm_P, cudaExtent volumeSize, int xdim, int ydim, int zdim, int distib_num)
{

	unsigned int m_seed = rand();
	thrust::minstd_rand rng(size);
	rng.seed(m_seed);
	thrust::random::uniform_real_distribution<float> dist(0,1);
	unsigned int m_seed2 = rand();

	gmm_kernel<<<gridSize, blockSize>>>(dist,rng,gmm_V,gmm_P,xdim,ydim,zdim,distib_num,m_seed2);


#if 0
	float sum,uni_rand;
	bool flag;
	for(int i=0;i<size;i++)
	{
			sum=gmm_P[i].parameter[2]; //weighted of first gaussian 
		    flag=false;
			rng.discard(1);
			uni_rand = dist(rng);
			for(int j=0;j<gmm_P[i].mixtured_numer-1;j++)
			{
				if(uni_rand<sum)
				{
					gmm_V[i] = j;
					flag = true;
					break;
				}
				else
					sum += gmm_P[i].parameter[(j+2)*3-1];
			}
			if(!flag)
				gmm_V[i] = gmm_P[i].mixtured_numer-1;

			//g.data.volume[i] = CV->generate_gaussian_random(g.data.gmm[i].parameter[gmm_index[i]*3],g.data.gmm[i].parameter[gmm_index[i]*3+1]);
			
			unsigned int m_seed2 = rand();
			thrust::minstd_rand rng2;
			rng2.seed(m_seed2);
			thrust::random::experimental::normal_distribution<float> dist2(gmm_P[i].parameter[gmm_V[i]*3],gmm_P[i].parameter[gmm_V[i]*3+1]);
			gmm_V[i] = dist2(rng2);
	}

// copy data to 3D array
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc<float>();
    cudaMemcpy3DParms copyParams = {0};
    copyParams.srcPtr   = make_cudaPitchedPtr(gmm_V, volumeSize.width*sizeof(float), volumeSize.width, volumeSize.height);
    copyParams.dstArray = d_volumeArray;
    copyParams.extent   = volumeSize;
    copyParams.kind     = cudaMemcpyHostToDevice;
    checkCudaErrors(cudaMemcpy3D(&copyParams));

    // set texture parameters
    tex_volume.normalized = false;                      // access with normalized texture coordinates
    tex_volume.filterMode = cudaFilterModeLinear;      // linear interpolation
	//tex.filterMode = cudaFilterModePoint;
	tex_volume.channelDesc = channelDesc;
    tex_volume.addressMode[0] = cudaAddressModeClamp;  // clamp texture coordinates
    tex_volume.addressMode[1] = cudaAddressModeClamp;
	tex_volume.addressMode[2] = cudaAddressModeClamp;
	checkCudaErrors(cudaBindTextureToArray(tex_volume, d_volumeArray, channelDesc));
#endif
}
#endif // #ifndef _VOLUMERENDER_KERNEL_CU_
