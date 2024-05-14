#include "Fuji.h"
#include <Windows.h>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"


#include "MatrixCalculation.h"



int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	Fuji::InitDX();


	Trans transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Trans cameraTrans{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };

	Trans transSprite = { {1.0f,1.0f,1.0f,},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	bool useMonsterBall = true;

	//ウィンドウのxボタンが押されるまでループ
	while (Fuji::ProcessMessage() == 0)
	{
		Fuji::StartFrame();

		ImGui::Begin("debug");
		ImGui::SliderFloat("trans x", &transform.translate.x, -1.0f, 1.0f);
		ImGui::SliderFloat("trans y", &transform.translate.y, -1.0f, 1.0f);
		ImGui::SliderFloat("trans z", &transform.translate.z, -1.0f, 1.0f);
		ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		ImGui::End();

		/*ImGui::Begin("light");
		ImGui::SliderFloat4("color", &directionalLightData->color.X, 0.0f, 1.0f);
		ImGui::SliderFloat3("direction", &directionalLightData->direction.x, -1.0f, 1.0f);
		ImGui::End();*/

		transform.rotate.y += 0.03f;
		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTrans.scale, cameraTrans.rotate, cameraTrans.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(Fuji::GetkWindowWidth()) / float(Fuji::GetkWindowHeight()), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		worldViewProjectionMatrix = Multiply(worldMatrix, worldViewProjectionMatrix);

		Fuji::SetWVP(worldMatrix, worldViewProjectionMatrix);


		Matrix4x4 worldMatSprite = MakeAffineMatrix(transSprite.scale, transSprite.rotate, transSprite.translate);


		Matrix4x4 viewMatSprite = MakeIdentity4x4();
		Matrix4x4 projectMatSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(Fuji::GetkWindowWidth()), float(Fuji::GetkWindowHeight()), 0.0f, 100.0f);
		Matrix4x4 worldViewProMatSprite = Multiply(viewMatSprite, projectMatSprite);
		worldViewProMatSprite = Multiply(worldMatSprite, worldViewProMatSprite);


		Fuji::SetSpriteWVP(worldMatSprite, worldViewProMatSprite);


		Fuji::EndFrame();
	}

	//OutputDebugStringA("Hello,DirectX!\n");
	//Log(ConvertString(std::format(L"WSTRING{}\n", wstringValue)));



	Fuji::End();
	return 0;
}

