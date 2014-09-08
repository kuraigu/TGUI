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


#include <TGUI/Container.hpp>
#include <TGUI/EditBox.hpp>
#include <TGUI/Clipboard.hpp>

#include <SFML/OpenGL.hpp>

#include <cmath>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    EditBox::EditBox()
    {
        m_callback.widgetType = Type_EditBox;
        m_animatedWidget = true;
        m_draggableWidget = true;
        m_allowFocus = true;

        m_defaultText.setStyle(sf::Text::Italic);

        m_caret.setSize({1, 0});

        m_renderer = std::make_shared<EditBoxRenderer>(this);

        getRenderer()->setBorders({2, 2, 2, 2});
        getRenderer()->setPadding({4, 2, 4, 2});

        m_textBeforeSelection.setColor({0, 0, 0});
        m_textSelection.setColor({255, 255, 255});
        m_textAfterSelection.setColor({0, 0, 0});
        m_defaultText.setColor({160, 160, 160});
        m_caret.setFillColor({0, 0, 0});
        m_selectedTextBackground.setFillColor({0, 110, 255});

        setSize({240, 30});
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    EditBox::Ptr EditBox::create(const std::string& themeFileFilename, const std::string& section)
    {
        auto editBox = std::make_shared<EditBox>();

        if (themeFileFilename != "")
        {
            editBox->getRenderer()->setBorders({0, 0, 0, 0});

            std::string loadedThemeFile = getResourcePath() + themeFileFilename;

            // Open the theme file
            ThemeFileParser themeFile{loadedThemeFile, section};

            // Find the folder that contains the theme file
            std::string themeFileFolder = "";
            std::string::size_type slashPos = loadedThemeFile.find_last_of("/\\");
            if (slashPos != std::string::npos)
                themeFileFolder = loadedThemeFile.substr(0, slashPos+1);

            // Handle the read properties
            for (auto it = themeFile.getProperties().cbegin(); it != themeFile.getProperties().cend(); ++it)
            {
                try
                {
                    editBox->getRenderer()->setProperty(it->first, it->second, themeFileFolder);
                }
                catch (const Exception& e)
                {
                    throw Exception{std::string(e.what()) + " In section '" + section + "' in " + loadedThemeFile + "."};
                }
            }

            if (editBox->getRenderer()->m_textureNormal.getData() != nullptr)
                editBox->setSize(editBox->getRenderer()->m_textureNormal.getImageSize());

            // Auto-size the text
            editBox->setTextSize(0);
        }

        return editBox;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::setPosition(const Layout& position)
    {
        Widget::setPosition(position);

        getRenderer()->m_textureHover.setPosition(getPosition());
        getRenderer()->m_textureNormal.setPosition(getPosition());
        getRenderer()->m_textureFocused.setPosition(getPosition());

        recalculateTextPositions();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::setSize(const Layout& size)
    {
        Widget::setSize(size);

        // Recalculate the text size when auto scaling
        if (m_textSize == 0)
            setText(m_text);

        getRenderer()->m_textureHover.setSize(getSize());
        getRenderer()->m_textureNormal.setSize(getSize());
        getRenderer()->m_textureFocused.setSize(getSize());

        // Set the size of the caret
        m_caret.setSize({m_caret.getSize().x,
                         getSize().y - getRenderer()->getScaledPadding().bottom - getRenderer()->getScaledPadding().top});

        // Recalculate the position of the images and texts
        updatePosition();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::setText(const sf::String& text)
    {
        // Check if the text is auto sized
        if (m_textSize == 0)
        {
            // Calculate the text size
            m_textFull.setString("kg");
            m_textFull.setCharacterSize(static_cast<unsigned int>((getSize().y - getRenderer()->getScaledPadding().bottom - getRenderer()->getScaledPadding().top) * 0.75f));
            m_textFull.setString(m_displayedText);

            // Also adjust the character size of the other texts
            m_textBeforeSelection.setCharacterSize(m_textFull.getCharacterSize());
            m_textSelection.setCharacterSize(m_textFull.getCharacterSize());
            m_textAfterSelection.setCharacterSize(m_textFull.getCharacterSize());
            m_defaultText.setCharacterSize(m_textFull.getCharacterSize());
        }
        else // When the text has a fixed size
        {
            // Set the text size
            m_textBeforeSelection.setCharacterSize(m_textSize);
            m_textSelection.setCharacterSize(m_textSize);
            m_textAfterSelection.setCharacterSize(m_textSize);
            m_textFull.setCharacterSize(m_textSize);
            m_defaultText.setCharacterSize(m_textSize);
        }

        // Change the text
        m_text = text;
        m_displayedText = text;

        // If the edit box only accepts numbers then remove all other characters
        if (m_numbersOnly)
            setNumbersOnly(true);

        // If there is a character limit then check if it is exeeded
        if ((m_maxChars > 0) && (m_displayedText.getSize() > m_maxChars))
        {
            // Remove all the excess characters
            m_text.erase(m_maxChars, sf::String::InvalidPos);
            m_displayedText.erase(m_maxChars, sf::String::InvalidPos);
        }

        // Check if there is a password character
        if (m_passwordChar != '\0')
        {
            // Loop every character and change it
            for (unsigned int i = 0; i < m_text.getSize(); ++i)
                m_displayedText[i] = m_passwordChar;
        }

        // Set the texts
        m_textBeforeSelection.setString(m_displayedText);
        m_textSelection.setString("");
        m_textAfterSelection.setString("");
        m_textFull.setString(m_displayedText);

        float width = getVisibleEditBoxWidth();

        // Check if there is a text width limit
        if (m_limitTextWidth)
        {
            // Now check if the text fits into the EditBox
            while (m_textBeforeSelection.findCharacterPos(m_textBeforeSelection.getString().getSize()).x - m_textBeforeSelection.getPosition().x > width)
            {
                // The text doesn't fit inside the EditBox, so the last character must be deleted.
                m_text.erase(m_text.getSize()-1);
                m_displayedText.erase(m_displayedText.getSize()-1);

                // Set the new text
                m_textBeforeSelection.setString(m_displayedText);
            }

            // Set the full text again
            m_textFull.setString(m_displayedText);
        }
        else // There is no text cropping
        {
            // Calculate the text width
            float textWidth = m_textFull.findCharacterPos(m_displayedText.getSize()).x;

            // If the text can be moved to the right then do so
            if (textWidth > width)
            {
                if (textWidth - m_textCropPosition < width)
                    m_textCropPosition = static_cast<unsigned int>(textWidth - width);
            }
            else
                m_textCropPosition = 0;
        }

        // Set the caret behind the last character
        setCaretPosition(m_displayedText.getSize());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    sf::String EditBox::getSelectedText() const
    {
        if (m_selStart < m_selEnd)
            return m_text.substring(m_selStart, m_selChars);
        else if (m_selStart > m_selEnd)
            return m_text.substring(m_selEnd, m_selChars);
        else
            return "";
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::setTextSize(unsigned int size)
    {
        // Change the text size
        m_textSize = size;

        // Call setText to re-position the text
        setText(m_text);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::setPasswordCharacter(char passwordChar)
    {
        // Change the password character
        m_passwordChar = passwordChar;

        // Recalculate the text position
        setText(m_text);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::setMaximumCharacters(unsigned int maxChars)
    {
        // Set the new character limit ( 0 to disable the limit )
        m_maxChars = maxChars;

        // If there is a character limit then check if it is exceeded
        if ((m_maxChars > 0) && (m_displayedText.getSize() > m_maxChars))
        {
            // Remove all the excess characters
            m_text.erase(m_maxChars, sf::String::InvalidPos);
            m_displayedText.erase(m_maxChars, sf::String::InvalidPos);

            // If we passed here then the text has changed.
            m_textBeforeSelection.setString(m_displayedText);
            m_textSelection.setString("");
            m_textAfterSelection.setString("");
            m_textFull.setString(m_displayedText);

            // Set the caret behind the last character
            setCaretPosition(m_displayedText.getSize());
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::limitTextWidth(bool limitWidth)
    {
        m_limitTextWidth = limitWidth;

        // Check if the width is being limited
        if (m_limitTextWidth == true)
        {
            // Now check if the text fits into the EditBox
            float width = getVisibleEditBoxWidth();
            while (m_textBeforeSelection.findCharacterPos(m_displayedText.getSize()).x - m_textBeforeSelection.getPosition().x > width)
            {
                // The text doesn't fit inside the EditBox, so the last character must be deleted.
                m_text.erase(m_text.getSize()-1);
                m_displayedText.erase(m_displayedText.getSize()-1);
                m_textBeforeSelection.setString(m_displayedText);
            }

            // The full text might have changed
            m_textFull.setString(m_displayedText);

            // There is no clipping
            m_textCropPosition = 0;

            // If the caret was behind the limit, then set it at the end
            if (m_selEnd > m_displayedText.getSize())
                setCaretPosition(m_selEnd);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::setCaretPosition(unsigned int charactersBeforeCaret)
    {
        // The caret position has to stay inside the string
        if (charactersBeforeCaret > m_text.getSize())
            charactersBeforeCaret = m_text.getSize();

        // Set the caret to the correct position
        m_selChars = 0;
        m_selStart = charactersBeforeCaret;
        m_selEnd = charactersBeforeCaret;

        // Change our texts
        m_textBeforeSelection.setString(m_displayedText);
        m_textSelection.setString("");
        m_textAfterSelection.setString("");
        m_textFull.setString(m_displayedText);

        // Check if scrolling is enabled
        if (m_limitTextWidth == false)
        {
            // Find out the position of the caret
            float caretPosition = m_textFull.findCharacterPos(m_selEnd).x;

            if (m_selEnd == m_displayedText.getSize())
                caretPosition += m_textFull.getCharacterSize() / 10.f;

            // If the caret is too far on the right then adjust the cropping
            if (m_textCropPosition + getVisibleEditBoxWidth() < caretPosition)
                m_textCropPosition = static_cast<unsigned int>(caretPosition - getVisibleEditBoxWidth());

            // If the caret is too far on the left then adjust the cropping
            if (m_textCropPosition > caretPosition)
                m_textCropPosition = static_cast<unsigned int>(caretPosition);
        }

        recalculateTextPositions();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::setCaretWidth(unsigned int width)
    {
        m_caret.setPosition(m_caret.getPosition().x + ((m_caret.getSize().x - width) / 2.0f), m_caret.getPosition().y);
        m_caret.setSize({static_cast<float>(width), getSize().y - getRenderer()->getScaledPadding().bottom - getRenderer()->getScaledPadding().top});
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::setNumbersOnly(bool numbersOnly)
    {
        m_numbersOnly = numbersOnly;

        // Remove all letters from the edit box if needed
        if (numbersOnly && !m_text.isEmpty())
        {
            sf::String newText;
            bool commaFound = false;

            if ((m_text[0] == '+') || (m_text[0] == '-'))
                newText += m_text[0];

            for (unsigned int i = 0; i < m_text.getSize(); ++i)
            {
                if (!commaFound)
                {
                    if ((m_text[i] == ',') || (m_text[i] == '.'))
                    {
                        newText += m_text[i];
                        commaFound = true;
                    }
                }

                if ((m_text[i] >= '0') && (m_text[i] <= '9'))
                    newText += m_text[i];
            }

            // When the text changed then reposition the text
            if (newText != m_text)
                setText(newText);
        }
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::setTransparency(unsigned char transparency)
    {
        ClickableWidget::setTransparency(transparency);

        getRenderer()->m_textureNormal.setColor(sf::Color(255, 255, 255, m_opacity));
        getRenderer()->m_textureHover.setColor(sf::Color(255, 255, 255, m_opacity));
        getRenderer()->m_textureFocused.setColor(sf::Color(255, 255, 255, m_opacity));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::leftMousePressed(float x, float y)
    {
        // Find the caret position
        float positionX = x - getPosition().x - getRenderer()->getScaledPadding().left;

        unsigned int caretPosition = findCaretPosition(positionX);

        // When clicking on the left of the first character, move the caret to the left
        if ((positionX < 0) && (caretPosition > 0))
            --caretPosition;

        // When clicking on the right of the right character, move the caret to the right
        else if ((positionX > getVisibleEditBoxWidth()) && (caretPosition < m_displayedText.getSize()))
            ++caretPosition;

        // Check if this is a double click
        if ((m_possibleDoubleClick) && (m_selChars == 0) && (caretPosition == m_selEnd))
        {
            // The next click is going to be a normal one again
            m_possibleDoubleClick = false;

            // Set the caret at the end of the text
            setCaretPosition(m_displayedText.getSize());

            // Select the whole text
            m_selStart = 0;
            m_selEnd = m_text.getSize();
            m_selChars = m_text.getSize();

            // Change the texts
            m_textBeforeSelection.setString("");
            m_textSelection.setString(m_displayedText);
            m_textAfterSelection.setString("");
        }
        else // No double clicking
        {
            // Set the new caret
            setCaretPosition(caretPosition);

            // If the next click comes soon enough then it will be a double click
            m_possibleDoubleClick = true;
        }

        // Set the mouse down flag
        m_mouseDown = true;

        // Add the callback (if the user requested it)
        if (m_callbackFunctions[LeftMousePressed].empty() == false)
        {
            m_callback.trigger = LeftMousePressed;
            m_callback.mouse.x = static_cast<int>(x - getPosition().x);
            m_callback.mouse.y = static_cast<int>(y - getPosition().y);
            addCallback();
        }

        recalculateTextPositions();

        // The caret should be visible
        m_caretVisible = true;
        m_animationTimeElapsed = {};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::mouseMoved(float x, float)
    {
        if (!m_mouseHover)
            mouseEnteredWidget();

        // The mouse has moved so a double click is no longer possible
        m_possibleDoubleClick = false;

        // Check if the mouse is hold down (we are selecting multiple characters)
        if (m_mouseDown)
        {
            // Check if there is a text width limit
            if (m_limitTextWidth)
            {
                // Find out between which characters the mouse is standing
                m_selEnd = findCaretPosition(x - getPosition().x - getRenderer()->getScaledPadding().left);
            }
            else // Scrolling is enabled
            {
                float width = getVisibleEditBoxWidth();

                // Check if the mouse is on the left of the text
                if (x - getPosition().x < getRenderer()->getScaledPadding().left)
                {
                    // Move the text by a few pixels
                    if (m_textFull.getCharacterSize() > 10)
                    {
                        if (m_textCropPosition > m_textFull.getCharacterSize() / 10)
                            m_textCropPosition -= static_cast<unsigned int>(std::floor(m_textFull.getCharacterSize() / 10.f + 0.5f));
                        else
                            m_textCropPosition = 0;
                    }
                    else
                    {
                        if (m_textCropPosition)
                            --m_textCropPosition;
                    }
                }
                // Check if the mouse is on the right of the text AND there is a possibility to scroll
                else if ((x - getPosition().x > getRenderer()->getScaledPadding().left + width) && (m_textFull.findCharacterPos(m_displayedText.getSize()).x > width))
                {
                    // Move the text by a few pixels
                    if (m_textFull.getCharacterSize() > 10)
                    {
                        if (m_textCropPosition + width < m_textFull.findCharacterPos(m_displayedText.getSize()).x + (m_textFull.getCharacterSize() / 10))
                            m_textCropPosition += static_cast<unsigned int>(std::floor(m_textFull.getCharacterSize() / 10.f + 0.5f));
                        else
                            m_textCropPosition = static_cast<unsigned int>(m_textFull.findCharacterPos(m_displayedText.getSize()).x + (m_textFull.getCharacterSize() / 10) - width);
                    }
                    else
                    {
                        if (m_textCropPosition + width < m_textFull.findCharacterPos(m_displayedText.getSize()).x)
                            ++m_textCropPosition;
                    }
                }

                // Find out between which characters the mouse is standing
                m_selEnd = findCaretPosition(x - getPosition().x - getRenderer()->getScaledPadding().left);
            }

            // Check if we are selecting text from left to right
            if (m_selEnd > m_selStart)
            {
                // There is no need to redo everything when nothing changed
                if (m_selChars != (m_selEnd - m_selStart))
                {
                    // Adjust the number of characters that are selected
                    m_selChars = m_selEnd - m_selStart;

                    // Change our three texts
                    m_textBeforeSelection.setString(m_displayedText.toWideString().substr(0, m_selStart));
                    m_textSelection.setString(m_displayedText.toWideString().substr(m_selStart, m_selChars));
                    m_textAfterSelection.setString(m_displayedText.toWideString().substr(m_selEnd));

                    recalculateTextPositions();
                }
            }
            else if (m_selEnd < m_selStart)
            {
                // There is no need to redo everything when nothing changed
                if (m_selChars != (m_selStart - m_selEnd))
                {
                    // Adjust the number of characters that are selected
                    m_selChars = m_selStart - m_selEnd;

                    // Change our three texts
                    m_textBeforeSelection.setString(m_displayedText.toWideString().substr(0, m_selEnd));
                    m_textSelection.setString(m_displayedText.toWideString().substr(m_selEnd, m_selChars));
                    m_textAfterSelection.setString(m_displayedText.toWideString().substr(m_selStart));

                    recalculateTextPositions();
                }
            }
            else if (m_selChars > 0)
            {
                // Adjust the number of characters that are selected
                m_selChars = 0;

                // Change our three texts
                m_textBeforeSelection.setString(m_displayedText);
                m_textSelection.setString("");
                m_textAfterSelection.setString("");

                recalculateTextPositions();
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::keyPressed(const sf::Event::KeyEvent& event)
    {
        // Check if one of the correct keys was pressed
        if (event.code == sf::Keyboard::Left)
        {
            // Check if we have selected some text
            if (m_selChars > 0)
            {
                // We will not move the caret, but just undo the selection
                if (m_selStart < m_selEnd)
                    setCaretPosition(m_selStart);
                else
                    setCaretPosition(m_selEnd);
            }
            else // When we did not select any text
            {
                // Move the caret to the left
                if (m_selEnd > 0)
                    setCaretPosition(m_selEnd - 1);
            }

            // Our caret has moved, it should be visible
            m_caretVisible = true;
            m_animationTimeElapsed = {};
        }
        else if (event.code == sf::Keyboard::Right)
        {
            // Check if we have selected some text
            if (m_selChars > 0)
            {
                // We will not move the caret, but just undo the selection
                if (m_selStart < m_selEnd)
                    setCaretPosition(m_selEnd);
                else
                    setCaretPosition(m_selStart);
            }
            else // When we did not select any text
            {
                // Move the caret to the right
                if (m_selEnd < m_displayedText.getSize())
                    setCaretPosition(m_selEnd + 1);
            }

            // Our caret has moved, it should be visible
            m_caretVisible = true;
            m_animationTimeElapsed = {};
        }
        else if (event.code == sf::Keyboard::Home)
        {
            // Set the caret to the beginning of the text
            setCaretPosition(0);

            // Our caret has moved, it should be visible
            m_caretVisible = true;
            m_animationTimeElapsed = {};
        }
        else if (event.code == sf::Keyboard::End)
        {
            // Set the caret behind the text
            setCaretPosition(m_text.getSize());

            // Our caret has moved, it should be visible
            m_caretVisible = true;
            m_animationTimeElapsed = {};
        }
        else if (event.code == sf::Keyboard::Return)
        {
            // Add the callback (if the user requested it)
            if (m_callbackFunctions[ReturnKeyPressed].empty() == false)
            {
                m_callback.trigger = ReturnKeyPressed;
                m_callback.text    = m_text;
                addCallback();
            }
        }
        else if (event.code == sf::Keyboard::BackSpace)
        {
            // Make sure that we did not select any characters
            if (m_selChars == 0)
            {
                // We can't delete any characters when you are at the beginning of the string
                if (m_selEnd == 0)
                    return;

                // Erase the character
                m_text.erase(m_selEnd-1, 1);
                m_displayedText.erase(m_selEnd-1, 1);

                // Set the caret back on the correct position
                setCaretPosition(m_selEnd - 1);

                float width = getVisibleEditBoxWidth();

                // Calculate the text width
                float textWidth = m_textFull.findCharacterPos(m_displayedText.getSize()).x;

                // If the text can be moved to the right then do so
                if (textWidth > width)
                {
                    if (textWidth - m_textCropPosition < width)
                        m_textCropPosition = static_cast<unsigned int>(textWidth - width);
                }
                else
                    m_textCropPosition = 0;
            }
            else // When you did select some characters, delete them
                deleteSelectedCharacters();

            // The caret should be visible again
            m_caretVisible = true;
            m_animationTimeElapsed = {};

            // Add the callback (if the user requested it)
            if (m_callbackFunctions[TextChanged].empty() == false)
            {
                m_callback.trigger = TextChanged;
                m_callback.text    = m_text;
                addCallback();
            }
        }
        else if (event.code == sf::Keyboard::Delete)
        {
            // Make sure that no text is selected
            if (m_selChars == 0)
            {
                // When the caret is at the end of the line then you can't delete anything
                if (m_selEnd == m_text.getSize())
                    return;

                // Erase the character
                m_text.erase(m_selEnd, 1);
                m_displayedText.erase(m_selEnd, 1);

                // Set the caret back on the correct position
                setCaretPosition(m_selEnd);

                // Calculate the text width
                float textWidth = m_textFull.findCharacterPos(m_displayedText.getSize()).x;

                // If the text can be moved to the right then do so
                float width = getVisibleEditBoxWidth();
                if (textWidth > width)
                {
                    if (textWidth - m_textCropPosition < width)
                        m_textCropPosition = static_cast<unsigned int>(textWidth - width);
                }
                else
                    m_textCropPosition = 0;
            }
            else // You did select some characters, delete them
                deleteSelectedCharacters();

            // The caret should be visible again
            m_caretVisible = true;
            m_animationTimeElapsed = {};

            // Add the callback (if the user requested it)
            if (m_callbackFunctions[TextChanged].empty() == false)
            {
                m_callback.trigger = TextChanged;
                m_callback.text    = m_text;
                addCallback();
            }
        }
        else
        {
            // Check if you are copying, pasting or cutting text
            if (event.control)
            {
                if (event.code == sf::Keyboard::C)
                {
                    TGUI_Clipboard.set(m_textSelection.getString());
                }
                else if (event.code == sf::Keyboard::V)
                {
                    auto clipboardContents = TGUI_Clipboard.get();

                    // Only continue pasting if you actually have to do something
                    if ((m_selChars > 0) || (clipboardContents.getSize() > 0))
                    {
                        deleteSelectedCharacters();

                        unsigned int oldCaretPos = m_selEnd;

                        if (m_text.getSize() > m_selEnd)
                            setText(m_text.toWideString().substr(0, m_selEnd) + TGUI_Clipboard.get() + m_text.toWideString().substr(m_selEnd, m_text.getSize() - m_selEnd));
                        else
                            setText(m_text + clipboardContents);

                        setCaretPosition(oldCaretPos + clipboardContents.getSize());

                        // Add the callback (if the user requested it)
                        if (m_callbackFunctions[TextChanged].empty() == false)
                        {
                            m_callback.trigger = TextChanged;
                            m_callback.text    = m_text;
                            addCallback();
                        }
                    }
                }
                else if (event.code == sf::Keyboard::X)
                {
                    TGUI_Clipboard.set(m_textSelection.getString());
                    deleteSelectedCharacters();
                }
                else if (event.code == sf::Keyboard::A)
                {
                    m_selStart = 0;
                    m_selEnd = m_text.getSize();
                    m_selChars = m_text.getSize();

                    m_textBeforeSelection.setString("");
                    m_textSelection.setString(m_displayedText);
                    m_textAfterSelection.setString("");

                    recalculateTextPositions();
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::textEntered(sf::Uint32 key)
    {
        // If only numbers are supported then make sure the input is valid
        if (m_numbersOnly)
        {
            if ((key < '0') || (key > '9'))
            {
                if ((key == '-') || (key == '+'))
                {
                    if ((m_selStart == 0) || (m_selEnd == 0))
                    {
                        if (!m_text.isEmpty())
                        {
                            // You can't have multiple + and - characters after each other
                            if ((m_text[0] == '-') || (m_text[0] == '+'))
                                return;
                        }
                    }
                    else // + and - symbols are only allowed at the beginning of the line
                        return;
                }
                else if ((key == ',') || (key == '.'))
                {
                    // Only one comma is allowed
                    for (auto it = m_text.begin(); it != m_text.end(); ++it)
                    {
                        if ((*it == ',') || (*it == '.'))
                            return;
                    }
                }
                else // Character not accepted
                    return;
            }
        }

        // If there are selected characters then delete them first
        if (m_selChars > 0)
            deleteSelectedCharacters();

        // Make sure we don't exceed our maximum characters limit
        if ((m_maxChars > 0) && (m_text.getSize() + 1 > m_maxChars))
            return;

        // Insert our character
        m_text.insert(m_selEnd, key);

        // Change the displayed text
        if (m_passwordChar != '\0')
            m_displayedText.insert(m_selEnd, m_passwordChar);
        else
            m_displayedText.insert(m_selEnd, key);

        // Append the character to the text
        m_textFull.setString(m_displayedText);

        // When there is a text width limit then reverse what we just did
        if (m_limitTextWidth)
        {
            // Now check if the text fits into the EditBox
            if (m_textFull.findCharacterPos(m_displayedText.getSize()).x > getVisibleEditBoxWidth())
            {
                // If the text does not fit in the EditBox then delete the added character
                m_text.erase(m_selEnd, 1);
                m_displayedText.erase(m_selEnd, 1);
                return;
            }
        }

        // Move our caret forward
        setCaretPosition(m_selEnd + 1);

        // The caret should be visible again
        m_caretVisible = true;
        m_animationTimeElapsed = {};

        // Add the callback (if the user requested it)
        if (m_callbackFunctions[TextChanged].empty() == false)
        {
            m_callback.trigger = TextChanged;
            m_callback.text    = m_text;
            addCallback();
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::widgetUnfocused()
    {
        // If there is a selection then undo it now
        if (m_selChars)
            setCaretPosition(m_selEnd);

        Widget::widgetUnfocused();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    float EditBox::getVisibleEditBoxWidth()
    {
        return std::max(0.f, getSize().x - getRenderer()->getScaledPadding().left - getRenderer()->getScaledPadding().right);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    unsigned int EditBox::findCaretPosition(float posX)
    {
        // This code will crash when the editbox is empty. We need to avoid this.
        if (m_displayedText.isEmpty())
            return 0;

        // Find out what the first visible character is
        unsigned int firstVisibleChar;
        if (m_textCropPosition)
        {
            // Start searching near the caret to quickly find the character even in a very long string
            firstVisibleChar = m_selEnd;

            // Go backwards to find the character
            while (m_textFull.findCharacterPos(firstVisibleChar-1).x > m_textCropPosition)
                --firstVisibleChar;
        }
        else // If the first part is visible then the first character is also visible
            firstVisibleChar = 0;

        sf::String tempString;
        float textWidthWithoutLastChar;
        float fullTextWidth;
        float halfOfLastCharWidth;
        unsigned int lastVisibleChar;
        float width = getVisibleEditBoxWidth();

        // Find out how many pixels the text is moved
        float pixelsToMove = 0;
        if (m_textAlignment != Alignment::Left)
        {
            // Calculate the text width
            float textWidth = m_textFull.findCharacterPos(m_displayedText.getSize()).x;

            // Check if a layout would make sense
            if (textWidth < width)
            {
                // Set the number of pixels to move
                if (m_textAlignment == Alignment::Center)
                    pixelsToMove = (width - textWidth) / 2.f;
                else // if (textAlignment == Alignment::Right)
                    pixelsToMove = width - textWidth;
            }
        }

        // Find out what the last visible character is, starting from the caret
        lastVisibleChar = m_selEnd;

        // Go forward to find the character
        while (m_textFull.findCharacterPos(lastVisibleChar+1).x < m_textCropPosition + width)
        {
            if (lastVisibleChar == m_displayedText.getSize())
                break;

            ++lastVisibleChar;
        }

        // Set the first part of the text
        tempString = m_displayedText.toWideString().substr(0, firstVisibleChar);
        m_textFull.setString(tempString);

        // Calculate the first position
        fullTextWidth = m_textFull.findCharacterPos(firstVisibleChar).x;

        // for all the other characters, check where you have clicked.
        for (unsigned int i = firstVisibleChar; i < lastVisibleChar; ++i)
        {
            // Add the next character to the temporary string
            tempString += m_displayedText[i];
            m_textFull.setString(tempString);

            // Make some calculations
            textWidthWithoutLastChar = fullTextWidth;
            fullTextWidth = m_textFull.findCharacterPos(i + 1).x;
            halfOfLastCharWidth = (fullTextWidth - textWidthWithoutLastChar) / 2.0f;

            // Check if you have clicked on the first halve of that character
            if (posX < textWidthWithoutLastChar + pixelsToMove + halfOfLastCharWidth - m_textCropPosition)
            {
                m_textFull.setString(m_displayedText);
                return i;
            }
        }

        // If you pass here then you clicked behind all the characters
        m_textFull.setString(m_displayedText);
        return lastVisibleChar;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::deleteSelectedCharacters()
    {
        // Nothing to delete when no text was selected
        if (m_selChars == 0)
            return;

        // Check if the characters were selected from left to right
        if (m_selStart < m_selEnd)
        {
            // Erase the characters
            m_text.erase(m_selStart, m_selChars);
            m_displayedText.erase(m_selStart, m_selChars);

            // Set the caret back on the correct position
            setCaretPosition(m_selStart);
        }
        else // When the text is selected from right to left
        {
            // Erase the characters
            m_text.erase(m_selEnd, m_selChars);
            m_displayedText.erase(m_selEnd, m_selChars);

            // Set the caret back on the correct position
            setCaretPosition(m_selEnd);
        }

        // Calculate the text width
        float textWidth = m_textFull.findCharacterPos(m_displayedText.getSize()).x;

        // If the text can be moved to the right then do so
        float width = getVisibleEditBoxWidth();
        if (textWidth > width)
        {
            if (textWidth - m_textCropPosition < width)
                m_textCropPosition = static_cast<unsigned int>(textWidth - width);
        }
        else
            m_textCropPosition = 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::recalculateTextPositions()
    {
        float textX = getPosition().x;
        float textY = getPosition().y;

        Padding padding = getRenderer()->getScaledPadding();

        textX += padding.left - m_textCropPosition;
        textY += padding.top;

        // Check if the layout wasn't left
        if (m_textAlignment != Alignment::Left)
        {
            // Calculate the text width
            float textWidth = m_textFull.findCharacterPos(m_displayedText.getSize()).x;

            // Check if a layout would make sense
            if (textWidth < getVisibleEditBoxWidth())
            {
                // Put the text on the correct position
                if (m_textAlignment == Alignment::Center)
                    textX += (getVisibleEditBoxWidth() - textWidth) / 2.f;
                else // if (textAlignment == Alignment::Right)
                    textX += getVisibleEditBoxWidth() - textWidth;
            }
        }

        float caretLeft = textX;

        // Set the position of the text
        sf::Text tempText(m_textFull);
        tempText.setString("kg");
        textY += (((getSize().y - padding.top - padding.bottom) - tempText.getLocalBounds().height) * 0.5f) - tempText.getLocalBounds().top;

        // Set the text before the selection on the correct position
        m_textBeforeSelection.setPosition(std::floor(textX + 0.5f), std::floor(textY + 0.5f));
        m_defaultText.setPosition(std::floor(textX + 0.5f), std::floor(textY + 0.5f));

        // Check if there is a selection
        if (m_selChars != 0)
        {
            // Watch out for the kerning
            if (m_textBeforeSelection.getString().getSize() > 0)
                textX += m_font->getKerning(m_displayedText[m_textBeforeSelection.getString().getSize() - 1], m_displayedText[m_textBeforeSelection.getString().getSize()], m_textBeforeSelection.getCharacterSize());

            textX += m_textBeforeSelection.findCharacterPos(m_textBeforeSelection.getString().getSize()).x - m_textBeforeSelection.getPosition().x;

            // Set the position and size of the rectangle that gets drawn behind the selected text
            m_selectedTextBackground.setSize({m_textSelection.findCharacterPos(m_textSelection.getString().getSize()).x - m_textSelection.getPosition().x,
                                              getSize().y - padding.top - padding.bottom});
            m_selectedTextBackground.setPosition(std::floor(textX + 0.5f), std::floor(getPosition().y + padding.top + 0.5f));

            // Set the text selected text on the correct position
            m_textSelection.setPosition(std::floor(textX + 0.5f), std::floor(textY + 0.5f));

            // Watch out for kerning
            if (m_displayedText.getSize() > m_textBeforeSelection.getString().getSize() + m_textSelection.getString().getSize())
                textX += m_font->getKerning(m_displayedText[m_textBeforeSelection.getString().getSize() + m_textSelection.getString().getSize() - 1], m_displayedText[m_textBeforeSelection.getString().getSize() + m_textSelection.getString().getSize()], m_textBeforeSelection.getCharacterSize());

            // Set the text selected text on the correct position
            textX += m_textSelection.findCharacterPos(m_textSelection.getString().getSize()).x  - m_textSelection.getPosition().x;
            m_textAfterSelection.setPosition(std::floor(textX + 0.5f), std::floor(textY + 0.5f));
        }

        // Set the position of the caret
        caretLeft += m_textFull.findCharacterPos(m_selEnd).x - (m_caret.getSize().x * 0.5f);
        m_caret.setPosition(std::floor(caretLeft + 0.5f), std::floor(padding.top + getPosition().y + 0.5f));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::initialize(Container *const parent)
    {
        Widget::initialize(parent);

        if (!m_font && m_parent->getGlobalFont())
            getRenderer()->setTextFont(m_parent->getGlobalFont());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::update()
    {
        // Only show/hide the caret every half second
        if (m_animationTimeElapsed >= sf::milliseconds(500))
        {
            // Reset the elapsed time
            m_animationTimeElapsed = {};

            // Switch the value of the visible flag
            m_caretVisible = !m_caretVisible;

            // Too slow for double clicking
            m_possibleDoubleClick = false;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBox::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Draw the background
        getRenderer()->draw(target, states);

        // Calculate the scale factor of the view
        const sf::View& view = target.getView();
        float scaleViewX = target.getSize().x / view.getSize().x;
        float scaleViewY = target.getSize().y / view.getSize().y;

        Padding padding = getRenderer()->getScaledPadding();

        // Get the global position
        sf::Vector2f topLeftPosition = {((getAbsolutePosition().x + padding.left - view.getCenter().x + (view.getSize().x / 2.f)) * view.getViewport().width) + (view.getSize().x * view.getViewport().left),
                                        ((getAbsolutePosition().y + padding.top - view.getCenter().y + (view.getSize().y / 2.f)) * view.getViewport().height) + (view.getSize().y * view.getViewport().top)};
        sf::Vector2f bottomRightPosition = {(getAbsolutePosition().x + getSize().x - padding.right - view.getCenter().x + (view.getSize().x / 2.f)) * view.getViewport().width + (view.getSize().x * view.getViewport().left),
                                            (getAbsolutePosition().y + getSize().y - padding.bottom - view.getCenter().y + (view.getSize().y / 2.f)) * view.getViewport().height + (view.getSize().y * view.getViewport().top)};

        // Get the old clipping area
        GLint scissor[4];
        glGetIntegerv(GL_SCISSOR_BOX, scissor);

        // Calculate the clipping area
        GLint scissorLeft = std::max(static_cast<GLint>(topLeftPosition.x * scaleViewX), scissor[0]);
        GLint scissorTop = std::max(static_cast<GLint>(topLeftPosition.y * scaleViewY), static_cast<GLint>(target.getSize().y) - scissor[1] - scissor[3]);
        GLint scissorRight = std::min(static_cast<GLint>(bottomRightPosition.x * scaleViewX), scissor[0] + scissor[2]);
        GLint scissorBottom = std::min(static_cast<GLint>(bottomRightPosition.y * scaleViewY), static_cast<GLint>(target.getSize().y) - scissor[1]);

        if (scissorRight < scissorLeft)
            scissorRight = scissorLeft;
        else if (scissorBottom < scissorTop)
            scissorTop = scissorBottom;

        // Set the clipping area
        glScissor(scissorLeft, target.getSize().y - scissorBottom, scissorRight - scissorLeft, scissorBottom - scissorTop);

        if ((m_textBeforeSelection.getString() != "") || (m_textSelection.getString() != ""))
        {
            target.draw(m_textBeforeSelection, states);

            if (m_textSelection.getString() != "")
            {
                target.draw(m_selectedTextBackground, states);

                target.draw(m_textSelection, states);
                target.draw(m_textAfterSelection, states);
            }
        }
        else if (m_defaultText.getString() != "")
        {
            target.draw(m_defaultText, states);
        }

        // Draw the caret
        if ((m_focused) && (m_caretVisible))
            target.draw(m_caret, states);

        // Reset the old clipping area
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setProperty(std::string property, const std::string& value, const std::string& rootPath)
    {
        if (property == "textcolor")
            setTextColor(extractColorFromString(property, value));
        else if (property == "selectedtextcolor")
            setSelectedTextColor(extractColorFromString(property, value));
        else if (property == "selectedtextbackgroundcolor")
            setSelectedTextBackgroundColor(extractColorFromString(property, value));
        else if (property == "defaulttextcolor")
            setDefaultTextColor(extractColorFromString(property, value));
        else if (property == "caretcolor")
            setCaretColor(extractColorFromString(property, value));
        else if (property == "padding")
            setPadding(extractBordersFromString(property, value));
        else if (property == "borders")
            setBorders(extractBordersFromString(property, value));
        else if (property == "bordercolor")
            setBorderColor(extractColorFromString(property, value));
        else if (property == "backgroundcolor")
            setBackgroundColor(extractColorFromString(property, value));
        else if (property == "backgroundcolornormal")
            setBackgroundColorNormal(extractColorFromString(property, value));
        else if (property == "backgroundcolorhover")
            setBackgroundColorHover(extractColorFromString(property, value));
        else if (property == "normalimage")
            extractTextureFromString(property, value, rootPath, m_textureNormal);
        else if (property == "hoverimage")
            extractTextureFromString(property, value, rootPath, m_textureHover);
        else if (property == "focusedimage")
            extractTextureFromString(property, value, rootPath, m_textureFocused);
        else
            throw Exception{"Unrecognized property '" + property + "'."};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setTextFont(std::shared_ptr<sf::Font> font)
    {
        m_editBox->m_font = font;

        if (font != nullptr)
        {
            m_editBox->m_textBeforeSelection.setFont(*font);
            m_editBox->m_textSelection.setFont(*font);
            m_editBox->m_textAfterSelection.setFont(*font);
            m_editBox->m_textFull.setFont(*font);
            m_editBox->m_defaultText.setFont(*font);
        }

        // Recalculate the text size and position
        m_editBox->setText(m_editBox->m_text);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setPadding(const Padding& padding)
    {
        WidgetPadding::setPadding(padding);

        // Recalculate the text size
        m_editBox->setText(m_editBox->m_text);

        // Set the size of the caret
        m_editBox->m_caret.setSize({m_editBox->m_caret.getSize().x,
                                    m_editBox->getSize().y - getScaledPadding().bottom - getScaledPadding().top});
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setTextColor(const sf::Color& textColor)
    {
        m_editBox->m_textBeforeSelection.setColor(textColor);
        m_editBox->m_textAfterSelection.setColor(textColor);
    }

     /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setDefaultTextColor(const sf::Color& defaultTextColor)
    {
        m_editBox->m_defaultText.setColor(defaultTextColor);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setBackgroundColor(const sf::Color& color)
    {
        m_backgroundColorNormal = color;
        m_backgroundColorHover = color;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setBackgroundColorNormal(const sf::Color& color)
    {
        m_backgroundColorNormal = color;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setBackgroundColorHover(const sf::Color& color)
    {
        m_backgroundColorHover = color;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setBorderColor(const sf::Color& color)
    {
        m_borderColor = color;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setNormalImage(const std::string& filename, const sf::IntRect& partRect, const sf::IntRect& middlePart, bool repeated)
    {
        if (filename != "")
            m_textureNormal.load(getResourcePath() + filename, partRect, middlePart, repeated);
        else
            m_textureNormal = {};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setHoverImage(const std::string& filename, const sf::IntRect& partRect, const sf::IntRect& middlePart, bool repeated)
    {
        if (filename != "")
            m_textureHover.load(getResourcePath() + filename, partRect, middlePart, repeated);
        else
            m_textureHover = {};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::setFocusedImage(const std::string& filename, const sf::IntRect& partRect, const sf::IntRect& middlePart, bool repeated)
    {
        if (filename != "")
            m_textureFocused.load(getResourcePath() + filename, partRect, middlePart, repeated);
        else
            m_textureFocused = {};
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void EditBoxRenderer::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Check if there is a background texture
        if (m_textureNormal.getData() != nullptr)
        {
            if (m_editBox->m_mouseHover && m_textureHover.getData())
                target.draw(m_textureHover, states);
            else
                target.draw(m_textureNormal, states);

            // When the edit box is focused then draw an extra image
            if (m_editBox->m_focused && m_textureFocused.getData())
                target.draw(m_textureFocused, states);
        }
        else // There is no background texture
        {
            sf::RectangleShape editBox(m_editBox->getSize());
            editBox.setPosition(m_editBox->getPosition());

            if (m_editBox->m_mouseHover)
                editBox.setFillColor(m_backgroundColorHover);
            else
                editBox.setFillColor(m_backgroundColorNormal);

            target.draw(editBox, states);
        }

        // Draw the borders around the edit box
        if (m_borders != Borders{0, 0, 0, 0})
        {
            sf::Vector2f position = m_editBox->getPosition();
            sf::Vector2f size = m_editBox->getSize();

            // Draw left border
            sf::RectangleShape border({m_borders.left, size.y + m_borders.top});
            border.setPosition(position.x - m_borders.left, position.y - m_borders.top);
            border.setFillColor(m_borderColor);
            target.draw(border, states);

            // Draw top border
            border.setSize({size.x + m_borders.right, m_borders.top});
            border.setPosition(position.x, position.y - m_borders.top);
            target.draw(border, states);

            // Draw right border
            border.setSize({m_borders.right, size.y + m_borders.bottom});
            border.setPosition(position.x + size.x, position.y);
            target.draw(border, states);

            // Draw bottom border
            border.setSize({size.x + m_borders.left, m_borders.bottom});
            border.setPosition(position.x - m_borders.left, position.y + size.y);
            target.draw(border, states);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Padding EditBoxRenderer::getScaledPadding() const
    {
        Padding padding = getPadding();
        Padding scaledPadding = padding;

        auto& texture = m_textureNormal;
        if (texture.getData() != nullptr)
        {
            switch (texture.getScalingType())
            {
            case Texture::ScalingType::Normal:
                scaledPadding.left = padding.left * (texture.getSize().x / texture.getImageSize().x);
                scaledPadding.right = padding.right * (texture.getSize().x / texture.getImageSize().x);
                scaledPadding.top = padding.top * (texture.getSize().y / texture.getImageSize().y);
                scaledPadding.bottom = padding.bottom * (texture.getSize().y / texture.getImageSize().y);
                break;

            case Texture::ScalingType::Horizontal:
                scaledPadding.left = padding.left * (texture.getSize().y / texture.getImageSize().y);
                scaledPadding.right = padding.right * (texture.getSize().y / texture.getImageSize().y);
                scaledPadding.top = padding.top * (texture.getSize().y / texture.getImageSize().y);
                scaledPadding.bottom = padding.bottom * (texture.getSize().y / texture.getImageSize().y);
                break;

            case Texture::ScalingType::Vertical:
                scaledPadding.left = padding.left * (texture.getSize().x / texture.getImageSize().x);
                scaledPadding.right = padding.right * (texture.getSize().x / texture.getImageSize().x);
                scaledPadding.top = padding.top * (texture.getSize().x / texture.getImageSize().x);
                scaledPadding.bottom = padding.bottom * (texture.getSize().x / texture.getImageSize().x);
                break;

            case Texture::ScalingType::NineSliceScaling:
                break;
            }
        }

        return scaledPadding;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::shared_ptr<WidgetRenderer> EditBoxRenderer::clone(Widget* widget)
    {
        auto renderer = std::shared_ptr<EditBoxRenderer>(new EditBoxRenderer{*this});
        renderer->m_editBox = static_cast<EditBox*>(widget);
        return renderer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
