/*=============================================================================
 *	This file is part of the "Synesthesia3D" graphics engine
 *	Copyright (C) 2014-2015 Iftode Bogdan-Marius <iftode.bogdan@gmail.com>
 *
 *		File:	ResourceManager.h
 *		Author:	Bogdan Iftode
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <http://www.gnu.org/licenses/>.
=============================================================================*/

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "ResourceData.h"

namespace Synesthesia3D
{
	class VertexFormat;
	class VertexBuffer;
	class IndexBuffer;
	class Texture;
	class ShaderProgram;
	class ShaderTemplate;
	class ShaderInput;
	class RenderTarget;

	// This class manages all allocated resources
	class ResourceManager
	{
	public:
				// Destroy all resources
				SYNESTHESIA3D_DLL	void ReleaseAll();
				// Create all platform specific resources
				SYNESTHESIA3D_DLL	void BindAll();
				// Destroy all platform specific resources
				SYNESTHESIA3D_DLL	void UnbindAll();

		// Support for deferred creation of a vertex format
		virtual	SYNESTHESIA3D_DLL	const unsigned int CreateVertexFormat(const unsigned int attributeCount) PURE_VIRTUAL
		// Create a vertex format
		virtual	SYNESTHESIA3D_DLL	const unsigned int CreateVertexFormat(const unsigned int attributeCount, const VertexAttributeUsage usage, const VertexAttributeType type, const unsigned int usageIdx, ...) PURE_VIRTUAL

		// Create an index buffer
		virtual	SYNESTHESIA3D_DLL const unsigned int CreateIndexBuffer(const unsigned int indexCount, const IndexBufferFormat indexFormat = IBF_INDEX16, const BufferUsage usage = BU_STATIC) PURE_VIRTUAL

		// Create a vertex buffer
		virtual	SYNESTHESIA3D_DLL const unsigned int CreateVertexBuffer(VertexFormat* const vertexFormat, const unsigned int vertexCount, IndexBuffer* const indexBuffer = nullptr, const BufferUsage usage = BU_STATIC) PURE_VIRTUAL

				// Create a shader input helper
				SYNESTHESIA3D_DLL const unsigned int CreateShaderInput(ShaderTemplate* const shaderTemplate);

		// Support for deferred creation of shader programs
		virtual	SYNESTHESIA3D_DLL const unsigned int CreateShaderProgram(const ShaderProgramType programType) PURE_VIRTUAL
				// Create a shader program
				SYNESTHESIA3D_DLL const unsigned int CreateShaderProgram(const char* filePath, const ShaderProgramType programType, char* const errors = nullptr, const char* entryPoint = "", const char* profile = "");

				// Create a shader template from a shader program
				// NB: TODO: shader templates will be extended in the future to encompass multiple passes / techniques
				SYNESTHESIA3D_DLL const unsigned int CreateShaderTemplate(ShaderProgram* const shaderProgram);

		// Create a texture
		virtual	SYNESTHESIA3D_DLL const unsigned int		CreateTexture(const PixelFormat pixelFormat, const TextureType texType, const unsigned int sizeX, const unsigned int sizeY = 1, const unsigned int sizeZ = 1, const unsigned int mipCount = 0, const BufferUsage usage = BU_TEXTURE) PURE_VIRTUAL
				// Create a texture and load data from an image file
				SYNESTHESIA3D_DLL const unsigned int		CreateTexture(const char* pathToFile);

		// Create a render target
		// NB: The widthRatio and heightRatio parameters are relative to the backbuffer's resolution, for dynamic render target sizes
		virtual	SYNESTHESIA3D_DLL const unsigned int		CreateRenderTarget(const unsigned int targetCount, PixelFormat pixelFormat, const unsigned int width, const unsigned int height, bool hasMipmaps, bool hasDepthStencil, PixelFormat depthStencilFormat) PURE_VIRTUAL
		virtual	SYNESTHESIA3D_DLL const unsigned int		CreateRenderTarget(const unsigned int targetCount, PixelFormat pixelFormat, const float widthRatio, const float heightRatio, bool hasMipmaps, bool hasDepthStencil, PixelFormat depthStencilFormat) PURE_VIRTUAL
		virtual	SYNESTHESIA3D_DLL const unsigned int		CreateRenderTarget(const unsigned int targetCount, PixelFormat pixelFormatRT0, PixelFormat pixelFormatRT1, PixelFormat pixelFormatRT2, PixelFormat pixelFormatRT3, const unsigned int width, const unsigned int height, bool hasMipmaps, bool hasDepthStencil, PixelFormat depthStencilFormat) PURE_VIRTUAL
		virtual	SYNESTHESIA3D_DLL const unsigned int		CreateRenderTarget(const unsigned int targetCount, PixelFormat pixelFormatRT0, PixelFormat pixelFormatRT1, PixelFormat pixelFormatRT2, PixelFormat pixelFormatRT3, const float widthRatio, const float heightRatio, bool hasMipmaps, bool hasDepthStencil, PixelFormat depthStencilFormat) PURE_VIRTUAL

