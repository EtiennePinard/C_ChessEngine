#ifndef CFB5E356_A54F_4398_A4F1_8533D1FD2F8D
#define CFB5E356_A54F_4398_A4F1_8533D1FD2F8D

#include "AppState.h"
#include "AppStyle.h"

/**
 * @brief Loads the game config from a file
 * 
 * @param data The pointer to fill with the loaded data or the 
 * default data if an error occurs.
 */
void loadGameConfig(GameConfig* data);

/**
 * @brief Saves the game config to a file
 * 
 * @param data The game config to save to a file
 * @return true If the game config was saved to a file
 * @return false If an error occurred
 */
bool saveGameConfig(const GameConfig* data);

void loadAppStyle(AppStyle* data);
bool saveAppStyle(const AppStyle* data);

#endif /* CFB5E356_A54F_4398_A4F1_8533D1FD2F8D */
