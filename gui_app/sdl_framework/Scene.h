#include <SDL3/SDL.h>

typedef struct App App;

typedef SDL_AppResult(*RenderFunction)(SDL_Rect, App*);
typedef SDL_AppResult(*EventCallbackBox)(SDL_Event*, SDL_Rect, App*);

typedef struct RenderBox {
    SDL_Rect renderRect;
    RenderFunction renderFunction;
    EventCallbackBox onMouseButtonDown;
    EventCallbackBox onMouseButtonUp;
    EventCallbackBox onMouseEntered;
    EventCallbackBox onMouseHovered;
    EventCallbackBox onMouseExited;
    EventCallbackBox onMouseWheelScrolled;
} RenderBox;

typedef struct SceneRender {
    RenderBox* renderBoxes;
    size_t numRenderBox;

    SDL_Color renderDrawColor;
} SceneRender;

typedef enum RerenderValue {
    NO_RERENDER = 0,
    MAIN_THREAD_RERENDER = 1,
    OTHER_THREAD_RERENDER = 2
} RerenderValue;

typedef int SceneId;

typedef void(*TerminateScene)(void* data);

typedef struct Scene {
    SceneId sceneId;
    SceneRender sceneRender;
    SDL_AtomicInt shouldRender;
    int selectedRenderBoxIndex;
    void* data;

    TerminateScene terminateSceneFunction;
} Scene;
