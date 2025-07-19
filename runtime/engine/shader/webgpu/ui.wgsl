// VS_UI

struct VertexData {
    @builtin(position) position : vec4<f32>,
    @location(0) texuv : vec2<f32>,
    @location(1) color : vec4<f32>,
};

@vertex
fn main_vs(@location(0) position: vec3<f32>, @location(1) texuv: vec2<f32>, @location(2) color: vec4<f32>) -> VertexData
{
    var output : VertexData;
    output.position = vec4<f32>(position, 1.0);
    output.texuv = vec2<f32>(position.x, -position.y);
    output.color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
    return output;
}

@fragment
fn main_fs(input: VertexData) -> @location(0) vec4<f32> 
{
    return input.color;
}


