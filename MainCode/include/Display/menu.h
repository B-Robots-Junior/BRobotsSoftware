#ifndef _MENU_H_
#define _MENU_H_

#include <Arduino.h>
#include <pos.h>
#include <array.h>
#include <Display/display.h>
//#include <element.h>
#include <Display/inputHandler.h>

class Display;
class Element;
class Selectable;
class InputHandler;

template <typename T>
T wrapT(T val, T min_val, T max_val) {
    T range = max_val - min_val;
    while (val < min_val)
        val += range;
    while (val >= max_val)
        val -= range;
    return val;
}

class Menu {
private:
    Array<Element*> _elements;
    Array<Selectable*> _selectables;
    InputHandler* _inputHandler;

public:
    Menu(const Array<Element*>& elements = Array<Element*>(), const Array<Selectable*>& selectables = Array<Selectable*>(), InputHandler* inputHandler = nullptr) 
        : _elements(elements), _selectables(selectables), _inputHandler(inputHandler) {}

    unsigned int selectedIndex;

    void setInputHandler(InputHandler* inputHandler) {_inputHandler = inputHandler;}
    InputHandler* getInputHandler() {return _inputHandler;}

    void update(Display* parent);
    void draw(Display* parent);

    void runCurrSelectable(Display* parent);
    void selectNextBy(int8_t amount = 1);

    unsigned int numElements() const {return _elements.size();}
    unsigned int numSelectables() const {return _selectables.size();}

    const Array<Element*>& getElements() {return _elements;}
    const Array<Selectable*>& getSelectables() {return _selectables;}
    Element* getElement(unsigned int index) {return _elements[index];}
    Selectable* getSelectable(unsigned int index) {return _selectables[index];}

    void addElement(Element* element) {_elements.push_back(element);}
    void removeElement(Element* element);
    void addSelectable(Selectable* element) {_selectables.push_back(element);}
    void removeSelectable(Selectable* element);

    friend Display;
};

#endif