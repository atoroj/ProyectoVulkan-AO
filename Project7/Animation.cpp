#include "Animation.h"

Animation::Animation(float d, CASkeleton* s){
	this->duration = d;
	this->skeleton = s;
}

Animation::~Animation(){
	delete this->skeleton;
}

void Animation::addKeyFrame(Keyframe kf){
	this->vKeyframe.push_back(kf);
}

void Animation::animation(float time){
	for (int i = 0; i < vKeyframe.size()-1; i++) {
		if (vKeyframe[i].time <= time && vKeyframe[i + 1].time >= time ){
			float t = (time - vKeyframe[i].time) / (vKeyframe[i+1].time - vKeyframe[i].time);
			std::vector<glm::vec2> ini = vKeyframe[i].direction;
			std::vector<glm::vec2> fin = vKeyframe[i+1].direction;

			for (int j = 0; j < vKeyframe[i].direction.size(); j++){
				glm::vec2 pos = ini[j] + t * (fin[j] - ini[j]);
				if (j == 0) {
					interpolationPose("leg_l", pos);
				}
				else if (j == 1){
					interpolationPose("leg_r", pos);
				}
				else if (j == 2){
					interpolationPose("knee_l", pos);
				}
				else {
					interpolationPose("knee_r", pos);
				}
			}
		}
	}
}
void Animation::interpolationPose(std::string name, glm::vec2 vector){
	std::vector<CABalljoint*> articulaciones = this->skeleton->getHijas();
	for (int i = 0; i < articulaciones.size(); i++){
		if (articulaciones[i]->getName() == name) {
			articulaciones[i]->setPose(vector.x, vector.y, 0.0f);
			break;
		}
		interpolationPose2(name, articulaciones[i], vector);
	}
}

void Animation::interpolationPose2(std::string name, CABalljoint* balljoint, glm::vec2 vector){
	std::vector<CABalljoint*> articulaciones = balljoint->getHijas();
	for (int i = 0; i < articulaciones.size(); i++) {
		if (articulaciones[i]->getName() == name) {
			articulaciones[i]->setPose(vector.x, vector.y, 0.0f);
		}
		interpolationPose2(name, articulaciones[i], vector);
	}
}

void Animation::createAnimation(){
	Keyframe kf;

	std::vector<glm::vec2>  direction = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(90.0f, 0.0f),
		glm::vec2(0.0f, 0.0f),
		glm::vec2(-90.0f, 0.0f)
	};

	kf.direction = direction;
	kf.time = 0.7f;

	this->addKeyFrame(kf);
}


