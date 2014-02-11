#ifndef RPGSS_GRAPHICS_GRAPHICS_HPP_INCLUDED
#define RPGSS_GRAPHICS_GRAPHICS_HPP_INCLUDED

#include "../io/Stream.hpp"
#include "Image.hpp"


namespace rpgss {
    namespace graphics {

        bool InitGraphicsSubsystem();
        void DeinitGraphicsSubsystem();

        Image::Ptr ReadImage(const std::string& filename);
        Image::Ptr ReadImage(io::Stream* istream);

        bool WriteImage(Image* image, const std::string& filename);
        bool WriteImage(Image* image, io::Stream* ostream);

    } // namespace graphics
} // namespace rpgss


#endif // RPGSS_GRAPHICS_GRAPHICS_HPP_INCLUDED