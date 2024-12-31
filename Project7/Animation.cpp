#include "Animation.h"

Animation::Animation(float d, CASkeleton* s){
	this->duration = d;
	this->skeleton = s;
}

Animation::~Animation(){
	delete this->skeleton;
}

void Animation::addKeyFrame(Keyframe kf){
	this->vKeyframe.push_back({
		kf.time, kf.direction
		});
}

void Animation::animation(float time){
	for (int i = 0; i < vKeyframe.size()-1; i++) {
		if (vKeyframe[i].time <= time && vKeyframe[i + 1].time >= time ){
			float t = (time - vKeyframe[i].time) / (vKeyframe[i+1].time - vKeyframe[i].time);
			std::vector<glm::vec2> ini = vKeyframe[i].direction;
			std::vector<glm::vec2> fin = vKeyframe[i+1].direction;

			for (int j = 0; j < vKeyframe[i].direction.size(); j++){
				glm::vec2 pos = ini[j] + t * (fin[j] - ini[j]);
				if (j == 0){
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
	int num = this->skeleton->getHijas().size();
	for (int i = 0; i < num; i++){
		CABalljoint* balljoint = this->skeleton->getHijas()[i];
		if (balljoint->getName() == name) {
			balljoint->setPose(vector.x, vector.y, 0.0f);
			break;
		}
		interpolationPose2(name, balljoint, vector);
	}
}

void Animation::interpolationPose2(std::string name, CABalljoint* balljoint, glm::vec2 vector){
	int num = balljoint->getHijas().size();
	for (int i = 0; i < num; i++) {
		CABalljoint* articulacion = balljoint->getHijas()[i];
		if (articulacion->getName() == name) {
			articulacion->setPose(vector.x, vector.y, 0.0f);
			return;
		}
		interpolationPose2(name, articulacion, vector);
	}
}

void Animation::createAnimation(){
	Keyframe kf = {};

	kf.direction = {
		glm::vec2(0.0f, 0.0f), //leg_l
		glm::vec2(0.0f, 0.0f), //leg_r
		glm::vec2(0.0f, 0.0f), //knee_l
		glm::vec2(0.0f, 0.0f) //knee_r
	};

	kf.time = 0.0f;

	this->addKeyFrame(kf);

	kf.direction = {
		glm::vec2(0.0f, 0.0f), //leg_l
		glm::vec2(-90.0f, 0.0f), //leg_r
		glm::vec2(0.0f, 0.0f), //knee_l
		glm::vec2(90.0f, 0.0f) //knee_r
	};

	kf.time = 0.7f;

	this->addKeyFrame(kf);

	kf.direction = {
		glm::vec2(30.0f, 70.0f), //leg_l
		glm::vec2(-90.0f, 0.0f), //leg_r
		glm::vec2(90.0f, 0.0f), //knee_l
		glm::vec2(0.0f, 0.0f) //knee_r
	};

	kf.time = 1.4f;

	this->addKeyFrame(kf);

	kf.direction = {
		glm::vec2(30.0f, 90.0f), //leg_l
		glm::vec2(-70.0f, 0.0f), //leg_r
		glm::vec2(90.0f, 0.0f), //knee_l
		glm::vec2(0.0f, 0.0f) //knee_r
	};

	kf.time = 2.1f;

	this->addKeyFrame(kf);

	kf.direction = {
		glm::vec2(40.0f, 40.0f), //leg_l
		glm::vec2(-50.0f, 0.0f), //leg_r
		glm::vec2(70.0f, 0.0f), //knee_l
		glm::vec2(30.0f, 0.0f) //knee_r
	};

	kf.time = 2.8f;

	this->addKeyFrame(kf);

	kf.direction = {
		glm::vec2(0.0f, 0.0f), //leg_l
		glm::vec2(-30.0f, 0.0f), //leg_r
		glm::vec2(70.0f, 0.0f), //knee_l
		glm::vec2(15.0f, 0.0f) //knee_r
	};

	kf.time = 3.5f;

	this->addKeyFrame(kf);

	kf.direction = {
		glm::vec2(-70.0f, 0.0f), //leg_l
		glm::vec2(-30.0f, 0.0f), //leg_r
		glm::vec2(40.0f, 0.0f), //knee_l
		glm::vec2(15.0f, 0.0f) //knee_r
	};

	kf.time = 4.2f;

	this->addKeyFrame(kf);

	kf.direction = {
		glm::vec2(-50.0f, 0.0f), //leg_l
		glm::vec2(0.0f, 0.0f), //leg_r
		glm::vec2(40.0f, 0.0f), //knee_l
		glm::vec2(0.0f, 0.0f) //knee_r
	};

	kf.time = 4.9f;

	this->addKeyFrame(kf);

	kf.direction = {
		glm::vec2(0.0f, 0.0f), //leg_l
		glm::vec2(0.0f, 0.0f), //leg_r
		glm::vec2(0.0f, 0.0f), //knee_l
		glm::vec2(0.0f, 0.0f) //knee_r
	};

	kf.time = 5.6f;

	this->addKeyFrame(kf);
}


