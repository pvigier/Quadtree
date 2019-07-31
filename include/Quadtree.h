#pragma once

#include <cassert>
#include <algorithm>
#include <memory>
#include <type_traits>
#include <vector>
#include "Box.h"

namespace ds
{

// MAYBE: add Equal type because it is used in std::find during removal
template<typename T, typename Contain, typename Intersect, typename Float = float>
class Quadtree
{
    static_assert(std::is_convertible_v<
        decltype(std::declval<Contain>()(math::Box<Float>(), std::declval<T>())), bool>,
        "Contain must be a callable of signature bool(const math::Box<Float>&, const T&)");
    static_assert(std::is_convertible_v<
        decltype(std::declval<Intersect>()(math::Box<Float>(), std::declval<T>())), bool>,
        "Intersect must be a callable of signature bool(const math::Box<Float>&, const T&)");
    static_assert(std::is_arithmetic_v<Float>);

public:
    Quadtree(const math::Box<Float>& box, const Contain& contain = Contain(),
        const Intersect& intersect = Intersect()) :
        mRoot(std::make_unique<Node>(box)), mContain(contain), mIntersect(intersect)
    {

    }

    void add(const T& value)
    {
        add(mRoot.get(), value);
    }

    void remove(const T& value)
    {
        remove(mRoot.get(), value);
    }

    std::vector<T> query(const math::Box<Float>& box) const
    {
        auto values = std::vector<T>();
        query(mRoot.get(), box, values);
        return values;
    }

private:
    static constexpr auto Threshold = std::size_t(16);
    static constexpr auto MaxDepth = std::size_t(8);

    class Node
    {
    public:
        enum Type
        {
            Interior,
            Leaf
        };

        Type type = Type::Leaf;
        const math::Box<Float> box;
        const std::size_t depth;
        Node* const parent;
        std::array<std::unique_ptr<Node>, 4> children;
        std::vector<T> values;

        Node(const math::Box<Float>& b, std::size_t d = 0, Node* p = nullptr) :
            box(b), depth(d), parent(p)
        {

        }
    };

    std::unique_ptr<Node> mRoot;
    Contain mContain;
    Intersect mIntersect;

    void add(Node* node, const T& value)
    {
        assert(node != nullptr);
        assert(mContain(node->box, value));
        switch (node->type)
        {
            case Node::Type::Leaf:
                // Insert the value in this node if possible
                if (node->depth >= MaxDepth || node->values.size() < Threshold)
                    node->values.push_back(value);
                // Otherwise, we split and we try again
                else
                {
                    split(node);
                    add(node, value);
                }
                return;
            case Node::Type::Interior:
                // Add the value in a child if the value is entirely contained in it
                for (const auto& child : node->children)
                {
                    if (mContain(child->box, value))
                    {
                        add(child.get(), value);
                        return;
                    }
                }
                // Otherwise, we add the value in the current node
                node->values.push_back(value);
                return;
            default:
                assert(false && "Invalid node type");
                return;
        }
    }

    void split(Node* node)
    {
        assert(node != nullptr);
        assert(node->type == Node::Type::Leaf && "Only leaves can be split");
        node->type = Node::Type::Interior;
        // Create children
        auto origin = node->box.getTopLeft();
        auto childSize = node->box.getSize() / static_cast<Float>(2);
        node->children[0] = std::make_unique<Node>(
            math::Box<Float>(origin, childSize),
            node->depth + 1,
            node);
        node->children[1] = std::make_unique<Node>(
            math::Box<Float>(math::Vector2<Float>(origin.x + childSize.x, origin.y), childSize),
            node->depth + 1,
            node);
        node->children[2] = std::make_unique<Node>(
            math::Box<Float>(math::Vector2<Float>(origin.x, origin.y + childSize.y), childSize),
            node->depth + 1,
            node);
        node->children[3] = std::make_unique<Node>(
            math::Box<Float>(origin + childSize, childSize),
            node->depth + 1,
            node);
        // Assign values to children
        auto newValues = std::vector<T>(); // New values for this node
        for (const auto& value : node->values)
        {
            auto inserted = false;
            for (auto& child : node->children)
            {
                if (mContain(child->box, value))
                {
                    child->values.push_back(value);
                    inserted = true;
                    break;
                }
            }
            if (!inserted)
                newValues.push_back(value);
        }
        node->values = std::move(newValues);
    }

    void remove(Node* node, const T& value)
    {
        assert(node != nullptr);
        assert(mContain(node->box, value));
        switch (node->type)
        {
            case Node::Type::Leaf:
                // Remove the value from node
                node->values.erase(std::find(std::begin(node->values), std::end(node->values), value));
                // Try to merge the parent
                if (node->parent != nullptr)
                    tryMerge(node->parent);
                return;
            case Node::Type::Interior:
                // Remove the value in a child if the value is entirely contained in it
                for (const auto& child : node->children)
                {
                    if (mContain(child->box, value))
                    {
                        remove(child.get(), value);
                        return;
                    }
                }
                // Otherwise, we remove the value from the current node
                node->values.erase(std::find(std::begin(node->values), std::end(node->values), value));
                return;
            default:
                assert(false && "Invalid node type");
                return;
        }
    }

    void tryMerge(Node* node)
    {
        assert(node != nullptr);
        assert(node->type == Node::Type::Interior && "Only interior nodes can be merged");
        auto nbValues = node->values.size();
        // MAYBE: unroll the loop
        for (const auto& child : node->children)
        {
            if (!child->type == Node::Type::Leaf)
                return;
            nbValues += child->values.size();
        }
        if (nbValues <= Threshold)
        {
            node->type = Node::Type::Leaf;
            node->values.reserve(nbValues);
            // Merge the values of all the children
            for (const auto& child : node->children)
            {
                for (const auto& value : child->values)
                    node->values.push_back(value);
            }
            // Remove the children
            for (auto& child : node->children)
                child.reset();
        }
    }

    void query(Node* node, const math::Box<Float>& box, std::vector<T>& values) const
    {
        assert(node != nullptr);
        assert(box.intersects(node->box));
        for (const auto& value : node->values)
        {
            if (mIntersect(box, value))
                values.push_back(value);
        }
        switch (node->type)
        {
            case Node::Type::Leaf:
                return;
            case Node::Type::Interior:
                for (const auto& child : node->children)
                {
                    if (box.intersects(child->box))
                        query(child.get(), box, values);
                }
                return;
            default:
                assert(false && "Invalid node type");
                break;
        }
    }
};

}