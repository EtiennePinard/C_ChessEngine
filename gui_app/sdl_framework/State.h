#ifndef D5A082FB_118E_4F77_A831_0F85357C54A5
#define D5A082FB_118E_4F77_A831_0F85357C54A5

#include <stdbool.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

/**
 * @brief Useful macro to convert a Rect to a FRect
 * 
 */
#define RECT_TO_FRECT(rect) ((SDL_FRect) { .x = (float) rect.x, .y = (float) rect.y, .w = (float) rect.w, .h = (float) rect.h })

typedef struct App App;

typedef int SceneId;

typedef struct SDL_State {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
} SDL_State;

typedef struct TextureState {
    SDL_Texture *texture;
    int width;
    int height;
} TextureState;

typedef struct Textures {
    TextureState *data;
    size_t count;
    size_t capacity;
} Textures;

#define da_append(da, valueToAppend) if (da->count >= da->capacity) { \
    da->capacity++; \
    da->capacity *= 2; \
    da->data = realloc(da->data, sizeof(valueToAppend) * da->capacity); \
} \
da->data[da->count++] = valueToAppend; \

typedef SDL_AppResult (*RenderFunction)(SDL_Rect, App*);
typedef SDL_AppResult (*EventCallbackBox)(SDL_Event*, SDL_Rect, App*);

typedef struct RenderBox {
    SDL_Rect renderRect;
    RenderFunction renderFunction;
    EventCallbackBox onMouseButtonDown;
    EventCallbackBox onMouseButtonUp;
    EventCallbackBox onMouseEntered;
    EventCallbackBox onMouseHovered;
    EventCallbackBox onMouseExited;
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

typedef struct Scene {
    SceneId sceneId;
    SceneRender sceneRender;
    SDL_AtomicInt shouldRender;
    int selectedRenderBoxIndex;
    void* data;
} Scene;

/**
 * @brief The design philosophy of this AppState is to be able
 * to have one scene active and when we switch scene then the next
 * scene will be initialized. At app startup the first scene is
 * initialized and then so on and so forth while the user clicks or
 * inputs key shortcuts.
 *
 */
typedef struct AppState {
    SDL_State sdlState;
    Scene currentScene;
} AppState;

typedef SDL_AppResult (*EventCallback)(App*, SDL_Event*);

typedef struct MouseState {
    int hoveredIndex; // -1 if not hovering anything
} MouseState;

typedef struct AppEvents {
    EventCallback onWindowResize;
    MouseState mouseState;
    bool shouldHandleEvents;
    bool lockSelectedBoxIndex;
} AppEvents;

/**
 * @brief Structs that holds a pointer to the AppEvents and AppState struct
 * 
 */
struct App {
    AppState state;
    AppEvents events;
    SDL_AppResult (*runAfterRenderAndEventsFunction)(App*);
};

#endif /* D5A082FB_118E_4F77_A831_0F85357C54A5 */
