#include "anki/event/AnimationEvent.h"
#include "anki/resource/Animation.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/Movable.h"

namespace anki {

//==============================================================================
AnimationEvent::AnimationEvent(EventManager* manager, 
	const AnimationResourcePointer& anim_, SceneNode* movableSceneNode)
	:	Event(manager, 0.0, 0.0, movableSceneNode),
		anim(anim_)
{
	ANKI_ASSERT(movableSceneNode && movableSceneNode->getMovable());

	startTime = anim->getStartingTime();
	duration = anim->getDuration();
}

//==============================================================================
void AnimationEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	ANKI_ASSERT(getSceneNode());
	Movable* mov = getSceneNode()->getMovable();
	ANKI_ASSERT(mov);

	Vec3 pos;
	Quat rot;
	F32 scale = 1.0;
	anim->interpolate(0, crntTime, pos, rot, scale);

	Transform trf;
	trf.setOrigin(pos);
	trf.setRotation(Mat3(rot));
	trf.setScale(scale);
	mov->setLocalTransform(trf);
}

} // end namespace anki