// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_HDR_H
#define ANKI_RENDERER_HDR_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gl.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/ProgramResource.h"
#include "anki/resource/Resource.h"
#include "anki/core/Timestamp.h"

namespace anki {

class ShaderProgram;

/// @addtogroup renderer
/// @{

/// High dynamic range lighting pass
class Hdr: public RenderingPass
{
	friend class Pps;

public:
	/// @name Accessors
	/// @{
	F32 getExposure() const
	{
		return m_exposure;
	}
	void setExposure(const F32 x)
	{
		m_exposure = x;
		m_parameterUpdateTimestamp = getGlobalTimestamp();
	}

	U32 getBlurringIterationsCount() const
	{
		return m_blurringIterationsCount;
	}
	void setBlurringIterationsCount(const U32 x)
	{
		m_blurringIterationsCount = x;
	}
	/// @}

	/// @privatesection
	/// @{
	GlTextureHandle& _getRt()
	{
		return m_vblurRt;
	}

	U32 _getWidth() const
	{
		return m_width;
	}

	U32 _getHeight() const
	{
		return m_height;
	}
	/// @}

private:
	U32 m_width, m_height;
	F32 m_exposure = 4.0; ///< How bright is the HDR
	U32 m_blurringIterationsCount = 2; ///< The blurring iterations
	F32 m_blurringDist = 1.0; ///< Distance in blurring
	
	GlFramebufferHandle m_hblurFb;
	GlFramebufferHandle m_vblurFb;

	ProgramResourcePointer m_toneFrag;
	ProgramResourcePointer m_hblurFrag;
	ProgramResourcePointer m_vblurFrag;

	GlPipelineHandle m_tonePpline;
	GlPipelineHandle m_hblurPpline;
	GlPipelineHandle m_vblurPpline;

	GlTextureHandle m_hblurRt; ///< pass0Fai with the horizontal blur FAI
	GlTextureHandle m_vblurRt; ///< The final FAI
	/// When a parameter changed by the setters
	Timestamp m_parameterUpdateTimestamp = 0;
	/// When the commonUbo got updated
	Timestamp m_commonUboUpdateTimestamp = 0;
	GlBufferHandle m_commonBuff;

	Hdr(Renderer* r)
	:	RenderingPass(r)
	{}

	~Hdr();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	ANKI_USE_RESULT Error run(GlCommandBufferHandle& jobs);

	ANKI_USE_RESULT Error initFb(GlFramebufferHandle& fb, GlTextureHandle& rt);
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	ANKI_USE_RESULT Error updateDefaultBlock(GlCommandBufferHandle& jobs);
};

/// @}

} // end namespace anki

#endif
