// 定数バッファ
struct FireParams
{
    float time; // アニメーション時間
    float2 resolution; // 画面解像度
    float distortionStrength; // UVディストーションの強度
    float highlightStrength; // ハイライトの強度
    float detailScale; // 細かいノイズのスケール
    float2 rangeMin; // 炎の描画範囲（最小UV）
    float2 rangeMax; // 炎の描画範囲（最大UV）
    float voronoiScale; // Voronoiノイズのスケール
    float voronoiSpeed; // Voronoiノイズの移動速度
    float fineNoiseSpeed; // 細かいノイズの移動速度
    float blendStrength; // 炎のエフェクトを元のテクスチャに混ぜる割合
};

Texture2D g_InputTexture : register(t0); // 元の画面テクスチャ
Texture2D maskTexture : register(t1); // サイドフェードの四角いマスク
Texture2D voronoiNoiseTexture : register(t2); // Voronoiベースのノイズ
Texture2D fineNoiseTexture : register(t3); // 細かいノイズのテクスチャ
SamplerState g_Sampler : register(s0);
ConstantBuffer<FireParams> g_Fire : register(b0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0; // UV座標
};

// Voronoiノイズを使用したUVディストーション
float2 ApplyVoronoiDistortion(float2 uv, float time)
{
    // 時間によるスクロールを加える
    float2 animatedUV = uv * g_Fire.voronoiScale + float2(0.1f, 0.2f) * time * g_Fire.voronoiSpeed;
    float2 distortion = voronoiNoiseTexture.Sample(g_Sampler, animatedUV).rg;
    return uv + (distortion - 0.5f) * g_Fire.distortionStrength;
}

// 細かいノイズを使用して炎の質感を追加
float AddFineNoiseTexture(float2 uv, float time)
{
    // 細かいノイズのUVに時間ベースのスクロールを加える
    float2 animatedFineUV = uv * g_Fire.detailScale + float2(0.2f, -0.3f) * time * g_Fire.fineNoiseSpeed;
    return fineNoiseTexture.Sample(g_Sampler, animatedFineUV).r;
}

// 炎の色を時間と共に動的に変化させる（濃さ調整）
float3 GetFireColor(float intensity)
{
    // 炎の色のグラデーション：赤から黄色、オレンジ、白に変化（濃さを強調）
    float3 fireColor = lerp(float3(1.0, 0.1, 0.0), float3(1.0f, 1.0f, 1.0f), intensity * 0.8f);
    return fireColor;
}

float4 main(PSInput input) : SV_TARGET
{
    // 元の画面テクスチャを取得
    float4 baseColor = g_InputTexture.Sample(g_Sampler, input.texcoord);

    // UV座標を正規化
    float2 normalizedUV = input.texcoord;

    // 描画範囲のチェック
    if (normalizedUV.x < g_Fire.rangeMin.x || normalizedUV.x > g_Fire.rangeMax.x ||
        normalizedUV.y < g_Fire.rangeMin.y || normalizedUV.y > g_Fire.rangeMax.y)
    {
        return float4(baseColor.rgb, 1.0f); // 範囲外は元のテクスチャをそのまま返す
    }

    // マスクのフェード効果
    float maskValue = maskTexture.Sample(g_Sampler, normalizedUV).r;

    // Voronoiノイズを使ったUVディストーション
    float2 distortedUV = ApplyVoronoiDistortion(normalizedUV, g_Fire.time);

    // Voronoiによる炎の強度（明るさ）
    float intensity = voronoiNoiseTexture.Sample(g_Sampler, distortedUV).r;

    // 細かいノイズを追加（色計算前に強度に加算）
    float fineNoise = AddFineNoiseTexture(normalizedUV, g_Fire.time);
    intensity += fineNoise * 0.15f; // 細かいノイズの影響を増加

    // 強度を調整して炎の効果を濃くする
    intensity = pow(intensity, 1.2f); // 強度を強調（明るい部分をさらに強く）

    // 時間に応じた炎の色を取得
    float3 fireColor = GetFireColor(intensity);

    // 炎の最終的な色を計算
    float3 fireEffect = fireColor * intensity;

    // fireColor の明るさ（輝度）を計算
    float fireBrightness = dot(fireColor, float3(0.299f, 0.587f, 0.114f));

    // fireBrightness を使って炎と元の色のブレンド比率を動的に調整
    float adjustedBlendFactor = g_Fire.blendStrength * maskValue * saturate(fireBrightness);

    // 元の色と炎のエフェクトをブレンド
    float3 finalColor = lerp(baseColor.rgb, fireEffect, adjustedBlendFactor);

    // 常に透明度は1.0
    return float4(finalColor, 1.0f);
}


