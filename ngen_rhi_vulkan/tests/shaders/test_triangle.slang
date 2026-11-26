// The ngen_rhi_vulkan tests' shader: positions in, one flat colour out. Both stages live in one module; the
// pipeline selects them by entry point name.

struct VertexInput {
    float3 position : POSITION; // location 0
};

struct VertexOutput {
    float4 position : SV_Position;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;
    output.position = float4(input.position, 1.0);
    return output;
}

[shader("fragment")]
float4 fragmentMain() : SV_Target {
    return float4(1.0, 0.5, 0.0, 1.0);
}
