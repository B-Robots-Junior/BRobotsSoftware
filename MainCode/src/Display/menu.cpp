#include <Display/menu.h>
#include <Display/element.h>

// ----------------------------------------------------------------------------------------------------
// menu class
// ----------------------------------------------------------------------------------------------------

// --------------------------------------------------
// public methods
// --------------------------------------------------

void Menu::runCurrSelectable(Display* parent) {
    _selectables[selectedIndex]->run(parent, this);
}

void Menu::selectNextBy(int8_t amount) {
    selectedIndex = wrapT<int>(selectedIndex + amount, 0, _selectables.size());
}

void Menu::update(Display* parent) {
    if (_inputHandler != nullptr)
        _inputHandler->update(parent, this);
}

void Menu::draw(Display* parent) {
    for (unsigned int i = 0; i < _elements.size(); i++)
        _elements[i]->draw(parent);
    for (unsigned int i = 0; i < _selectables.size(); i++)
        _selectables[i]->draw(i == selectedIndex, parent);
}

void Menu::removeElement(Element* element) {
    for (unsigned int i = 0; i < _elements.size(); i++) {
        if (_elements[i] != element) continue;
        _elements.pop(i);
        return;
    }
}

void Menu::removeSelectable(Selectable* element) {
    for (unsigned int i = 0; i < _selectables.size(); i++) {
        if (_selectables[i] != element) continue;
        _selectables.pop(i);
        return;
    }
}