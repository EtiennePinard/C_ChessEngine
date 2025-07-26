#ifndef D5A082FB_118E_4F77_A831_0F85357C54A5
#define D5A082FB_118E_4F77_A831_0F85357C54A5

#include <stdbool.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "Scene.h"

/**
 * @brief Macro use for appending a value to a dynamic array.
 * The dynamic array struct needs to have these fields:
 * 
 * typedef struct da {
 *      type* data; // Pointer to the elements of the dynamic array
 *      size_t count; // The number of elements of the dynamic array
 *      size_t capacity; // The total amount of elements that the dynamic array can hold
 * } da;
 * 
 * If your dynamic array have this structure, you can use this macro as such:
 * 
 * da dynamic_array = { 0 };
 * type itemToAdd = ...;
 * da_append((&dynamic_array), itemToAdd);
 * 
 * In this example, the dynamic is 0 initialized and the da_append macro
 * will allocate an initial buffer and then add the item to the dynamic array.
 */
#define da_append(da, valueToAppend) if (da->count >= da->capacity) { \
    da->capacity++; \
    da->capacity *= 2; \
    da->data = SDL_realloc(da->data, sizeof(valueToAppend) * da->capacity); \
} \
da->data[da->count++] = valueToAppend; \

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
#define HOVERING_TEXTINPUT (-3)

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

typedef struct TextInputCursor {
    SDL_Cursor* sdlCursor;
    size_t index;
    Uint64 lastCursorToggleTime;
    bool showCursor;
} TextInputCursor;

typedef int TextInputId;

typedef struct TextInput {
    TextInputId textInputId;
    bool isActive;

    RenderBox textInputRender;

    EventCallback onEscape;
    EventCallback onReturn;
    EventCallback onKeyDown;
    EventCallback onTextInputEvent;

    bool keepOnlyAscii;
    Text_da text;
    Rect_da glyphRects;

    TextInputCursor cursor;

    size_t selectionStart;
    int nbCharSelected;

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
