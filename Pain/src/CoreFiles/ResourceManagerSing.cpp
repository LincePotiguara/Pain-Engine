#include "ResourceManagerSing.h"
#include <SDL2/SDL_surface.h>
#include <utility>

namespace pain
{
Resources *Resources::getInstance()
{
  if (m_instance == nullptr) {
    m_instance = new Resources();
  }
  return m_instance;
}
SDL_Surface *Resources::getSurface(const std::string filepath)
{
  auto search = m_surfacesMap.find(filepath);
  if (search != m_surfacesMap.end()) {
    return m_surfacesMap[filepath];
  } else {
    SDL_Surface *surface = SDL_LoadBMP(filepath.c_str());
    m_surfacesMap.insert(std::make_pair(filepath, surface));
    return m_surfacesMap[filepath];
  }
}

} // namespace pain