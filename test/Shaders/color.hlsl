

cbuffer object_const_buffer : register(b0)
{
    float4x4 mvp_mat;
};

struct vertex_in
{
    float3 pos_input : POSITION;
    float4 color_input: COLOR;
};


struct vertex_out
{
    float4 pos_out   : SV_Position;
    float4 color_out : COLOR;
};

vertex_out VS(vertex_in vin)
{
    vertex_out vout;
    vout.color_out = vin.color_input;
    vout.pos_out = mul(float4(vin.pos_input,1.0f),mvp_mat);
    return vout;
}

float4 PS(vertex_out vout) : SV_Target
{
    return vout.color_out;
}
