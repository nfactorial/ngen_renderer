# ngen_renderer
The renderer: what a game draws through, and the render graph beneath it. The renderer takes a render list of draw
items and, each frame, builds a render graph with one pass per stage that has items, records it and presents. The
render graph orders a frame's passes, works out their barriers and load ops, and records the frame onto an RHI
`CommandList`.

- **Specs:** [documents/specs/graphics/renderer/renderer.md](../documents/specs/graphics/renderer/renderer.md), the renderer's definition, and [documents/specs/graphics/renderer/render_graph/render_graph.md](../documents/specs/graphics/renderer/render_graph/render_graph.md), the render graph's.
- **Target:** `ngen_renderer` (alias `ngen::renderer`). Public headers are in `include/ngen/renderer/`.
- **Tests:** `ngen_renderer_tests`, run with the root readme's `ctest` command.
