//////////////////////////////////////////////////////////////////////////
// This file is part of the "LibRenderer" 3D graphics library           //
//                                                                      //
// Copyright (C) 2014 - Iftode Bogdan-Marius <iftode.bogdan@gmail.com>  //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program. If not, see <http://www.gnu.org/licenses/>. //
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "Renderer.h"
#include "Buffer.h"
using namespace LibRendererDll;

Buffer::Buffer(const unsigned int elementCount, const unsigned int elementSize, const BufferUsage usage)
	: m_nElementCount(elementCount)
	, m_nElementSize(elementSize)
	, m_eBufferUsage(usage)
	, m_nSize(elementCount * elementSize)
	, m_pData(nullptr)
{
	assert(elementCount >= 0);
	assert(elementSize >= 0);
	assert(usage >= 0 && usage < BU_MAX);

	if (elementCount > 0 && elementSize > 0)
	{
		m_pData = new byte[m_nSize];
		assert(m_pData != nullptr);
		memset(m_pData, 0, m_nSize);
	}
}

Buffer::~Buffer()
{
	assert(m_pData != nullptr || Renderer::GetAPI() == API_NULL);
	delete[] m_pData;
}

std::ostream& LibRendererDll::operator<<(std::ostream& output_out, Buffer& buf_in)
{
	output_out.write((const char*)&buf_in.m_nElementCount, sizeof(unsigned int));
	output_out.write((const char*)&buf_in.m_nElementSize, sizeof(unsigned int));
	output_out.write((const char*)&buf_in.m_eBufferUsage, sizeof(BufferUsage));
	output_out.write((const char*)&buf_in.m_nSize, sizeof(unsigned int));
	output_out.write((const char*)buf_in.m_pData, buf_in.m_nSize);

	return output_out;
}

std::istream& LibRendererDll::operator>>(std::istream& s_in, Buffer& buf_out)
{
	s_in.read((char*)&buf_out.m_nElementCount, sizeof(unsigned int));
	s_in.read((char*)&buf_out.m_nElementSize, sizeof(unsigned int));
	s_in.read((char*)&buf_out.m_eBufferUsage, sizeof(BufferUsage));
	s_in.read((char*)&buf_out.m_nSize, sizeof(unsigned int));
	delete[] buf_out.m_pData;
	buf_out.m_pData = new byte[buf_out.m_nSize];
	s_in.read((char*)buf_out.m_pData, buf_out.m_nSize);

	return s_in;
}