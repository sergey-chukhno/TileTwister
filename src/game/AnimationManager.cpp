#include "AnimationManager.hpp"
#include <algorithm>

namespace Game {

void AnimationManager::addAnimation(const Animation &anim) {
  m_animations.push_back(anim);
}

void AnimationManager::update(float dt) {
  for (auto &anim : m_animations) {
    anim.timer += dt;
    if (anim.timer >= anim.duration) {
      anim.timer = anim.duration;
      anim.finished = true;
    }
  }

  // Remove finished animations
  m_animations.erase(
      std::remove_if(m_animations.begin(), m_animations.end(),
                     [](const Animation &a) { return a.finished; }),
      m_animations.end());
}

void AnimationManager::clear() { m_animations.clear(); }

const std::vector<Animation> &AnimationManager::getAnimations() const {
  return m_animations;
}

bool AnimationManager::isAnimating() const { return !m_animations.empty(); }

} // namespace Game
