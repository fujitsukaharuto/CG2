#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <string>
#include <assert.h>

#include "Vector2.h"
#include "Vector3.h"
#include "Vector2Matrix.h"

struct Vector4
{
	float X;
	float Y;
	float Z;
	float W;
};

struct Trans
{
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct VertexDate
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct AABB {
	Vector3 min;
	Vector3 max;
};

struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTransPose;
};

struct TransformationParticleMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct Materials
{
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct DirectionalLight
{

	Vector4 color;
	Vector3 direction;
	float intensity;

};

struct CameraForGPU {
	Vector3 worldPosition;
};

struct GrayscaleVertex
{
	Vector4 position;
	Vector2 texcoord;
};

struct Sphere
{
	Vector3 center;
	float radius;
};

struct MaterialDataPath
{
	std::string textureFilePath;
};

struct ModelData
{
	std::vector<VertexDate> vertices;
	MaterialDataPath material;
};

struct ParticleDate
{
	Vector4 center[400];
	Vector4 radius[400];
	int particleCount;
	float threshold;
	float padding[2];
};


struct ShockWaveData {
	Vector4 center;
	float shockTime;
	float radius;
	float intensity;// 歪みの強さ
	float padding;
};


struct FireElement {
	float animeTime; // アニメーション時間
	Vector2 resolution; // 画面解像度
	float distortionStrength; // UVディストーションの強度
	float highlightStrength; // ハイライトの強度
	float detailScale; // 細かいノイズのスケール
	Vector2 rangeMin; // 炎の描画範囲（最小UV）
	Vector2 rangeMax; // 炎の描画範囲（最大UV）
	float scale; // Voronoiノイズのスケール
	float speed; // 炎の揺らぎ速度
	float noiseSpeed; // 細かいノイズの移動速度
	float blendStrength;// どれくらい混ぜるか
};


/// <summary>
/// 2次元ベクトルのスカラー倍を求める
/// </summary>
Vector2 Multiply(const Vector2& vec, const float& num);

/// <summary>
/// アフィン行列
/// </summary>
Matrix3x3 MakeAffineMat(Vector2 scale, float rotate, Vector2 translate);

/// <summary>
/// 3x3行列の積
/// </summary>
Matrix3x3 Multiply(Matrix3x3 matrix1, Matrix3x3 matrix2);

/// <summary>
/// ベクトルと2x2行列の積
/// </summary>
Vector2 Multiply(Vector2 vector, Matrix2x2 matrix);

/// <summary>
/// 2x2行列の逆行列
/// </summary>
Matrix2x2 Inverse(Matrix2x2 matrix);

/// <summary>
/// 3x3行列の逆行列
/// </summary>
Matrix3x3 Inverse(Matrix3x3 matrix);

/// <summary>
/// 正射影行列
/// </summary>
Matrix3x3 MakeOrthographicMat(float left, float top, float right, float bottom);

/// <summary>
/// ビューポート行列
/// </summary>
Matrix3x3 MakeViewportMat(float left, float top, float width, float hight);

/// <summary>
/// レンダリング
/// </summary>
Matrix3x3 MakeWvpVpMat(const Matrix3x3& world, const Matrix3x3& view, const Matrix3x3& ortho, const Matrix3x3& viewPort);

/// <summary>
/// ワールド座標に変える
/// </summary>
Vector2 Transform(const Vector2& vector, const Matrix3x3& matrix);

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

Matrix4x4 Multiply(const Matrix4x4& matrix1,const Matrix4x4& matrix2);

Matrix4x4 Transpose(const Matrix4x4& m);

Matrix4x4 Inverse(const Matrix4x4& matrix);

Matrix4x4 MakeIdentity4x4();

Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

Matrix4x4 MakeScaleMatrix(const Vector3& scale);

Matrix4x4 MakeRotateXMatrix(float rad);

Matrix4x4 MakeRotateYMatrix(float rad);

Matrix4x4 MakeRotateZMatrix(float rad);

Matrix4x4 MakeRotateXYZMatrix(const Vector3& rota);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

Matrix4x4 MakePerspectiveFovMatrix(float fovy, float aspectRation, float nearClip, float farClip);

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

Matrix4x4 MakeViewportMat(float left, float top, float width, float height, float minDepth, float maxDepth);

Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

float Lerp(float v1, float v2, float t);

Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

Vector3 Cross(const Vector3& a, const Vector3& b);

float Clamp(float x, float min, float max);

Vector3 CatmullRomPoint(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t);

Vector3 CatmullRom(const std::vector<Vector3>& control, float t);

Vector3 ExtractEulerAngles(const Matrix4x4& rotationMatrix);

Matrix4x4 MakeLookAtMatrix(const Vector3& forward, const Vector3& up);

Matrix4x4 MakeRotationAxisAngle(const Vector3& axis, float angle);

bool IsCollision(const AABB& aabb, const Vector3& point);