#include "mario/ui/HUD.hpp"

namespace mario {

void HUD::set_lives(int lives) { (void)lives; }
void HUD::set_coins(int coins) { (void)coins; }
void HUD::set_score(int score) { (void)score; }
void HUD::set_timer(int seconds) { (void)seconds; }
void HUD::render() {}

void Text::set_string(std::string_view text) { (void)text; }
void Text::render() {}

} // namespace mario
