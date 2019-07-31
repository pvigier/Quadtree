#include <random>
#include <benchmark/benchmark.h>
#include "Quadtree.h"

using namespace ds;
using namespace math;

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

std::vector<Node*> computeIntersections(const math::Box<float>& box, std::vector<Node>& nodes)
{
    auto intersections = std::vector<Node*>();
    for (auto& n : nodes)
    {
        if (box.intersects(n.box))
            intersections.push_back(&n);
    }
    return intersections;
}

void quadtreeIntersections(benchmark::State& state)
{

    auto contain = [](const Box<float>& box, Node* node)
    {
        return box.contains(node->box);
    };
    auto intersect = [](const Box<float>& box, Node* node)
    {
        return box.intersects(node->box);
    };
    auto box = Box(0.0f, 0.0f, 1.0f, 1.0f);
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state)
    {
        auto intersections = std::vector<std::vector<Node*>>(nodes.size());
        auto quadtree = Quadtree<Node*, decltype(contain), decltype(intersect)>(box, contain, intersect);
        for (auto& node : nodes)
            quadtree.add(&node);
        for (const auto& node : nodes)
            intersections[node.id] = quadtree.query(node.box);
    }
}

void bruteForceIntersections(benchmark::State& state)
{
    auto nodes = generateRandomNodes(static_cast<std::size_t>(state.range()));
    for (auto _ : state)
    {
        auto intersections = std::vector<std::vector<Node*>>(nodes.size());
        for (const auto& node : nodes)
            intersections[node.id] = computeIntersections(node.box, nodes);
    }
}

BENCHMARK(quadtreeIntersections)->RangeMultiplier(10)->Range(100, 100000)->Unit(benchmark::kMicrosecond);
BENCHMARK(bruteForceIntersections)->RangeMultiplier(10)->Range(100, 10000)->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN()