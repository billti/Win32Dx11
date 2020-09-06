cbuffer CBuffer {
	row_major matrix transform;
};

float4 main(float2 pos : POSITION) : SV_POSITION
{
	return mul(float4(pos.x, pos.y, 0.0f, 1.0f), transform);
}
