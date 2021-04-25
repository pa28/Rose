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
#include <iostream>
#include <sstream>

namespace rose {

    class NodeTypeError : std::runtime_error {
    public:
        explicit NodeTypeError(const char *what) : std::runtime_error(what) {}
        explicit NodeTypeError(const std::string &what) : std::runtime_error(what) {}
        NodeTypeError(const NodeTypeError &other) noexcept = default;
    };

    class NodeRangeError : std::out_of_range {
    public:
        explicit NodeRangeError(const char *what) : std::out_of_range(what) {}

        explicit NodeRangeError(const std::string &what) : std::out_of_range(what) {}

        NodeRangeError(const NodeRangeError &other) noexcept = default;
    };

    class Container;

    /// A path of objects by Id.
    template<typename S>
    struct IdPath : public std::vector<S> {
        bool absolutePath{};
        static constexpr char PathSep = '/';
        static constexpr std::string_view PathParent = "..";

        explicit IdPath(bool absolute = false) : std::vector<S>(), absolutePath(absolute) {}

        explicit IdPath(const S element, bool absolute = false) : IdPath(absolute) {
            emplace_back(element);
        }

        [[nodiscard]] std::string str() const {
            bool notFirst = false;
            std::stringstream strm{};
            for (auto &element : *this) {
                if (absolutePath || notFirst) {
                    strm << PathSep;
                    notFirst = true;
                }
                strm << element;
            }
            return strm.str();
        }
    };

    template<typename I>
    inline IdPath<std::string> parsePathIdString(I inputString) {
        std::stringstream strm{inputString};
        IdPath<std::string> idPath{};
        std::string tmp;
        while (std::getline(strm, tmp, IdPath<std::string>::PathSep)) {
            if (idPath.empty() && tmp.empty())
                idPath.absolutePath = true;
            else if (!tmp.empty())
                idPath.emplace_back(tmp);
        }
        return idPath;
    }

    /**
     * @brief A type to specify an Id value.
     */
    struct Id {
        std::string_view idString;

        constexpr explicit operator bool() const noexcept {
            return !idString.empty();
        }
    };

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

        ///< The object Id string.
        Id mId{};

    public:
        Node() = default;

        /**
         * @brief Create a node and add it to a Container.
         * @param container
         */
        explicit Node(const std::shared_ptr<Container> &container);

        virtual ~Node() = default;

        static constexpr std::string_view id = "Node";
        virtual std::string_view nodeId() const noexcept {
            return id;
        }

        /// Set Id
        void setId(const Id& nodeId) noexcept {
            mId = nodeId;
        }

        /// Get Id
        std::string_view getId() const {
            if (mId)
                return mId.idString;
            return nodeId();
        }

        /// Get the Id Path.
        template<typename S = std::string_view>
        IdPath<S> getIdPath() const;

        /**
         * @brief Called when a Node is added to a Container.
         * @details By default it is ignored but can be used to perform initialization that is required
         * when a Node is placed in a Container.
         */
        virtual void addedToContainer() {
        }

        std::shared_ptr<Container> container() {
            if (mContainer.expired())
                return nullptr;
            return mContainer.lock();
        }

        std::shared_ptr<Container> container() const {
            if (mContainer.expired())
                return nullptr;
            return mContainer.lock();
        }

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

        static constexpr std::string_view id = "Container";
        std::string_view nodeId() const noexcept override {
            return id;
        }

        virtual void add(const std::shared_ptr<Node> &node) {
            if (auto container = getNode<Container>(); container) {
                container->push_back(node);
                node->mContainer = container;
                node->addedToContainer();
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

    template<typename S>
    inline IdPath<S> Node::getIdPath() const {
        static_assert(std::is_base_of_v<std::string,S> || std::is_base_of_v<std::string_view,S>,
                "Only std::string or std::string_view supported for getIdPath().");
        auto node = container();
        auto idPath = node ? node->getIdPath<S>() : IdPath<S>{true};
        idPath.emplace_back(getId());
        return idPath;
    }
}

/**
 * @brief An insertion operator to set the Id of a Widget.
 * @tparam NodeClass The class of the Widget.
 * @param widget The Widget.
 * @param id The Id.
 * @return The Widget.
 */
template<class NodeClass>
inline std::shared_ptr<NodeClass> operator<<(std::shared_ptr<NodeClass> widget, const rose::Id& id) {
    static_assert(std::is_base_of_v<rose::Node, NodeClass>, "WidgetClass must be derived from rose::Node.");
    widget->setId(id);
    return widget;
}

template<typename S>
inline std::ostream& operator<<(std::ostream& ostrm, const rose::IdPath<S>& idPath) {
    return ostrm << idPath.str();
}