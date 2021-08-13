#include <iostream>
#include <random>
#include <benchmark/benchmark.h>
#include "Quadtree.h"

using namespace quadtree;

struct Node
{
    Box<float> box;
    std::size_t id;
};

std::vector<Node> generateRandomNodes(std::size_t n)
{
    auto generator = std::default_random_engine();
    auto originDistribution = std::uniform_real_distribution(0.0f, 1.0f);
    auto sizeDistribution = std::uniform_real_distribution(0.0f, 0.01f);
    auto nodes = std::vector<Node>(n);
    for (auto i = std::size_t(0); i < n; ++i)
    {
        nodes[i].box.left = originDistribution(generator);
        nodes[i].box.top = originDistribution(generator);
        nodes[i].box.width = std::min(1.0f - nodes[i].box.left, sizeDistribution(generator));
        nodes[i].box.height = std::min(1.0f - nodes[i].box.top, sizeDistribution(generator));
        nodes[i].id = i;
    }
    return nodes;
}

std::vector<Node*> query(const Box<float>& box, std::vector<Node>& nodes)
{
    auto intersections = std::vector<Node*>();
    for (auto& n : nodes)
    {
        if (box.intersects(n.box))
            intersections.push_back(&n);
    }
    return intersections;
}

std::vector<std::pair<Node*, Node*>> findAllIntersections(std::vector<Node>& nodes)
{
    auto intersections = std::vector<std::pair<Node*, Node*>>();
    for (auto i = std::size_t(0); i < nodes.size(); ++i)
    {
        for (auto j = std::size_t(0); j < i; ++j)
        {
            if (nodes[i].box.intersects(nodes[j].box))
                intersections.emplace_back(&nodes[i], &nodes[j]);
        }
    }
    return intersections;
}

void quadtreeBuild(benchmark::State& state)
{

    auto getBox = [](Node* node)
    {
        return node->box;
    };
    auto box = Box(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state)
    {
        auto intersections = std::vector<std::vector<Node*>>(nodes.size());
        auto quadtree = Quadtree<Node*, decltype(getBox)>(box, getBox);
        for (auto& node : nodes)
            quadtree.add(&node);
    }
}

void quadtreeQuery(benchmark::State& state)
{

    auto getBox = [](Node* node)
    {
        return node->box;
    };

    auto box = Box(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state)
    {
        auto intersections = std::vector<std::vector<Node*>>(nodes.size());
        auto quadtree = Quadtree<Node*, decltype(getBox)>(box, getBox);
        for (auto& node : nodes)
            quadtree.add(&node);
        for (const auto& node : nodes)
            intersections[node.id] = quadtree.query(node.box);
    }
}

void quadtreeFindAllIntersections(benchmark::State& state)
{

    auto getBox = [](Node* node)
    {
        return node->box;
    };
    auto box = Box(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state)
    {
        auto quadtree = Quadtree<Node*, decltype(getBox)>(box, getBox);
        for (auto& node : nodes)
            quadtree.add(&node);
        auto intersections = quadtree.findAllIntersections();
    }
}

void bruteForceQuery(benchmark::State& state)
{
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state)
    {
        auto intersections = std::vector<std::vector<Node*>>(nodes.size());
        for (const auto& node : nodes)
            intersections[node.id] = query(node.box, nodes);
    }
}

void bruteForceFindAllIntersections(benchmark::State& state)
{
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state)
    {
        auto intersections = findAllIntersections(nodes);
    }
}

BENCHMARK(quadtreeBuild)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMicrosecond);
BENCHMARK(quadtreeQuery)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMicrosecond);
BENCHMARK(quadtreeFindAllIntersections)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMicrosecond);
BENCHMARK(bruteForceQuery)->RangeMultiplier(10)->Range(100, 10000)->Unit(benchmark::kMicrosecond);
BENCHMARK(bruteForceFindAllIntersections)->RangeMultiplier(10)->Range(100, 10000)->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();