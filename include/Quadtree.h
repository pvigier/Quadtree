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
template<typename T, typename GetBox, typename Equal = std::equal_to<T>, typename Float = float>
class Quadtree
{
    static_assert(std::is_convertible_v<
        decltype(std::declval<GetBox>()(std::declval<T>())), math::Box<Float>>,
        "GetBox must be a callable of signature math::Box<Float>(const T&)");
    static_assert(std::is_convertible_v<
        decltype(std::declval<Equal>()(std::declval<T>(), std::declval<T>())), bool>,
        "Equal must be a callable of signature bool(const T&, const T&)");
    static_assert(std::is_arithmetic_v<Float>);

public:
    Quadtree(const math::Box<Float>& box, const GetBox& getBox = GetBox(),
        const Equal& equal = Equal()) :
        mBox(box), mRoot(std::make_unique<Node>()), mGetBox(getBox), mEqual(equal)
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

    std::vector<T> query(const math::Box<Float>& box) const
    {
        auto values = std::vector<T>();
        query(mRoot.get(), mBox, box, values);
        return values;
    }

    std::vector<std::pair<T, T>> findAllIntersections() const
    {
        auto intersections = std::vector<std::pair<T, T>>();
        findAllIntersections(mRoot.get(), intersections);
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
    GetBox mGetBox;
    Equal mEqual;

    bool isLeaf(const Node* node) const
    {
        return !static_cast<bool>(node->children[0]);
    }

    math::Box<Float> computeBox(const math::Box<Float>& box, int i) const
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

    int getQuadrant(const math::Box<Float>& nodeBox, const math::Box<Float>& valueBox) const
    {
        // TODO: compute center
        auto center = nodeBox.getCenter();
        if (valueBox.getRight() < center.x)
        {
            if (valueBox.getBottom() < center.y)
                return 0;
            else if (valueBox.top >= center.y)
                return 2;
            else
                return -1;
        }
        else if (valueBox.left >= center.x)
        {
            if (valueBox.getBottom() < center.y)
                return 1;
            else if (valueBox.top >= center.y)
                return 3;
            else
                return -1;
        }
        else
            return -1;
    }

    void add(Node* node, std::size_t depth, const math::Box<Float>& box, const T& value)
    {
        assert(node != nullptr);
        assert(box.contains(mGetBox(value)));
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
            auto i = getQuadrant(box, mGetBox(value));
            // Add the value in a child if the value is entirely contained in it
            if (i != -1)
                add(node->children[static_cast<std::size_t>(i)].get(), depth + 1, computeBox(box, i), value);
            // Otherwise, we add the value in the current node
            else
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
        for (const auto& value : node->values)
        {
            auto i = getQuadrant(box, mGetBox(value));
            if (i != -1)
                node->children[static_cast<std::size_t>(i)]->values.push_back(value);
            else
                newValues.push_back(value);
        }
        node->values = std::move(newValues);
    }

    void remove(Node* node, Node* parent, const math::Box<Float>& box, const T& value)
    {
        assert(node != nullptr);
        assert(box.contains(mGetBox(value)));
        if (isLeaf(node))
        {
            // Remove the value from node
            removeValue(node, value);
            // Try to merge the parent
            if (parent != nullptr)
                tryMerge(parent);
        }
        else
        {
            // Remove the value in a child if the value is entirely contained in it
            auto i = getQuadrant(box, mGetBox(value));
            if (i != -1)
                remove(node->children[static_cast<std::size_t>(i)].get(), node, computeBox(box, i), value);
            // Otherwise, we remove the value from the current node
            else
                removeValue(node, value);
        }
    }

    void removeValue(Node* node, const T& value)
    {
        // Find the value in node->values
        auto it = std::find_if(std::begin(node->values), std::end(node->values),
            [this, &value](const auto& rhs){ return mEqual(value, rhs); });
        assert(it != std::end(node->values) && "Trying to remove a value thas is not present in the node");
        // Swap with the last element and pop back
        *it = std::move(node->values.back());
        node->values.pop_back();
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

    void query(Node* node, const math::Box<Float>& box, const math::Box<Float>& queryBox, std::vector<T>& values) const
    {
        assert(node != nullptr);
        assert(queryBox.intersects(box));
        for (const auto& value : node->values)
        {
            if (queryBox.intersects(mGetBox(value)))
                values.push_back(value);
        }
        if (!isLeaf(node))
        {
            for (auto i = std::size_t(0); i < node->children.size(); ++i)
            {
                auto childBox = computeBox(box, static_cast<int>(i));
                if (queryBox.intersects(childBox))
                    query(node->children[i].get(), childBox, queryBox, values);
            }
        }
    }

    void findAllIntersections(Node* node, std::vector<std::pair<T, T>>& intersections) const
    {
        // Find intersections between values stored in this node
        // Make sure to not report the same intersection twice
        for (auto i = std::size_t(0); i < node->values.size(); ++i)
        {
            for (auto j = std::size_t(0); j < i; ++j)
            {
                if (mGetBox(node->values[i]).intersects(mGetBox(node->values[j])))
                    intersections.emplace_back(node->values[i], node->values[j]);
            }
        }
        if (!isLeaf(node))
        {
            // Values in this node can intersect values in descendants
            for (const auto& child : node->children) // TODO: try to swap the two loops
            {
                for (const auto& value : node->values)
                    findIntersectionsInChildren(child.get(), value, intersections);
            }
            // Find intersections in children
            for (const auto& child : node->children)
                findAllIntersections(child.get(), intersections);
        }
    }

    void findIntersectionsInChildren(Node* node, const T& value, std::vector<std::pair<T, T>>& intersections) const
    {
        // Test against the values stored in this node
        for (const auto& other : node->values)
        {
            if (mGetBox(value).intersects(mGetBox(other)))
                intersections.emplace_back(value, other);
        }
        // Test against values stored into descendants of this node
        if (!isLeaf(node))
        {
            for (const auto& child : node->children)
                findIntersectionsInChildren(child.get(), value, intersections);
        }
    }
};

}