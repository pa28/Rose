/**
 * @file StructuredTypes.h
 * @author Richard Buckley <richard.buckley@ieee.org>
 * @version 1.0
 * @date 2021-03-04
 * @brief Classes and functions used to implement structured data collections.
 */

#pragma once

#include <algorithm>
#include <exception>
#include <memory>
#include <vector>

namespace rose {

    class NodeTypeError : std::runtime_error {
    public:
        explicit NodeTypeError(const char *what) : std::runtime_error(what) {}
        explicit NodeTypeError(const std::string &what) : std::runtime_error(what) {}
        NodeTypeError(const NodeTypeError &other) noexcept = default;
    };

    class Container;

    /**
     * @class Node
     * @brief A class to define a polymorphic member in an N-Array tree.
     */
    class Node : public std::enable_shared_from_this<Node> {
    public:
        friend class Container;
        using NodePtr = std::shared_ptr<Node>;

    protected:
        /// The Container that holds the Node.
        std::weak_ptr<Container> mContainer{};

    public:
        Node() = default;

        /**
         * @brief Create a node and add it to a Container.
         * @param container
         */
        explicit Node(const std::shared_ptr<Container> &container);

        virtual ~Node() = default;

        /**
         * @brief Create a Node with no arguments.
         * @tparam NodeType The type of Node to create.
         * @return A std::shared_ptr to the NodeType created.
         */
        template<typename NodeType>
        static std::shared_ptr<NodeType> create() {
            return std::make_shared<NodeType>();
        }

        /**
         * @brief Create a Node with arguments.
         * @tparam NodeType The type of Node to create.
         * @tparam Args The argument parameter pack type.
         * @param args The argument parameter pack data.
         * @return A std::shared_ptr to the NodeType created.
         */
        template<typename NodeType, typename ... Args>
        static std::shared_ptr<NodeType> create(Args ... args) {
            return std::make_shared<NodeType>(args ...);
        }

        /**
         * @brief Create a Node with arguments and add to a Container.
         * @tparam NodeType The type of Node to create.
         * @tparam ContainerType The type of Container the Node is added to.
         * @tparam Args The argument parameter pack type.
         * @param container The Container.
         * @param args The argument parameter pack data.
         * @return A std::shared_ptr to the NodeType created.
         */
        template<typename NodeType, typename ContainerType, typename ... Args>
        static std::shared_ptr<NodeType> add(ContainerType container, Args ... args) {
            static_assert(std::is_base_of_v<std::shared_ptr<Container>, ContainerType>,
                          "ContainerType must be derived from Container.");
            auto node = std::make_shared<NodeType>(args ...);
            node->mContainer = container;
            container->add(node);
            return node;
        }

        /**
         * @brief Get a const std::shared_ptr to this node.
         * @return std::shared_ptr<const Node>
         */
        std::shared_ptr<const Node> getNode() const {
            return shared_from_this();
        }

        /**
         * @brief Get a std::shared_ptr to this node.
         * @return std::shared_ptr<const Node>
         */
        std::shared_ptr<Node> getNode() {
            return shared_from_this();
        }

        /**
         * @brief Get a std::shared_ptr<const NodeType> to this node.
         * @tparam NodeType
         * @return std::shared_ptr<const NodeType> which will be empty if this Node is not a NodeType or
         * derived from NodeType.
         */
        template<typename NodeType>
        std::shared_ptr<const Node> getNode() const {
            return std::dynamic_pointer_cast<NodeType>(shared_from_this());
        }

        /**
         * @brief Get a std::shared_ptr<NodeType> to this node.
         * @tparam NodeType
         * @return std::shared_ptr<NodeType> which will be empty if this Node is not a NodeType or
         * derived from NodeType.
         */
        template<typename NodeType>
        std::shared_ptr<NodeType> getNode() {
            return std::dynamic_pointer_cast<NodeType>(shared_from_this());
        }

        std::shared_ptr<Node> remove();
    };

    /**
     * @class Container
     * @brief A container which holds the subordinate Node objects.
     */
    class Container : public Node, public std::vector<Node::NodePtr> {
    public:
        ~Container() override = default;

        virtual void add(const std::shared_ptr<Node> &node) {
            if (auto container = getNode<Container>(); container) {
                container->push_back(node);
                node->mContainer = container;
            }
        }

        void remove(const std::shared_ptr<Node>& node) {
            erase(std::remove(begin(), end(), node));
        }
    };

    inline std::shared_ptr<Node> Node::remove() {
        auto node = shared_from_this();
        if (auto container = mContainer.lock(); container) {
            container->remove(node);
        }
        return node;
    }

}
