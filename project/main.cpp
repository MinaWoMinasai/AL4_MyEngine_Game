#define DIRECTINPUT_VERSION 0x0800
#include "GameScene.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	D3DResouceLeakCheaker leakCheck;

	// COMの初期化を行う
	CoInitializeEx(0, COINIT_MULTITHREADED);

	MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

	// 誰も捕捉しなかった場合に(Unheadled)、捕捉する関数を登録
	Dump dump;
	SetUnhandledExceptionFilter(dump.Export);

	
	WinApp::GetInstance()->Initialize();

	std::unique_ptr<DirectXCommon> dxCommon;
	dxCommon = std::make_unique<DirectXCommon>();
	dxCommon->Initialize(WinApp::GetInstance());

	std::unique_ptr<SrvManager> srvManager;
	srvManager = std::make_unique<SrvManager>();
	srvManager->Initialize(dxCommon.get());

	// Imguiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(WinApp::GetInstance()->GetHwnd());
	ImGui_ImplDX12_Init(dxCommon->GetDevice().Get(),
		dxCommon->GetSwapChainDesc().BufferCount,
		dxCommon->GetRtvDesc().Format,
		srvManager->GetSrvHeap().Get(),
		srvManager->GetSrvHeap()->GetCPUDescriptorHandleForHeapStart(),
		srvManager->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());

	TextureManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());

	ModelManager::GetInstance()->Initialize(dxCommon.get());
	
	//　objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("teapot.obj");
	ModelManager::GetInstance()->LoadModel("bunny.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("cube.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("player.obj");
	ModelManager::GetInstance()->LoadModel("bullet.obj");

	Object3dCommon::GetInstance()->Initialize(dxCommon.get());
	
	// キーの初期化
	Input::GetInstance()->Initialize(WinApp::GetInstance()->GetWindowClass(), WinApp::GetInstance()->GetHwnd());
	
	std::unique_ptr<GameScene> gameScene;
	gameScene = std::make_unique<GameScene>();
	gameScene->Initialize();

	MSG msg{};
	// ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {

		// Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			// インプットインスタンスを取得
			Input* input = Input::GetInstance();

			// 前のフレームのキー状態を保存
			input->BeforeFrameData();
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			if (input->IsPress(input->GetKey()[DIK_LSHIFT]) && input->IsTrigger(input->GetKey()[DIK_D], input->GetPreKey()[DIK_D])) {
				if (Object3dCommon::GetInstance()->GetIsDebugCamera()) {
					Object3dCommon::GetInstance()->SetIsDebugCamera(false);
				} else {
					Object3dCommon::GetInstance()->SetIsDebugCamera(true);
				}
			}

			gameScene->Update();

			// ImGuiの内部コマンドを生成する
			ImGui::Render();

			dxCommon->PreDraw();

			srvManager->PreDraw();

			gameScene->Draw();

			// 実際のcommandListのImGuiの描画コマンドを組む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetList().Get());

			dxCommon->PostDraw();

		}
	}

	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CoUninitialize();

	CloseHandle(dxCommon->GetFenceEvent());
	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();

	dxCommon->Release();
	WinApp::GetInstance()->Finalize();

	return 0;
}