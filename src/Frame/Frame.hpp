#ifndef FRAME_HPP
#define FRAME_HPP

#include <cstdint>
#include <memory>

#include <Color/Color.hpp>

class Frame {
private:
    uint16_t width, height;
    std::unique_ptr<Color[]> pixels;

public:
    Frame(uint16_t _width, uint16_t _height);

    inline const Color& getPixel(uint16_t x, uint16_t y) const { return this->pixels[this->width * y + x]; };
    inline Color& getPixel(uint16_t x, uint16_t y) { return this->pixels[this->width * y + x]; };

    inline std::unique_ptr<Color[]>& getPixels() { return this->pixels; };
};

#endif // FRAME_HPP