				SYNESTHESIA3D_DLL	const unsigned int		CreateModel(const char* pathToFile);

				// Getters for various resource types
				SYNESTHESIA3D_DLL VertexFormat*	const	GetVertexFormat(const unsigned int idx)		const;
				SYNESTHESIA3D_DLL IndexBuffer*	const	GetIndexBuffer(const unsigned int idx)		const;
				SYNESTHESIA3D_DLL VertexBuffer*	const	GetVertexBuffer(const unsigned int idx)		const;
				SYNESTHESIA3D_DLL ShaderInput*	const	GetShaderInput(const unsigned int idx)		const;
				SYNESTHESIA3D_DLL ShaderProgram*	const	GetShaderProgram(const unsigned int idx)	const;
				SYNESTHESIA3D_DLL ShaderTemplate*	const	GetShaderTemplate(const unsigned idx)		const;
				SYNESTHESIA3D_DLL Texture*		const	GetTexture(const unsigned int idx)			const;
				SYNESTHESIA3D_DLL RenderTarget*	const	GetRenderTarget(const unsigned int idx)		const;
				SYNESTHESIA3D_DLL Model*			const	GetModel(const unsigned int idx)			const;

				SYNESTHESIA3D_DLL const unsigned int		GetVertexFormatCount()		const;
				SYNESTHESIA3D_DLL const unsigned int		GetIndexBufferCount()		const;
				SYNESTHESIA3D_DLL const unsigned int		GetVertexBufferCount()		const;
				SYNESTHESIA3D_DLL const unsigned int		GetShaderInputCount()		const;
				SYNESTHESIA3D_DLL const unsigned int		GetShaderProgramCount()		const;
				SYNESTHESIA3D_DLL const unsigned int		GetShaderTemplateCount()	const;
				SYNESTHESIA3D_DLL const unsigned int		GetTextureCount()			const;
				SYNESTHESIA3D_DLL const unsigned int		GetRenderTargetCount()		const;
				SYNESTHESIA3D_DLL const unsigned int		GetModelCount()				const;

				// Utility functions for finding a resource by its' original file name from which it was loaded
				// NB: If the strict parameter is set to false, it will first try to find an exact match, then try
				// to find a substring in the resource's original file path; default behaviour (i.e. strict = true)
				// is to only search for an exact match
				SYNESTHESIA3D_DLL const unsigned int		FindTexture(const char* pathToFile, const bool strict = true);
				SYNESTHESIA3D_DLL const unsigned int		FindModel(const char* pathToFile, const bool strict = true);

				// Destroy various resource types, freeing memory
				// NB: Destroying a high level resource also destroys the associated low level resources
				SYNESTHESIA3D_DLL		void				ReleaseVertexFormat(const unsigned int idx);
				SYNESTHESIA3D_DLL		void				ReleaseIndexBuffer(const unsigned int idx);
				SYNESTHESIA3D_DLL		void				ReleaseVertexBuffer(const unsigned int idx);
				SYNESTHESIA3D_DLL		void				ReleaseShaderInput(const unsigned int idx);
				SYNESTHESIA3D_DLL		void				ReleaseShaderProgram(const unsigned int idx);
				SYNESTHESIA3D_DLL		void				ReleaseShaderTemplate(const unsigned idx);
				SYNESTHESIA3D_DLL		void				ReleaseTexture(const unsigned int idx);
				SYNESTHESIA3D_DLL		void				ReleaseRenderTarget(const unsigned int idx);
				SYNESTHESIA3D_DLL		void				ReleaseModel(const unsigned int idx);
				
	protected:
		ResourceManager();
		virtual ~ResourceManager();

		// Low level resources
		std::vector<VertexFormat*>		m_arrVertexFormat;
		std::vector<IndexBuffer*>		m_arrIndexBuffer;
		std::vector<VertexBuffer*>		m_arrVertexBuffer;
		std::vector<ShaderInput*>		m_arrShaderInput;
		std::vector<ShaderProgram*>		m_arrShaderProgram;
		std::vector<ShaderTemplate*>	m_arrShaderTemplate;
		std::vector<Texture*>			m_arrTexture;
		std::vector<RenderTarget*>		m_arrRenderTarget;

		// High level resources (comprised of low level resources)
		std::vector<Model*>				m_arrModel;

		friend class Renderer;
	};
}

#endif // RESOURCEMANAGER_H