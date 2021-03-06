/*
    The MIT License (MIT)

    Copyright (c) 2014 Anatoli Steinmark

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#ifndef RPGSS_AUDIO_SOUND_HPP_INCLUDED
#define RPGSS_AUDIO_SOUND_HPP_INCLUDED

#include "../common/RefCountedObject.hpp"
#include "../common/RefCountedObjectPtr.hpp"


namespace rpgss {
    namespace audio {

        class Sound : public RefCountedObject {
        public:
            typedef RefCountedObjectPtr<Sound> Ptr;

        public:
            virtual void play() = 0;
            virtual void stop() = 0;
            virtual void reset() = 0;
            virtual bool isPlaying() const = 0;
            virtual bool isSeekable() const = 0;
            virtual int  getLength() const = 0;
            virtual bool getRepeat() const = 0;
            virtual void setRepeat(bool repeat) = 0;
            virtual float getPitch() const = 0;
            virtual void setPitch(float pitch) = 0;
            virtual float getVolume() const = 0;
            virtual void setVolume(float volume) = 0;
            virtual int  getPosition() const = 0;
            virtual void setPosition(int position) = 0;

        protected:
            virtual ~Sound() { }
        };

    } // namespace audio
} // namespace rpgss


#endif // RPGSS_AUDIO_SOUND_HPP_INCLUDED
