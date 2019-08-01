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
template<typename T, typename Contain, typename Float = float>
class Quadtree
{
    static_assert(std::is_convertible_v<
        decltype(std::declval<Contain>()(math::Box<Float>(), std::declval<T>())), bool>,
        "Contain must be a callable of signature bool(const math::Box<Float>&, const T&)");
    static_assert(std::is_arithmetic_v<Float>);

public:
    Quadtree(const math::Box<Float>& box, const Contain& contain = Contain()) :
        mBox(box), mRoot(std::make_unique<Node>()), mContain(contain)
    {

    }

    void add(const T& value)
    {
        add(mRoot.get(), 0, mBox, value);
    }

    void remove(const T& value)
    {
        remove(mRoot.get(), nullptr, mBox, value);
    }

    template<typename Intersect>
    std::vector<T> query(const Intersect& intersect, const math::Box<Float>& box) const
    {
        static_assert(std::is_convertible_v<
            decltype(std::declval<Intersect>()(math::Box<Float>(), std::declval<T>())), bool>,
            "Intersect must be a callable of signature bool(const math::Box<Float>&, const T&)");
        auto values = std::vector<T>();
        query(intersect, mRoot.get(), mBox, box, values);
        return values;
    }

    template<typename Intersect>
    std::vector<std::pair<T, T>> findAllIntersections(const Intersect& intersect) const
    {
        static_assert(std::is_convertible_v<
            decltype(std::declval<Intersect>()(std::declval<T>(), std::declval<T>())), bool>,
            "Intersect must be a callable of signature bool(const T&, const T&)");
        auto intersections = std::vector<std::pair<T, T>>();
        findAllIntersections(intersect, mRoot.get(), intersections);
        return intersections;
    }

private:
    static constexpr auto Threshold = std::size_t(16);
    static constexpr auto MaxDepth = std::size_t(8);

    struct Node
    {
        std::array<std::unique_ptr<Node>, 4> children;
        std::vector<T> values;
    };

    math::Box<Float> mBox;
    std::unique_ptr<Node> mRoot;
    Contain mContain;

    bool isLeaf(const Node* node) const
    {
        return !static_cast<bool>(node->children[0]);
    }

    math::Box<Float> computeBox(const math::Box<Float>& box, std::size_t i) const
    {
        auto origin = box.getTopLeft();
        auto childSize = box.getSize() / static_cast<Float>(2);
        switch (i)
        {
            case 0:
                return math::Box<Float>(origin, childSize);
            case 1:
                return math::Box<Float>(math::Vector2<Float>(origin.x + childSize.x, origin.y), childSize);
            case 2:
                return math::Box<Float>(math::Vector2<Float>(origin.x, origin.y + childSize.y), childSize);
            case 3:
                return math::Box<Float>(origin + childSize, childSize);
            default:
                assert(false && "Invalid child index");
                return math::Box<Float>();
        }
    }

    void add(Node* node, std::size_t depth, const math::Box<Float>& box, const T& value)
    {
        assert(node != nullptr);
        assert(mContain(box, value));
        if (isLeaf(node))
        {
            // Insert the value in this node if possible
            if (depth >= MaxDepth || node->values.size() < Threshold)
                node->values.push_back(value);
            // Otherwise, we split and we try again
            else
            {
                split(node, box);
                add(node, depth, box, value);
            }
        }
        else
        {
            // Add the value in a child if the value is entirely contained in it
            for (auto i = std::size_t(0); i < node->children.size(); ++i)
            {
                auto childBox = computeBox(box, i);
                if (mContain(childBox, value))
                {
                    add(node->children[i].get(), depth + 1, childBox, value);
                    return;
                }
            }
            // Otherwise, we add the value in the current node
            node->values.push_back(value);
        }
    }

