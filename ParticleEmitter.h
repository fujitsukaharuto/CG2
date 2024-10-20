#pragma once
#include "ParticleManager.h"
#include "Particle.h"


class ParticleEmitter {
public:
	ParticleEmitter();
	~ParticleEmitter();

public:

	void Emit();

	void Burst();

	void BurstAnime();

	void RandomSpeed(const Vector2& x, const Vector2& y, const Vector2& z);
	void RandomTranslate(const Vector2& x, const Vector2& y, const Vector2& z);

	float frequencyTime = 0;

	std::string name;
	Vector3 pos;
	Particle grain{};
	AnimeData animeData{};
	uint32_t count;

private:



private:

	float time_;

	RandomParametor para_;

};
