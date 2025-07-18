#ifndef D5A082FB_118E_4F77_A831_0F85357C54A5
#define D5A082FB_118E_4F77_A831_0F85357C54A5

#include <stdbool.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "Scene.h"

/**
 * @brief Useful macro to convert a Rect to a FRect
 *
 */
#define RECT_TO_FRECT(rect) ((SDL_FRect) { .x = (float) rect.x, .y = (float) rect.y, .w = (float) rect.w, .h = (float) rect.h })

typedef struct SDL_State {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
} SDL_State;

typedef struct TextureState {
    SDL_Texture* texture;
    int width;
    int height;
} TextureState;

typedef struct Textures {
    TextureState* data;
    size_t count;
    size_t capacity;
} Textures;

#define da_append(da, valueToAppend) if (da->count >= da->capacity) { \
    da->capacity++; \
    da->capacity *= 2; \
    da->data = realloc(da->data, sizeof(valueToAppend) * da->capacity); \
} \
da->data[da->count++] = valueToAppend; \

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

typedef SDL_AppResult(*EventCallback)(SDL_Event*, App*);

#define HOVERING_NOTHING (-1)
#define HOVERING_MODAL (-2)

typedef struct MouseState {
    int hoveredIndex;
    bool holdingLeftMouseButton;
    bool holdingRightMouseButton;
    SDL_FPoint mousePoint;
} MouseState;

typedef int ModalId;

typedef struct Modal {
    ModalId modalId;
    bool isActive;
    bool canOnlyInteractWithModal;
    
    RenderBox modalRender;

    EventCallback onEscape;
    EventCallback onReturn;

    void* data;
} Modal;

typedef struct Text_da {
    char* data;
    size_t count;
    size_t capacity;
} Text_da;

typedef struct Rect_da {
    SDL_FRect* data;
    size_t count;
    size_t capacity;
} Rect_da;

typedef struct TextInput {
    bool isActive;
    
    EventCallback onEscape;
    EventCallback onReturn;

    Text_da text;
    bool keepOnlyAscii;

    SDL_Cursor* cursor;
    size_t cursorIndex;
    Uint64 lastCursorToggleTime;
    bool showCursor;

    size_t selectionStart;
    int nbCharSelected;    

    Rect_da glyphRects;
} TextInput;

typedef struct AppEvents {
    EventCallback onWindowResize;
    EventCallback onKeyDown;
    EventCallback onTextInput;

    Modal modal;
    TextInput textInput;

    MouseState mouseState;
    bool shouldHandleEvents;
} AppEvents;

/**
 * @brief Structs that holds the AppEvents and AppState struct
 *
 */
struct App {
    AppState state;
    AppEvents events;
    SDL_AppResult(*runAfterRenderAndEventsFunction)(App*);
};

#endif /* D5A082FB_118E_4F77_A831_0F85357C54A5 */
