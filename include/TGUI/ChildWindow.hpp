/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus's Graphical User Interface
// Copyright (C) 2012-2014 Bruno Van de Velde (vdv_b@tgui.eu)
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


#ifndef TGUI_CHILD_WINDOW_HPP
#define TGUI_CHILD_WINDOW_HPP


#include <TGUI/Container.hpp>
#include <TGUI/Button.hpp>
#include <TGUI/Label.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    class ChildWindowRenderer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Movable Panel with title bar.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class TGUI_API ChildWindow : public Container
    {
    public:

        typedef std::shared_ptr<ChildWindow> Ptr;


        /// Title alignments, possible options for the setTitleAlignment function
        enum TitleAlignment
        {
            /// Places the title on the left side of the title bar
            TitleAlignmentLeft,

            /// Places the title in the middle of the title bar
            TitleAlignmentCentered,

            /// Places the title on the right side of the title bar
            TitleAlignmentRight
        };


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Default constructor
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ChildWindow();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Virtual destructor
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual ~ChildWindow() {}


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Creates the child window
        ///
        /// @param themeFileFilename  Filename of the theme file.
        /// @param section            The section in the theme file to read.
        ///
        /// @throw Exception when the theme file could not be opened.
        /// @throw Exception when the theme file did not contain the requested section with the needed information.
        /// @throw Exception when one of the images, described in the theme file, could not be loaded.
        ///
        /// When an empty string is passed as filename, the built-in white theme will be used.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        static ChildWindow::Ptr create(const std::string& themeFileFilename = "", const std::string& section = "ChildWindow");


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Makes a copy of another child window
        ///
        /// @param childWindow  The other child window
        ///
        /// @return The new child window
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        static ChildWindow::Ptr copy(const ChildWindow::Ptr& childWindow)
        {
            return std::make_shared<ChildWindow>(*childWindow);
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the renderer, which gives access to functions that determine how the widget is displayed
        ///
        /// @return Reference to the renderer
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::shared_ptr<ChildWindowRenderer> getRenderer() const
        {
            return std::static_pointer_cast<ChildWindowRenderer>(m_renderer);
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Set the position of the widget
        ///
        /// This function completely overwrites the previous position.
        /// See the move function to apply an offset based on the previous position instead.
        /// The default position of a transformable widget is (0, 0).
        ///
        /// @param x X coordinate of the new position
        /// @param y Y coordinate of the new position
        ///
        /// @see move, getPosition
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setPosition(const Layout& position) override;
        using Transformable::setPosition;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the size of the child window.
        ///
        /// @param size   Sets the new size of the child window
        /// @param height  Sets the new height of the child window
        ///
        /// This is the size of the child window, without the title bar nor the borders.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setSize(const Layout& size) override;
        using Transformable::setSize;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the size of the full child window.
        ///
        /// @return Size of the child window
        ///
        /// The size returned by this function is the size of the child window, including the title bar and the borders.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual sf::Vector2f getFullSize() const override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the global font.
        ///
        /// This font will be used by all widgets that are created after calling this function.
        ///
        /// @param filename  Path of the font file to load
        ///
        /// @throw Exception when loading fails
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setGlobalFont(const std::string& filename) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the global font.
        ///
        /// This font will be used by all widgets that are created after calling this function.
        ///
        /// @param font  Font to use
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setGlobalFont(std::shared_ptr<sf::Font> font) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the transparency of the widget.
        ///
        /// @param transparency  The transparency of the widget.
        ///                      0 is completely transparent, while 255 (default) means fully opaque.
        ///
        /// Note that this will only change the transparency of the images. The parts of the widgets that use a color will not
        /// be changed. You must change them yourself by setting the alpha channel of the color.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setTransparency(unsigned char transparency) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the title that is displayed in the title bar of the child window.
        ///
        /// @param title  New title for the child window
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTitle(const sf::String& title);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the title that is displayed in the title bar of the child window.
        ///
        /// @return Title of the child window
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const sf::String& getTitle() const
        {
            return m_titleText.getText();
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the title alignment.
        ///
        /// @param alignment  How should the title be aligned in the title bar?
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTitleAlignment(TitleAlignment alignment);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the title alignment.
        ///
        /// @return How the title is aligned in the title bar
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TitleAlignment getTitleAlignment() const
        {
            return m_titleAlignment;
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the icon in the top left corner of the child window.
        ///
        /// @param filename  Filename of the icon image
        ///
        /// There is no icon by default.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setIcon(const std::string& filename);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Removes the icon in the top left corner of the child window.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void removeIcon();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Destroys the window.
        ///
        /// When no callback is requested when closing the window, this function will be called automatically.
        ///
        /// When you requested a callback then you get the opportunity to cancel the closure of the window.
        /// If you want to keep it open then don't do anything, if you want to close it then just call this function.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void destroy();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Set the child window to be kept inside its parent.
        ///
        /// @param enabled  When it's set to true, the child window will always be kept automatically inside its parent.
        ///                 It will be fully kept on left, right and top.
        ///                 At the bottom of the parent only the title bar will be kept inside.
        ///                 It's set to false by default.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void keepInParent(bool enabled);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Tells whether the child window is kept inside its parent.
        ///
        /// @return  When it's set to true, the child window will always be kept automatically inside its parent.
        ///          It will be fully kept on left, right and top.
        ///          At the bottom of the parent only the title bar will be kept inside.
        ///          It's set to false by default.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool isKeptInParent() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the distance between the position of the container and a widget that would be drawn inside
        ///        this container on relative position (0,0).
        ///
        /// @return Offset of the widgets in the container
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual sf::Vector2f getWidgetsOffset() const override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual bool mouseOnWidget(float x, float y) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void leftMousePressed(float x, float y) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void leftMouseReleased(float x, float y) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void mouseMoved(float x, float y) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void mouseWheelMoved(int delta, int x, int y) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void mouseNoLongerDown() override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        // This function is called when the widget is added to a container.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void initialize(Container *const container) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Creates the child window
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void createChildWindow(const std::string& themeFileFilename, const std::string& section);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Makes a copy of the widget
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual Widget::Ptr clone() override
        {
            return std::make_shared<ChildWindow>(*this);
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        // Draws the widget on the render target.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    public:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// Defines specific triggers to ChildWindow.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        enum ChildWindowCallbacks
        {
            LeftMousePressed = WidgetCallbacksCount * 1,  ///< The left mouse button was pressed (child window was thus brought to front)
            Closed = WidgetCallbacksCount * 2,            ///< Child window was closed
            ChildWindowCallbacksCount = WidgetCallbacksCount * 4
        };


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        Texture        m_iconTexture;

        Label          m_titleText;
        sf::Vector2f   m_draggingPosition;
        TitleAlignment m_titleAlignment = TitleAlignmentCentered;

        Button m_closeButton;

        bool m_mouseDownOnTitleBar = false;
        bool m_keepInParent = false;

        friend class ChildWindowRenderer;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class ChildWindowRenderer : public WidgetRenderer, public WidgetBorders
    {
    public:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Constructor
        ///
        /// @param childWindow  The child window that is connected to the renderer
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ChildWindowRenderer(ChildWindow* childWindow) : m_childWindow{childWindow} {}


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Dynamically change a property of the renderer, without even knowing the type of the widget.
        ///
        /// This function should only be used when you don't know the type of the widget.
        /// Otherwise you can make a direct function call to make the wanted change.
        ///
        /// @param property  The property that you would like to change
        /// @param value     The new value that you like to assign to the property
        /// @param rootPath  Path that should be placed in front of any resource filename
        ///
        /// @throw Exception when the property doesn't exist for this widget.
        /// @throw Exception when the value is invalid for this property.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setProperty(std::string property, const std::string& value, const std::string& rootPath = getResourcePath()) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change the image of the title bar
        ///
        /// When this image is set, the title bar color property will be ignored.
        ///
        /// Pass an empty string to unset the image, in this case the title bar color property will be used again.
        ///
        /// @param filename   Filename of the image to load.
        /// @param partRect   Load only part of the image. Don't pass this parameter if you want to load the full image.
        /// @param middlePart Choose the middle part of the image for 9-slice scaling (relative to the part defined by partRect)
        /// @param repeated   Should the image be repeated or stretched when the size is bigger than the image?
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTitleBarImage(const std::string& filename,
                              const sf::IntRect& partRect = sf::IntRect(0, 0, 0, 0),
                              const sf::IntRect& middlePart = sf::IntRect(0, 0, 0, 0),
                              bool repeated = false);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the title bar
        ///
        /// @param color  New title bar color
        ///
        /// Note that this color is ignored when you set an image as title bar.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTitleBarColor(const sf::Color& color);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change the height of the title bar.
        ///
        /// @param height  New height of the title bar
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTitleBarHeight(float height);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the title that is displayed in the title bar of the child window.
        ///
        /// @param color  New title color for the child window
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setTitleColor(const sf::Color& color);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Set the border color.
        ///
        /// @param borderColor  The color of the borders
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setBorderColor(const sf::Color& borderColor);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the size of the borders.
        ///
        /// @param borders  The size of the borders
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setBorders(const Borders& borders) override;
        using WidgetBorders::setBorders;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the distance between the title and the side of the title bar.
        ///
        /// @param distanceToSide  distance between the title and the side of the title bar
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setDistanceToSide(unsigned int distanceToSide);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the background color of the child window.
        ///
        /// @param backgroundColor  New background color
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setBackgroundColor(const sf::Color& backgroundColor);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the renderer of the close button
        ///
        /// @return The close button renderer
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::shared_ptr<ButtonRenderer> getCloseButton() const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Draws the widget on the render target.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void draw(sf::RenderTarget& target, sf::RenderStates states) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Makes a copy of the renderer
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual std::shared_ptr<WidgetRenderer> clone(Widget* widget) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        ChildWindowRenderer(const ChildWindowRenderer&) = default;
        ChildWindowRenderer& operator=(const ChildWindowRenderer&) = delete;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        ChildWindow* m_childWindow;

        float        m_titleBarHeight = 20;
        float        m_distanceToSide = 3;

        Texture      m_textureTitleBar;

        sf::Color    m_titleBarColor   = {255, 255, 255};

        sf::Color    m_backgroundColor = {230, 230, 230};
        sf::Color    m_borderColor     = {0, 0, 0};

        friend class ChildWindow;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TGUI_CHILD_WINDOW_HPP
