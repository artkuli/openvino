// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <core/graph_cache.hpp>
#include <ngraph/except.hpp>

namespace ngraph
{
    namespace onnx_import
    {
        void GraphCache::emplace_node(const std::string& name, Output<ngraph::Node>&& node)
        {
            m_graph_cache_map[name] = std::move(node);
        }

        void GraphCache::remove_node(const std::string& name)
        {
            auto it = m_graph_cache_map.find(name);
            if (it != m_graph_cache_map.end())
            {
                m_graph_cache_map.erase(it);
            }
        }

        Output<ngraph::Node> GraphCache::get_node(const std::string& name) const
        {
            try
            {
                return m_graph_cache_map.at(name);
            }
            catch (const std::out_of_range&)
            {
                throw ngraph_error(name + " node not found in graph cache");
            }
        }

        bool GraphCache::contains(const std::string& name) const
        {
            return (m_graph_cache_map.count(name) > 0);
        }

        NodeScope GraphCache::node_scope(const std::string& name) const
        {
            return contains(name) ? NodeScope::ParentGraph : NodeScope::Lack;
        }

        SubgraphCache::SubgraphCache(const GraphCache& parent_graph_cache)
            : m_parent_graph_cache{&parent_graph_cache}
        {
            if (m_parent_graph_cache == nullptr)
            {
                throw ngraph_error("Parent graph cache is not initialized");
            }
        }

        Output<ngraph::Node> SubgraphCache::get_node(const std::string& name) const
        {
            // present in subgraph scope
            if (GraphCache::contains(name))
            {
                return GraphCache::get_node(name);
            }
            else // present in parent graph scope
            {
                return m_parent_graph_cache->get_node(name);
            }
        }

        bool SubgraphCache::contains(const std::string& name) const
        {
            // the node is in subgraph or in parent graph scope
            return GraphCache::contains(name) || m_parent_graph_cache->contains(name);
        }

        NodeScope SubgraphCache::node_scope(const std::string& name) const
        {
            if (GraphCache::contains(name))
            {
                return NodeScope::SubGraph;
            }
            else if (m_parent_graph_cache->contains(name))
            {
                return NodeScope::ParentGraph;
            }
            else
            {
                return NodeScope::Lack;
            }
        }

    } // namespace onnx_import
} // namespace ngraph