    void split(Node* node, const math::Box<Float>& box)
    {
        assert(node != nullptr);
        assert(isLeaf(node) && "Only leaves can be split");
        // Create children
        // MAYBE: seems to be faster to unroll the loops
        for (auto& child : node->children)
            child = std::make_unique<Node>();
        // Assign values to children
        auto newValues = std::vector<T>(); // New values for this node
        auto childBoxes = std::array<math::Box<Float>, 4>();
        for (auto i = std::size_t(0); i < node->children.size(); ++i)
            childBoxes[i] = computeBox(box, i);
        for (const auto& value : node->values)
        {
            auto inserted = false;
            for (auto i = std::size_t(0); i < node->children.size(); ++i)
            {
                if (mContain(childBoxes[i], value))
                {
                    node->children[i]->values.push_back(value);
                    inserted = true;
                    break;
                }
            }
            if (!inserted)
                newValues.push_back(value);
        }
        node->values = std::move(newValues);
    }

    void remove(Node* node, Node* parent, const math::Box<Float>& box, const T& value)
    {
        assert(node != nullptr);
        assert(mContain(box, value));
        if (isLeaf(node))
        {
            // Remove the value from node
            node->values.erase(std::find(std::begin(node->values), std::end(node->values), value));
            // Try to merge the parent
            if (parent != nullptr)
                tryMerge(parent);
        }
        else
        {
            // Remove the value in a child if the value is entirely contained in it
            for (auto i = std::size_t(0); i < node->children.size(); ++i)
            {
                auto childBox = computeBox(box, i);
                if (mContain(childBox, value))
                {
                    remove(node->children[i].get(), node, childBox, value);
                    return;
                }
            }
            // Otherwise, we remove the value from the current node
            node->values.erase(std::find(std::begin(node->values), std::end(node->values), value));
        }
    }

    void tryMerge(Node* node)
    {
        assert(node != nullptr);
        assert(!isLeaf(node) && "Only interior nodes can be merged");
        auto nbValues = node->values.size();
        // MAYBE: unroll the loop
        for (const auto& child : node->children)
        {
            if (!isLeaf(child.get()))
                return;
            nbValues += child->values.size();
        }
        if (nbValues <= Threshold)
        {
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

    template<typename Intersect>
    void query(const Intersect& intersect, Node* node, const math::Box<Float>& box,
        const math::Box<Float>& queryBox, std::vector<T>& values) const
    {
        assert(node != nullptr);
        assert(queryBox.intersects(box));
        for (const auto& value : node->values)
        {
            if (intersect(queryBox, value))
                values.push_back(value);
        }
        if (!isLeaf(node))
        {
            for (auto i = std::size_t(0); i < node->children.size(); ++i)
            {
                auto childBox = computeBox(box, i);
                if (queryBox.intersects(childBox))
                    query(intersect, node->children[i].get(), childBox, queryBox, values);
            }
        }
    }

    template<typename Intersect>
    void findAllIntersections(const Intersect& intersect, Node* node,
        std::vector<std::pair<T, T>>& intersections) const
    {
        // Find intersections between values stored in this node
        // Make sure to not report the same intersection twice
        for (auto i = std::size_t(0); i < node->values.size(); ++i)
        {
            for (auto j = std::size_t(0); j < i; ++j)
            {
                if (intersect(node->values[i], node->values[j]))
                    intersections.emplace_back(node->values[i], node->values[j]);
            }
        }
        if (!isLeaf(node))
        {
            // Values in this node can intersect values in descendants
            for (const auto& child : node->children) // TODO: try to swap the two loops
            {
                for (const auto& value : node->values)
                    findIntersectionsInChildren(intersect, child.get(), value, intersections);
            }
            // Find intersections in children
            for (const auto& child : node->children)
                findAllIntersections(intersect, child.get(), intersections);
        }
    }

    template<typename Intersect>
    void findIntersectionsInChildren(const Intersect& intersect, Node* node, const T& value,
        std::vector<std::pair<T, T>>& intersections) const
    {
        // Test against the values stored in this node
        for (const auto& other : node->values)
        {
            if (intersect(value, other))
                intersections.emplace_back(value, other);
        }
        // Test against values stored into descendants of this node
        if (!isLeaf(node))
        {
            for (const auto& child : node->children)
                findIntersectionsInChildren(intersect, child.get(), value, intersections);
        }
    }
};

}