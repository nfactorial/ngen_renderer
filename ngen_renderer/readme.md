# ngen_renderer
The renderer: the render graph, which orders a frame's passes, works out their barriers and load ops, and
records the frame onto an RHI `CommandList`.

- **Spec:** [documents/specs/graphics/renderer/render_graph/render_graph.md](../documents/specs/graphics/renderer/render_graph/render_graph.md), the module's definition.
- **Target:** `ngen_renderer` (alias `ngen::renderer`). Public headers are in `include/ngen/renderer/`.
- **Tests:** `ngen_renderer_tests`, run with the root readme's `ctest` command.
