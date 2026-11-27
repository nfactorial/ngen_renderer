// Spec: renderer. Behaviour changes go through it.
#include <ngen/renderer/renderer.h>

#include <ngen/renderer/render_graph.h>

#include <span>
#include <string>
#include <utility>
#include <vector>

namespace ngen::renderer
{
    void RenderList::add(DrawItem item)
    {
        items_.push_back(std::move(item));
    }

    void RenderList::clear()
    {
        items_.clear();
    }

    std::span<const DrawItem> RenderList::items() const
    {
        return items_;
    }

    namespace
    {
        // Draws the items in order, binding a pipeline only when it differs from the one bound in this pass.
        void recordItems(rhi::CommandList &commands, const std::span<const DrawItem *const> items)
        {
            std::optional<rhi::PipelineHandle> bound;
            for (const DrawItem *item : items)
            {
                if (!bound || !(*bound == item->pipeline))
                {
                    commands.bindPipeline(item->pipeline);
                    bound = item->pipeline;
                }
                for (const VertexBufferBinding &vertexBuffer : item->vertexBuffers)
                {
                    commands.bindVertexBuffer(vertexBuffer.binding, vertexBuffer.buffer, vertexBuffer.offset);
                }
                commands.draw(item->vertexCount, item->instanceCount, item->firstVertex, item->firstInstance);
            }
        }
    } // namespace

    Renderer::Renderer(rhi::Device &device, RendererConfiguration configuration)
        : device_(device), configuration_(std::move(configuration))
    {
    }

    std::expected<std::unique_ptr<Renderer>, ngen::Error> Renderer::create(rhi::Device &device,
                                                                           RendererConfiguration configuration)
    {
        if (auto valid = validate(configuration); !valid)
        {
            return std::unexpected(std::move(valid.error()));
        }
        return std::unique_ptr<Renderer>(new Renderer(device, std::move(configuration)));
    }

    std::optional<StageId> Renderer::stage(const std::string_view name) const
    {
        for (size_t i = 0; i < configuration_.stages.size(); ++i)
        {
            if (configuration_.stages[i].name == name)
            {
                return StageId{static_cast<uint32_t>(i)};
            }
        }
        return std::nullopt;
    }

    const RendererConfiguration &Renderer::configuration() const
    {
        return configuration_;
    }

    std::expected<FrameResult, ngen::Error> Renderer::render(const RenderList &list)
    {
        const std::span<const DrawItem> items = list.items();
        for (size_t i = 0; i < items.size(); ++i)
        {
            if (items[i].stage.index >= configuration_.stages.size())
            {
                return std::unexpected(makeError(
                    ErrorCode::InvalidArgument, "render: draw item " + std::to_string(i) + " names stage " +
                                                    std::to_string(items[i].stage.index) +
                                                    ", which is not configured"));
            }
            if (items[i].vertexCount == 0)
            {
                return std::unexpected(
                    makeError(ErrorCode::InvalidArgument, "render: draw item " + std::to_string(i) + " has no vertices"));
            }
        }

        auto frame = device_.beginFrame();
        if (!frame)
        {
            return std::unexpected(std::move(frame.error()));
        }
        rhi::CommandList *const commands = *frame;
        if (commands == nullptr)
        {
            return FrameResult::Skipped;
        }

        // Items of each stage, in list order. The graph runs its passes before render returns.
        std::vector<std::vector<const DrawItem *>> byStage(configuration_.stages.size());
        for (const DrawItem &item : items)
        {
            byStage[item.stage.index].push_back(&item);
        }

        RenderGraph graph;
        const ResourceId image = graph.importTarget(rhi::SwapChainImage{}, rhi::ResourceState::Undefined);
        bool addedPass = false;
        for (size_t i = 0; i < byStage.size(); ++i)
        {
            if (byStage[i].empty())
            {
                continue;
            }
            PassDescription pass;
            pass.name = configuration_.stages[i].name;
            pass.order = static_cast<int32_t>(i);
            pass.color.target = image;
            if (!addedPass)
            {
                pass.color.clear = configuration_.clearColor;
                addedPass = true;
            }
            pass.record = [stageItems = &byStage[i]](rhi::CommandList &recorder) {
                recordItems(recorder, *stageItems);
            };
            graph.addPass(std::move(pass));
        }
        if (!addedPass)
        {
            PassDescription pass;
            pass.name = "Clear";
            pass.color = {image, configuration_.clearColor};
            pass.record = [](rhi::CommandList &) {};
            graph.addPass(std::move(pass));
        }

        if (auto executed = graph.execute(*commands); !executed)
        {
            // Close the frame so the device is not left recording; the execute error is the one reported.
            [[maybe_unused]] const auto closed = device_.endFrame();
            return std::unexpected(std::move(executed.error()));
        }
        if (auto ended = device_.endFrame(); !ended)
        {
            return std::unexpected(std::move(ended.error()));
        }
        return FrameResult::Presented;
    }
} // namespace ngen::renderer
