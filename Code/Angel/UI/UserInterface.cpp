//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2008-2012, Shane Liesegang
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the copyright holder nor the names of any
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../UI/UserInterface.h"

#include "Gwen/Gwen.h"
#include "Gwen/Controls/Canvas.h"
#include "Gwen/Controls/Button.h"
#include "Gwen/Controls/GroupBox.h"

#if defined(__APPLE__)
	// included files from gwen trigger Clang/GCC warnings
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdangling-else"
	#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
		#include "Gwen/Skins/TexturedBase.h"
#if defined(__APPLE__)
	#pragma GCC diagnostic pop
#endif

#include "../UI/GwenRenderer.h"
#include "../Infrastructure/Log.h"
#include "../Infrastructure/Camera.h"
#include "../Infrastructure/World.h"


typedef std::map<AngelUIHandle, void (*)()> EventMapping;

class EventHandler : public Gwen::Event::Handler
{
public:
	void AddCallback(Gwen::Controls::Base* control, void (*callback)())
	{
		_callbacks[control] = callback;
	}
	
	void OnPress(Gwen::Controls::Base* control)
	{
		Check(control);
	}
	
private:
	void Check(Gwen::Controls::Base* control)
	{
		EventMapping::iterator it = _callbacks.find(control);
		if (it != _callbacks.end())
		{
			it->second();
		}
	}
	
	EventMapping _callbacks;
};


namespace 
{
	Gwen::Controls::Canvas* AngelCanvas;
	Gwen::Skin::Base* AngelSkin;
	
	EventHandler handler;
}

UserInterface* UserInterface::s_UserInterface = NULL;

UserInterface& UserInterface::GetInstance()
{
	if (s_UserInterface == NULL)
	{
		s_UserInterface = new UserInterface();
	}
	return *s_UserInterface;
}

UserInterface::UserInterface()
{
	glfwGetMousePos(&_mousePosition.X, &_mousePosition.Y);
	_renderer = new GwenRenderer();

	AngelSkin = new Gwen::Skin::TexturedBase(_renderer);
	((Gwen::Skin::TexturedBase*)AngelSkin)->Init("Resources/Images/DefaultSkin.png");
	AngelSkin->SetDefaultFont(Gwen::Utility::StringToUnicode("Resources/Fonts/Inconsolata.otf"), 20);
	
	AngelCanvas = new Gwen::Controls::Canvas(AngelSkin);
	AngelCanvas->SetSize(1024, 768); // should be size of window (update when change)
}

UserInterface::~UserInterface()
{
	
}

void UserInterface::Shutdown()
{
	delete AngelCanvas;
	delete AngelSkin;
	
	delete _renderer;
}

void UserInterface::Render()
{
	AngelCanvas->DoThink();
	AngelCanvas->RenderCanvas();
}

void UserInterface::MouseMotionEvent(Vec2i screenCoordinates)
{
    if (theWorld.GetConsole()->IsEnabled())
    {
        return;
    }
    
	int deltaX = _mousePosition.X - screenCoordinates.X;
	int deltaY = _mousePosition.Y - screenCoordinates.Y;
	_mousePosition.X = screenCoordinates.X;
	_mousePosition.Y = screenCoordinates.Y;
	AngelCanvas->InputMouseMoved(screenCoordinates.X, screenCoordinates.Y, deltaX, deltaY);
}

void UserInterface::MouseDownEvent(Vec2i screenCoordinates, MouseButtonInput button)
{
    if (theWorld.GetConsole()->IsEnabled())
    {
        return;
    }
    
	AngelCanvas->InputMouseButton(button, true);
}

void UserInterface::MouseUpEvent(Vec2i screenCoordinates, MouseButtonInput button)
{
    if (theWorld.GetConsole()->IsEnabled())
    {
        return;
    }
    
	AngelCanvas->InputMouseButton(button, false);
}

void UserInterface::MouseWheelEvent(int position)
{
    if (theWorld.GetConsole()->IsEnabled())
    {
        return;
    }
    
	AngelCanvas->InputMouseWheel(position);
}

void UserInterface::RemoveUIElement(AngelUIHandle element)
{
    std::set<AngelUIHandle>::iterator it = _elements.find(element);
    if (it != _elements.end())
    {
        delete (Gwen::Controls::Base*)(*it);
        _elements.erase(it);
    }
}

AngelUIHandle UserInterface::AddButton(const String& label, Vec2i position, void (*callback)(), const String& font, Vec2i padding)
{
	Gwen::Controls::Button* button = new Gwen::Controls::Button(AngelCanvas);
	if (font != "")
	{
		button->SetFont(Gwen::Utility::StringToUnicode(font), 20, false);
	}
	button->SetText(label);
	button->SetPadding(Gwen::Padding(padding.X, padding.Y, padding.X, padding.Y));
	button->SizeToContents();
	button->SetPos(position.X, position.Y);
    
	button->onPress.Add(&handler, &EventHandler::OnPress);
	handler.AddCallback(button, callback);
	
    _elements.insert(button);
    
	return button;
}

AngelUIHandle UserInterface::ShowChoiceBox(const StringList& labels, Vec2i position, void (*callback)(int), const String& font, bool modal)
{
	Gwen::Controls::GroupBox* box = new Gwen::Controls::GroupBox(AngelCanvas);
	
	Gwen::TextObject fontTextObject = Gwen::Utility::StringToUnicode(font);

	for	(unsigned int i=0; i < labels.size(); i++)
	{
		Gwen::Controls::Button* button = new Gwen::Controls::Button(box);
		if (font != "")
		{
			button->SetFont(fontTextObject, 20, false);
		}
		button->SetText(labels[i]);
		button->SizeToContents();
	}

	box->SetPos(position.X, position.Y);
	box->SizeToContents();

	_elements.insert(box);

	return box;
}
