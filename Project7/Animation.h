#pragma once
#include "CASkeleton.h"

struct Keyframe {
	float time;
	std::vector<glm::vec2> direction;
};

class Animation{
	private:
		float duration;
		CASkeleton* skeleton;
		std::vector<Keyframe> vKeyframe;

	public:
		Animation(float d, CASkeleton* s);
		~Animation();
		void addKeyFrame(Keyframe kf);
		void animation(float time);
		void interpolationPose(std::string name, glm::vec2 vector);
		void interpolationPose2(std::string name, CABalljoint* balljoint, glm::vec2 vector);
		void createAnimation();
};

