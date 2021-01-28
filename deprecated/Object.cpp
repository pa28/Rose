//
// Created by richard on 2020-10-20.
//

#include "Object.h"
#include <stdexcept>

namespace rose {
    void Object::decRef(bool dealloc) const noexcept {
        --m_refCount;
        if (m_refCount == 0 && dealloc) {
            delete this;
        } else if (m_refCount < 0) {
            fprintf(stderr, "Internal error: Object reference count < 0!\n");
            abort();
        }
    }
}

