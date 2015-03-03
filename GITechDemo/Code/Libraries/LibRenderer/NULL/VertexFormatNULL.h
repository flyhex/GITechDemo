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
#ifndef VERTEXFORMATNULL_H
#define VERTEXFORMATNULL_H

#include "VertexFormat.h"

namespace LibRendererDll
{
	//This is the NULL implementation of the VertexFormat class
	class VertexFormatNULL : public VertexFormat
	{
	public:
		void	Enable();
		void	Disable();
		void	Update();

		void	Bind();
		void	Unbind();

	private:
		VertexFormatNULL(const unsigned int attributeCount);
		~VertexFormatNULL();

		friend class ResourceManagerNULL;
	};
}

#endif //VERTEXFORMATNULL_H
