//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "MeshLoaderTerrain.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>

MeshLoaderStream::MeshLoaderStream() :
	m_scale(1.0f),
	m_verts(0),
	m_tris(0),
	m_normals(0),
	m_vertCount(0),
	m_triCount(0)
{
}

MeshLoaderStream::~MeshLoaderStream()
{
	delete [] m_verts;
	delete [] m_normals;
	delete [] m_tris;
}
		
void MeshLoaderStream::addVertex(float x, float y, float z, int& cap)
{
	if (m_vertCount+1 > cap)
	{
		cap = !cap ? 8 : cap*2;
		float* nv = new float[cap*3];
		if (m_vertCount)
			memcpy(nv, m_verts, m_vertCount*3*sizeof(float));
		delete [] m_verts;
		m_verts = nv;
	}
	float* dst = &m_verts[m_vertCount*3];
	*dst++ = x*m_scale;
	*dst++ = y*m_scale;
	*dst++ = z*m_scale;
	m_vertCount++;
}

void MeshLoaderStream::addTriangle(int a, int b, int c, int& cap)
{
	if (m_triCount+1 > cap)
	{
		cap = !cap ? 8 : cap*2;
		int* nv = new int[cap*3];
		if (m_triCount)
			memcpy(nv, m_tris, m_triCount*3*sizeof(int));
		delete [] m_tris;
		m_tris = nv;
	}
	int* dst = &m_tris[m_triCount*3];
	*dst++ = a;
	*dst++ = b;
	*dst++ = c;
	m_triCount++;
}

static char* parseRow(char* buf, char* bufEnd, char* row, int len)
{
	bool cont = false;
	bool start = true;
	bool done = false;
	int n = 0;
	while (!done && buf < bufEnd)
	{
		char c = *buf;
		buf++;
		// multirow
		switch (c)
		{
			case '\\':
				cont = true; // multirow
				break;
			case '\n':
				if (start) break;
				done = true;
				break;
			case '\r':
				break;
			case '\t':
			case ' ':
				if (start) break;
			default:
				start = false;
				cont = false;
				row[n++] = c;
				if (n >= len-1)
					done = true;
				break;
		}
	}
	row[n] = '\0';
	return buf;
}

static int parseFace(char* row, int* data, int n, int vcnt)
{
	int j = 0;
	while (*row != '\0')
	{
		// Skip initial white space
		while (*row != '\0' && (*row == ' ' || *row == '\t'))
			row++;
		char* s = row;
		// Find vertex delimiter and terminated the string there for conversion.
		while (*row != '\0' && *row != ' ' && *row != '\t')
		{
			if (*row == '/') *row = '\0';
			row++;
		}
		if (*s == '\0')
			continue;
		int vi = atoi(s);
		data[j++] = vi < 0 ? vi+vcnt : vi-1;
		if (j >= n) return j;
	}
	return j;
}

bool MeshLoaderStream::load(const float* vertstream, unsigned int count)
{
	if (0 != count % (3 * 3))
		return false;

	m_verts = new float[count];
	m_vertCount = int(count / 3);
	memcpy(m_verts, vertstream, count * sizeof(float));

	m_tris = new int[m_vertCount];
	m_triCount = m_vertCount / 3;
	for (int i = 0; i < m_triCount; ++i)
	{
		const int idx = i * 3;
		int* const pTri = m_tris + idx;
		*pTri = idx;
		*(pTri + 1) = idx + 1;
		*(pTri + 2) = idx + 2;
	}

	// Calculate normals.
	m_normals = new float[m_triCount*3];
	for (int i = 0; i < m_triCount*3; i += 3)
	{
		const float* v0 = &m_verts[m_tris[i]*3];
		const float* v1 = &m_verts[m_tris[i+1]*3];
		const float* v2 = &m_verts[m_tris[i+2]*3];
		float e0[3], e1[3];
		for (int j = 0; j < 3; ++j)
		{
			e0[j] = v1[j] - v0[j];
			e1[j] = v2[j] - v0[j];
		}
		float* n = &m_normals[i];
		n[0] = e0[1]*e1[2] - e0[2]*e1[1];
		n[1] = e0[2]*e1[0] - e0[0]*e1[2];
		n[2] = e0[0]*e1[1] - e0[1]*e1[0];
		float d = sqrtf(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		if (d > 0)
		{
			d = 1.0f/d;
			n[0] *= d;
			n[1] *= d;
			n[2] *= d;
		}
	}

	return true;
}

bool MeshLoaderStream::Save( const char* szPathFile )
{

	InputHeader header;
	header.indexStart = sizeof(InputHeader);
	header.indexCount = m_triCount*3;
	header.vertsStart = header.indexStart+header.indexCount*sizeof(int);
	header.vertsCount = m_vertCount;

	std::ofstream outfile(szPathFile, std::ios::binary|std::ios::out);
	outfile.write((const char*)&header, sizeof(InputHeader));
	outfile.write((const char*)m_tris, sizeof(int)*header.indexCount);
	outfile.write((const char*)m_verts, sizeof(float)*3*header.vertsCount);
	return true;
}

bool MeshLoaderStream::loadfromnavinput( const char* pathfile )
{
	std::ifstream outfile(pathfile, std::ios::binary|std::ios::in);

	InputHeader header;
	outfile.read((char*)&header,  sizeof(InputHeader));

	m_triCount = header.indexCount/3;
	m_tris = new int[header.indexCount];
	outfile.seekg(header.indexStart, std::ios::beg);
	outfile.read((char*)m_tris, header.indexCount*sizeof(int));

	m_vertCount = header.vertsCount;
	m_verts = new float[m_vertCount*3];
	outfile.seekg(header.vertsStart, std::ios::beg);
	outfile.read((char*)m_verts, sizeof(float)*3*header.vertsCount);
	return true;
}