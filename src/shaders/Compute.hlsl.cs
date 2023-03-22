Texture2D ColorTexture : register(s0);
RWTexture2D<unorm float4> Copy : register(u0);

#define size_x 1
#define size_y 1
#define size_z 1

[numthreads(size_x, size_y, size_z)]
void
main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint2 ScreenCoord = DispatchThreadID.xy;
    float Red = ColorTexture[ScreenCoord].x;
    float Green = ColorTexture[ScreenCoord].y;
    float Blue = ColorTexture[ScreenCoord].z;

    Copy[ScreenCoord] = float4(0, Green, 0, 1);
}
