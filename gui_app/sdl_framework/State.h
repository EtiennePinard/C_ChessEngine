#ifndef D5A082FB_118E_4F77_A831_0F85357C54A5
#define D5A082FB_118E_4F77_A831_0F85357C54A5

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#define DEFAULT_TEXTURE_CAPACITY (16)

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
    da->data = realloc(da->data, sizeof(valueToAppend) * da->capacity * 2); \
    da->capacity *= 2; \
} \
da->data[da->count++] = valueToAppend; \

/**
 * @brief The AppState struct is declared here
 * but it is defined by the programmer using this
 * framework.
 * 
 */
typedef struct AppState AppState;

typedef struct App App;

typedef struct ClickableArea {
    SDL_Rect rect;
    void (*callback)(SDL_Event, App);
} ClickableArea;

typedef struct ClickableAreas {
    ClickableArea* data;
    size_t capacity;
} ClickableAreas;

typedef struct AppEvents {
    bool hasQuitEventHappened;
    ClickableAreas clickableAreas;
} AppEvents;

typedef struct Popup {
    SDL_Rect rect;
    // Returns true if the popup worked, else returns false
    bool (*callback)(SDL_Event, SDL_Rect, App);
} Popup;


struct App {
    AppState* state;
    AppEvents* events;
};

#endif /* D5A082FB_118E_4F77_A831_0F85357C54A5 */
