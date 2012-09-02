/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus's Graphical User Interface
// Copyright (C) 2012 Bruno Van de Velde (VDV_B@hotmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _TGUI_ANIMATED_PICTURE_INCLUDED_
#define _TGUI_ANIMATED_PICTURE_INCLUDED_

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Load multiple images and add then behind each other to create a simple animation or even a movie.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct TGUI_API AnimatedPicture : public OBJECT, OBJECT_ANIMATION
    {
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Default constructor
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        AnimatedPicture();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Copy constructor
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        AnimatedPicture(const AnimatedPicture& copy);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Destructor
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual ~AnimatedPicture();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Overload of assignment operator
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        AnimatedPicture& operator= (const AnimatedPicture& right);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual AnimatedPicture* clone();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Load another image/frame from a file.
        ///
        /// All frames should have the same size.
        /// The size of the first frame is always used in the calculations.
        ///
        /// \param filename       The filename of the image that you want to use as next frame.
        /// \param frameDuration  The amount of time that the frame will be displayed on the screen.
        ///                       When the duration is 0 (default) then the animation will be blocked at that frame.
        ///
        /// \return
        ///        - 0 when the filename is empty
        ///        - 0 when the image couldn't be loaded (probalby not found)
        ///        - The frame number on success (the first frame has number 1)
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual unsigned int addFrame(const std::string filename, const sf::Time frameDurarion = sf::Time());


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Changes the size of the displayed image.
        ///
        /// This function will undo all scaling, as it just calculates the correct scale factors for you.
        ///
        /// All frames should have the same size.
        /// The scaling factors used to get the first frame to the correct size, will be used for the other frames.
        ///
        /// This function can only be called after the first frame is loaded.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setSize(float width, float height);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Returns the size of the first frame, unaffected by scaling.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual Vector2u getSize() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Returns the size of the first frame, after the scaling tranformations.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual Vector2f getScaledSize() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Starts or resumes playing the animation.
        ///
        /// \see pause
        /// \see stop
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void play();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Pauses the animation.
        ///
        /// You can continue the animation with the start() function.
        ///
        /// \see play
        /// \see stop
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void pause();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Stops the animation.
        ///
        /// When calling start() after calling this function, the animation will restart from the first frame.
        ///
        /// \see play
        /// \see pause
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void stop();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Sets the animation at a specific frame.
        ///
        /// When there are less frames than \a frame, the last frame will be displayed.
        /// Both 0 and 1 will set the first frame.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setFrame(const unsigned int frame);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Returns the current displayed frame.
        ///
        /// This function will return 0 when no frames have been loaded.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual unsigned int getCurrentFrame() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Changes the amount of time that the frame will be displayed on the screen.
        ///
        /// \param frame  The frame that is going to have the given duration.
        ///               Both 0 and 1 will affect the first frame.
        ///               When \a frame is bigger than the amount of frames then the last frame will be changed.
        ///
        /// \param frameDuration  The amount of time that the frame will be displayed on the screen.
        ///                       When the duration is 0 (default) then the animation will be blocked at that frame.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setFrameDuration(const unsigned int frame, const sf::Time frameDuration = sf::Time());


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Returns the duration of the currently displayed frame.
        ///
        /// This function will return 0 when no frames have been loaded.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual sf::Time getCurrentFrameDuration() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Returns the amount of frames in the animation.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual unsigned int getFrames() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Returns the filenames that were used to load the pictures.
        ///
        /// When no picture has not been loaded yet then this function will return an empty vector.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual std::vector<std::string> getLoadedFilenames() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Removes a frame from the animation.
        ///
        /// Both 0 and 1 will remove the first frame.
        /// When \a frame is bigger than the amount of frames then the last frame will be removed.
        ///
        /// \see removeAllFrames
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void removeFrame(const unsigned int frame);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// \brief Remove all frames from the animation.
        ///
        /// \see removeFrame
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void removeAllFrames();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // When the elapsed time changes then this function is called.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void update();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Because this struct is derived from sf::Drawable, you can just call the draw function from your sf::RenderTarget.
        // This function will be called and it will draw the picture on the render target.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    public:

        bool loop;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:


        std::vector<sf::Texture*> m_Textures;
        std::vector<sf::Sprite>   m_Sprites;
        std::vector<std::string>  m_LoadedFilenames;
        std::vector<sf::Time>     m_FrameDuration;

        unsigned int m_CurrentFrame;

        bool m_Playing;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_TGUI_ANIMATED_PICTURE_INCLUDED_
