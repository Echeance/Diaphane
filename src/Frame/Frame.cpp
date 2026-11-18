#include "Frame.hpp"

Frame::Frame(uint16_t _width, uint16_t _height) : width(_width), height(_height), pixels(new Color[this->width * this->height]) {};