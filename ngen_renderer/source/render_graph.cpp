// Spec: render-graph. Behaviour changes go through it.
#include <ngen/renderer/render_graph.h>

#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <utility>
#include <variant>

namespace ngen::renderer
{
    namespace
    {
        // What the graph tracks about a resource while compiling.
        struct ResourceTrack
        {
            rhi::BarrierTarget target;
            rhi::ResourceState state;
            bool written = false;
        };

        // Whether a pass's shaders may read the target. A target added to the variant is a compile error here.
        bool shaderReadable(const rhi::BarrierTarget &target)
        {
            return std::visit([](const rhi::SwapChainImage &) { return false; }, target);
        }

        bool hasDefinedContents(const ResourceTrack &track)
        {
            return track.written || track.state != rhi::ResourceState::Undefined;
        }

        std::unexpected<ngen::Error> invalid(const std::string &pass, const std::string &reason)
        {
            return std::unexpected(
                ngen::makeError(ngen::ErrorCode::InvalidArgument, "pass \"" + pass + "\": " + reason));
        }

        // Checks that a pass is legal against the graph's state before it runs. A valid pass's name is added to
        // names; index is its position in addPass order, used to name a pass with no name.
        std::expected<void, ngen::Error> validatePass(const PassDescription &pass, const size_t index,
                                                      std::unordered_set<std::string> &names,
                                                      const std::vector<ResourceTrack> &tracks)
        {
            const auto issued = [&tracks](const ResourceId id) { return id.index < tracks.size(); };

            if (pass.name.empty())
            {
                return invalid("#" + std::to_string(index), "name is empty");
            }
            if (!names.insert(pass.name).second)
            {
                return invalid(pass.name, "name is used by more than one pass");
            }
            if (!issued(pass.color.target))
            {
                return invalid(pass.name, "color.target is not a resource of this graph");
            }
            for (const ResourceId read : pass.reads)
            {
                if (!issued(read))
                {
                    return invalid(pass.name, "reads names a resource not in this graph");
                }
                if (read == pass.color.target)
                {
                    return invalid(pass.name, "reads its own colour target");
                }
                if (!hasDefinedContents(tracks[read.index]))
                {
                    return invalid(pass.name, "reads a resource with no defined contents");
                }
                if (!shaderReadable(tracks[read.index].target))
                {
                    return invalid(pass.name, "reads the swap chain image, which shaders cannot read");
                }
            }
            return {};
        }
    } // namespace

    ResourceId RenderGraph::importTarget(const rhi::BarrierTarget target, const rhi::ResourceState current)
    {
        resources_.push_back({target, current});
        return ResourceId{static_cast<uint32_t>(resources_.size() - 1)};
    }

    void RenderGraph::addPass(PassDescription pass)
    {
        passes_.push_back(std::move(pass));
    }

    void RenderGraph::clear()
    {
        resources_.clear();
        passes_.clear();
    }

    std::vector<size_t> RenderGraph::runOrder() const
    {
        std::vector<size_t> indices(passes_.size());
        std::iota(indices.begin(), indices.end(), size_t{0});
        std::ranges::stable_sort(
            indices, [this](const size_t a, const size_t b) { return passes_[a].order < passes_[b].order; });
        return indices;
    }

    std::expected<CompiledGraph, ngen::Error> RenderGraph::compile() const
    {
        std::vector<ResourceTrack> tracks;
        tracks.reserve(resources_.size());
        for (const Resource &resource : resources_)
        {
            tracks.push_back({resource.target, resource.state});
        }

        CompiledGraph graph;
        std::unordered_set<std::string> names;

        for (const size_t index : runOrder())
        {
            const PassDescription &pass = passes_[index];
            if (auto valid = validatePass(pass, index, names, tracks); !valid)
            {
                return std::unexpected(std::move(valid.error()));
            }

            CompiledPass compiled;
            compiled.name = pass.name;

            ResourceTrack &color = tracks[pass.color.target.index];
            if (color.state != rhi::ResourceState::ColorAttachment || color.written)
            {
                compiled.barriers.push_back(
                    {color.target, color.state, rhi::ResourceState::ColorAttachment});
            }

            if (pass.color.clear)
            {
                compiled.colorTarget = {rhi::LoadOp::Clear, rhi::StoreOp::Store, *pass.color.clear};
            }
            else
            {
                compiled.colorTarget.load = hasDefinedContents(color) ? rhi::LoadOp::Load : rhi::LoadOp::DontCare;
                compiled.colorTarget.store = rhi::StoreOp::Store;
            }
            color.state = rhi::ResourceState::ColorAttachment;
            color.written = true;

            for (const ResourceId read : pass.reads)
            {
                ResourceTrack &track = tracks[read.index];
                if (track.state != rhi::ResourceState::ShaderRead)
                {
                    compiled.barriers.push_back(
                        {track.target, track.state, rhi::ResourceState::ShaderRead});
                }
                track.state = rhi::ResourceState::ShaderRead;
                track.written = false;
            }

            graph.passes.push_back(std::move(compiled));
        }
        return graph;
    }

    std::expected<void, ngen::Error> RenderGraph::execute(rhi::CommandList &commands) const
    {
        const auto compiled = compile();
        if (!compiled)
        {
            return std::unexpected(compiled.error());
        }

        const std::vector<size_t> order = runOrder();
        for (size_t i = 0; i < order.size(); ++i)
        {
            const CompiledPass &pass = compiled->passes[i];
            for (const rhi::Barrier &barrier : pass.barriers)
            {
                commands.barrier(barrier);
            }
            commands.beginRendering(pass.colorTarget);
            if (const auto &record = passes_[order[i]].record)
            {
                record(commands);
            }
            commands.endRendering();
        }
        return {};
    }
} // namespace ngen::renderer
