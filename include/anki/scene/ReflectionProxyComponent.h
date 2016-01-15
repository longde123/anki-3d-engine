// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneComponent.h>
#include <anki/collision/Plane.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Reflection proxy component.
class ReflectionProxyComponent : public SceneComponent
{
public:
	/// Reflection proxy face. One out of many
	class Face
	{
	public:
		Array<Vec4, 4> m_vertices;
		Plane m_plane;
	};

	ReflectionProxyComponent(SceneNode* node, U faceCount)
		: SceneComponent(SceneComponent::Type::REFLECTION_PROXY, node)
	{
		ANKI_ASSERT(faceCount > 0);
		m_faces.create(getAllocator(), faceCount);
	}

	~ReflectionProxyComponent()
	{
		m_faces.destroy(getAllocator());
	}

	static Bool classof(const SceneComponent& c)
	{
		return c.getType() == Type::REFLECTION_PROXY;
	}

	void setQuad(
		U index, const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d);

	const DArray<Face>& getFaces() const
	{
		ANKI_ASSERT(m_faces.getSize() > 0);
		return m_faces;
	}

	ANKI_USE_RESULT Error update(
		SceneNode& node, F32 prevTime, F32 crntTime, Bool& updated) final;

private:
	DArray<Face> m_faces; ///< Quads.
	Bool8 m_dirty = true;
};
/// @}

} // end namespace anki