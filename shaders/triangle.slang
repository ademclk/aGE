struct VSOut {
    float4 position : SV_Position;
    float3 color    : COLOR;
};

// The triangle is generated from SV_VertexID: no vertex buffer needed, for testing.
[shader("vertex")]
VSOut vsMain(uint vertex_id : SV_VertexID) {
    // Three positions in clip space (x, y, z, w) - +Y up.
    static const float2 positions[3] = {
        float2( 0.0,  0.5),   // top
        float2(-0.5, -0.5),   // bottom-left
        float2( 0.5, -0.5),   // bottom-right
    };
    static const float3 colors[3] = {
        float3(1.0, 0.2, 0.2),   // red
        float3(0.2, 1.0, 0.2),   // green
        float3(0.2, 0.2, 1.0),   // blue
    };

    VSOut out_;
    out_.position = float4(positions[vertex_id], 0.0, 1.0);
    out_.color    = colors[vertex_id];
    return out_;
}

[shader("fragment")]
float4 fsMain(VSOut input) : SV_Target {
    return float4(input.color, 1.0);
}
