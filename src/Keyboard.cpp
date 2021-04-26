/**
 * @file Keyboard.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-04-26
 */

#include "Keyboard.h"

namespace rose {

    void Keyboard::addedToContainer() {
        Node::addedToContainer();
        auto grid = getNode<Grid>();
        for( auto& key : keyDataRow0) {
            if (key[0] >= '@')
                grid << wdg<LetterKey>(key);
        }
    }
}
