/**
 * @file StructuredTypes.cpp
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 */

#include "StructuredTypes.h"

namespace rose {

    Node::Node(const std::shared_ptr<Container> &container) {
        mContainer = container;
    }

}