//// Voronoiノイズを使用したUVディストーション
//float2 ApplyVoronoiDistortion(float2 uv, float time)
//{
//    // 時間によるスクロールとスケール調整
//    float2 animatedUV = uv * g_Fire.voronoiScale 
//                        + float2(0.1f, 0.2f) * time * g_Fire.voronoiSpeed;
                        
//    // ノイズパターンに時間による変化を加える
//    float2 scrollOffset = float2(sin(time * 0.5f), cos(time * 0.3f)) * 0.1f; // 適度な動き
//    animatedUV += scrollOffset;

//    // Voronoiノイズをサンプリングしてディストーション計算
//    float2 distortion = voronoiNoiseTexture.Sample(g_Sampler, animatedUV).rg;
//    return uv + (distortion - 0.5f) * g_Fire.distortionStrength;
//}

//// 細かいノイズを使用して炎の質感を追加
//float AddFineNoiseTexture(float2 uv, float time)
//{
//    // 細かいノイズのUVに時間ベースのスクロールを加える
//    float2 animatedFineUV = uv * g_Fire.detailScale + float2(0.2f, -0.3f) * time * g_Fire.fineNoiseSpeed;
//    return fineNoiseTexture.Sample(g_Sampler, animatedFineUV).r;
//}

//// 炎の色を時間と共に動的に変化させる（濃さ調整）
//float3 GetFireColor(float intensity)
//{
//    // 炎の色のグラデーション：赤から黄色、オレンジ、白に変化（濃さを強調）
//    float3 fireColor = lerp(float3(1.0, 0.1, 0.0), float3(1.0f, 1.0f, 1.0f), intensity * 0.8f);
//    return fireColor;
//}

//// UVに時間による動きを追加
//float2 AddTimeBasedUVOffset(float2 uv, float time)
//{
//    // 時間に基づく基本的なスクロール
//    float2 scrollOffset = float2(0.1f, -0.2f) * time;

//    // 周期的な変動を追加（sin/cos を使った動き）
//    float2 waveOffset = float2(sin(time * 1.5f + uv.y * 10.0f),
//                               cos(time * 1.2f + uv.x * 8.0f)) * 0.05f;

//    // 最終的なUVを計算
//    return uv + scrollOffset + waveOffset;
//}


//float4 main(PSInput input) : SV_TARGET
//{
//    // 元の画面テクスチャを取得
//    float4 baseColor = g_InputTexture.Sample(g_Sampler, input.texcoord);

//    // UV座標を正規化
//    float2 normalizedUV = input.texcoord;
//    float2 dynamicUV = AddTimeBasedUVOffset(normalizedUV, g_Fire.time);
    
//    // 描画範囲のチェック
//    if (normalizedUV.x < g_Fire.rangeMin.x || normalizedUV.x > g_Fire.rangeMax.x ||
//        normalizedUV.y < g_Fire.rangeMin.y || normalizedUV.y > g_Fire.rangeMax.y)
//    {
//        return float4(baseColor.rgb, 1.0f); // 範囲外は元のテクスチャをそのまま返す
//    }

//    // マスクのフェード効果
//    float maskValue = maskTexture.Sample(g_Sampler, normalizedUV).r;

//    // Voronoiノイズを使ったUVディストーション
//    float2 distortedUV = ApplyVoronoiDistortion(dynamicUV, g_Fire.time);

//    // Voronoiによる炎の強度（明るさ）
//    float intensity = voronoiNoiseTexture.Sample(g_Sampler, distortedUV).r;

//    // 細かいノイズを追加（色計算前に強度に加算）
//    float fineNoise = AddFineNoiseTexture(normalizedUV, g_Fire.time);
//    intensity += fineNoise * 0.15f; // 細かいノイズの影響を増加

//    // 強度を調整して炎の効果を濃くする
//    intensity = pow(intensity, 1.2f); // 強度を強調（明るい部分をさらに強く）

//    // 時間に応じた炎の色を取得
//    float3 fireColor = GetFireColor(intensity);

//    // 炎の最終的な色を計算
//    float3 fireEffect = fireColor * intensity;

//    // fireColor の明るさ（輝度）を計算
//    float fireBrightness = dot(fireColor, float3(0.299f, 0.587f, 0.114f));

//    // fireBrightness を使って炎と元の色のブレンド比率を動的に調整
//    float adjustedBlendFactor = g_Fire.blendStrength * maskValue * saturate(fireBrightness);

//    // 元の色と炎のエフェクトをブレンド
//    float3 finalColor = lerp(baseColor.rgb, fireEffect, adjustedBlendFactor);

//    // 常に透明度は1.0
//    return float4(finalColor, 1.0f);
//}