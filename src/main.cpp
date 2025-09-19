#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <random>

using namespace geode::prelude;

std::vector<float> points;

std::random_device rd;
std::mt19937 gen(rd());

bool enable = false;	// if the mod works
bool setup = false;		// if the mod was set up
bool first = false;		// play next song from 0
bool chaos = false;		// randomly pick a point in the song

void randomizeMusicPos() {
	// Ignore on startup
	if (first) {
		first = false;
		return;
	}

	auto fmod = FMODAudioEngine::sharedEngine();

	// Chaos random
	if (chaos) {
		float dur = fmod->getMusicLengthMS(0);
		if (dur > 30000) dur -= 5000; // don't allow the last 5 seconds for longer songs, seems fair enough
		std::uniform_real_distribution<> dist(0.0, dur);
		fmod->setMusicTimeMS(dist(gen), false, 0);
	}

	// Only one point
	else if (points.size() == 1) {
		fmod->setMusicTimeMS(points[0] * 1000, false, 0);
	}

	// Random from points
	else {
		std::uniform_int_distribution<> dist(0, points.size() - 1);
		fmod->setMusicTimeMS(points[dist(gen)] * 1000, false, 0);
	}
}

class $modify(GameManager) {
	void fadeInMenuMusic() {
		auto oldTrack = FMODAudioEngine::sharedEngine()->getActiveMusic(0);
		GameManager::fadeInMenuMusic();
		if (enable && oldTrack != FMODAudioEngine::sharedEngine()->getActiveMusic(0)) {
			randomizeMusicPos();
		}
	}
};

void reloadPoints() {
	enable = Mod::get()->getSettingValue<bool>("enable");
	chaos = Mod::get()->getSettingValue<bool>("chaos");
	points.clear();

	if (!enable || chaos) return;

	// Base start time
	points.push_back(Mod::get()->getSettingValue<float>("pt0"));

	// Random points
	for (int i = 1; i <= 16; i++) {
		float val = Mod::get()->getSettingValue<float>("pt" + std::to_string(i));
		if (val > 0) points.push_back(val);
	}
}

static void onSetupComplete() {
	setup = false;
	reloadPoints();
}

$on_mod(Loaded) {
	reloadPoints();
	if (Mod::get()->getSettingValue<bool>("noFirst")) first = true;

	// this runs multiple times for each setting changed, we only need to do it once
	geode::listenForAllSettingChanges([](auto) {
		if (!setup) {
			setup = true;
			geode::queueInMainThread(onSetupComplete);
		}
	});
}