#include "InputClass.h"

InputClass::InputClass() {
}

InputClass::InputClass(const InputClass& other) {
}

InputClass::~InputClass() {
}

void InputClass::initialize() {
    int i;

    for (i = 0; i < 256; i++) {
        m_keys[i] = false;
    }
}

void InputClass::key_down(unsigned int input) {
    m_keys[input] = true;
}

void InputClass::key_up(unsigned int input) {
    m_keys[input] = false;
}

bool InputClass::is_key_down(unsigned int key) {
    return m_keys[key];
}
