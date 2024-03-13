#include "CoreFiles/Application.h"
#include "Core.h"
#include "CoreFiles/LogWrapper.h"
#include "CoreRender/Renderer.h"
#include "external/SDL/include/SDL3/SDL_keyboard.h"
#include "external/SDL/include/SDL3/SDL_timer.h"
#include "external/SDL/include/SDL3/SDL_video.h"
#include <filesystem>
#include <glm/fwd.hpp>

const unsigned int NUM_FLOATS_PER_VERTICE = 6;
const unsigned int VERTEX_SIZE = NUM_FLOATS_PER_VERTICE * sizeof(float);

namespace pain
{
/* Creates window, opengl context and init glew*/
Application::Application(const char *title, int w, int h) : m_maxFrameRate(60)
{
  // =========================================================================//
  // SDL Initial setup
  // =========================================================================//
  P_ASSERT(SDL_Init(SDL_INIT_VIDEO) >= 0, "SDL video could not be init {}",
           SDL_GetError());
  PLOG_T("SDL video is initialized");

  m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL);
  SDL_SetWindowResizable(m_window, SDL_TRUE);
  if (m_window == nullptr)
    PLOG_E("Application window not initialized");
  m_context = SDL_GL_CreateContext(m_window);

  // =========================================================================//
  // OpenGL Initial setup
  // =========================================================================//

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    PLOG_E("Error: {}", std::string(reinterpret_cast<const char *>(
                            glewGetErrorString(err))));
    exit(1);
  }
  PLOG_T("GLEW version: {}", std::string(reinterpret_cast<const char *>(
                                 glewGetString(GLEW_VERSION))));
  PLOG_T("GL version: {}",
         std::string(reinterpret_cast<const char *>(glGetString(GL_VERSION))));
  PLOG_T("Default relative path is: {}",
         std::filesystem::current_path().string());

  int versionMajor;
  int versionMinor;
  glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
  glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

  P_ASSERT(versionMajor >= 4 && versionMinor >= 3,
           "OpenGL version must be above 4.3, current version is {}.{}",
           versionMajor, versionMinor);

#ifndef NDEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(glErrorHandler, 0);
#endif

  // =========================================================================//
  // Application Initial setup
  // =========================================================================//

  m_isMinimized = false;
  m_layerStack = new pain::LayerStack();
}

Application::~Application()
{
  SDL_GL_DeleteContext(m_context);
  delete m_layerStack;
  SDL_DestroyWindow(m_window);
  SDL_Quit();
}

void Application::pushLayer(Layer *layer) { m_layerStack->pushLayer(layer); }

void Application::popLayer(Layer *layer) { m_layerStack->popLayer(layer); }

void Application::stop() { m_isGameRunning = false; }

void Application::handleEvents(const SDL_Event &event)
{
  // Handle each specific event
  // (1) Handle Input
  for (auto player = m_layerStack->end(); player != m_layerStack->begin();) {
    (*--player)->onEvent(event);
  }

  switch (event.type) {
  case SDL_WINDOWEVENT:
    if (event.window.event == SDL_WINDOWEVENT_MINIMIZED)
      m_isMinimized = true;
    else if (event.window.event == SDL_WINDOWEVENT_RESTORED)
      m_isMinimized = false;
    break;
  case SDL_QUIT:
    stop();
    break;
  case SDL_KEYDOWN:
    // PLOG_I("key pressed: {}", SDL_GetKeyName(event.key.keysym.sym));
    break;
  default:
    break;
  }
}

void Application::handleUpdate(DeltaTime deltaTime)
{
  for (auto pLayer = m_layerStack->end(); pLayer != m_layerStack->begin();) {
    (*--pLayer)->onUpdate(deltaTime);
  }
  SDL_GL_SwapWindow(m_window);
}

void Application::handleRender() {}

void Application::run()
{
  while (m_isGameRunning) {
    uint64_t start = SDL_GetPerformanceCounter();
    m_deltaTime = start - m_lastFrameTime;
    m_lastFrameTime = start;

    // unsigned int buttons;
    // buttons = SDL_GetMouseState(&m_mouseX, &m_mouseY);

    // handle events first
    // Start our event loop
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      handleEvents(event);
    }

    // if (elapsedTime < m_maxFrameRate) {
    //   SDL_Delay(m_maxFrameRate - elapsedTime);
    // }
    // handle any updates
    if (!m_isMinimized)
      handleUpdate(m_deltaTime);

    // hanlde our rendering
    handleRender();
  };
}

void Application::glErrorHandler(unsigned int source, unsigned int type,
                                 unsigned int id, unsigned int severity,
                                 int lenght, const char *message,
                                 const void *userParam)
{
  // uncomment this if your gpu being too educated with warnings
  // notification warnings are usually just optimizations anyway
  // if(severity == GL_DEBUG_SEVERITY_NOTIFICATION)
  //   return;

  PLOG_W("---------------");
  PLOG_W("Debug message ({}): {}", id, message);
  // clang-format off
    switch (source) 
    { 
        case GL_DEBUG_SOURCE_API:             PLOG_W(  "Source: API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   PLOG_W(  "Source: Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: PLOG_W(  "Source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     PLOG_W(  "Source: Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     PLOG_W(  "Source: Application"); break;
        case GL_DEBUG_SOURCE_OTHER:           PLOG_W(  "Source: Other"); break;
    } 
    switch (type) 
    { 
        case GL_DEBUG_TYPE_ERROR:               PLOG_E(  "Type: Error"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: PLOG_W(  "Type: Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  PLOG_W(  "Type: Undefined Behaviour"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         PLOG_W(  "Type: Portability"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         PLOG_W(  "Type: Performance"); break;
        case GL_DEBUG_TYPE_MARKER:              PLOG_W(  "Type: Marker"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          PLOG_W(  "Type: Push Group"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           PLOG_W(  "Type: Pop Group"); break;
        case GL_DEBUG_TYPE_OTHER:               PLOG_W(  "Type: Other"); break;
    } 
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         PLOG_E(  "Severity: high"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       PLOG_W(  "Severity: medium"); break;
        case GL_DEBUG_SEVERITY_LOW:          PLOG_W(  "Severity: low"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: PLOG_W(  "Severity: notification"); break;
    }
	if (GL_DEBUG_SEVERITY_HIGH == type || GL_DEBUG_TYPE_ERROR == type){
		P_ASSERT(false,"OpenGL critical error");
	}
  // clang-format on
}
} // namespace pain
