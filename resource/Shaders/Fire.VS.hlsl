// 入力レイアウト（頂点バッファの構造）
struct VSInput
{
    float4 position : POSITION0; // 頂点の位置
    float2 texCoord : TEXCOORD0; // テクスチャ座標
};

// 頂点シェーダーの出力
struct PSInput
{
    float4 position : SV_POSITION; // スクリーン空間での位置
    float2 texCoord : TEXCOORD0; // テクスチャ座標
};

// 頂点シェーダー（パススルー処理）
PSInput main(VSInput input)
{
    PSInput output;
    output.position = input.position; // 位置をそのまま出力
    output.texCoord = input.texCoord; // テクスチャ座標をそのまま出力
    return output;
}