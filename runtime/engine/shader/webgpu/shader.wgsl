
struct fragment_stage_struct {
    @builtin(position)              position_data               : vec4<f32>,
    @location(0)                    other_position_data         : vec2<f32>
}

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> fragment_stage_struct {
    let positions = array<vec2<f32>, 3>(
        vec2<f32>(-1.0, 3.0),
        vec2<f32>(-1.0, -1.0),
        vec2<f32>(3.0, -1.0)
    );

    let pos = positions[in_vertex_index];
    var output : fragment_stage_struct;
    output.position_data        =  vec4<f32>(pos, 0.0, 1.0);
    //output.other_position_data  =  vec2<f32>(0.5 * (1 + pos));
    output.other_position_data  =  vec2<f32>(pos.x, -pos.y);
    return output;
}

@fragment
fn fs_main(input:    fragment_stage_struct) -> @location(0) vec4<f32> 
{
    //var tex_uv = input.position_data.xy / vec2<f32>(f32(screen.width), f32(screen.height));
    return vec4<f32>(1.0, 1.0, 1.0, 1.0);
}


