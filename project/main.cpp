#define DIRECTINPUT_VERSION 0x0800
#include "SceneManager.h"
#include "RenderTexture.h"
#include "BloomConstantBuffer.h"
#include "PostEffect.h"

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
	
	std::unique_ptr<RtvManager> rtvManager;
	rtvManager = std::make_unique<RtvManager>();
	rtvManager->Initialize(dxCommon.get());
	
	std::unique_ptr<RenderTexture> sceneRenderTexture;
	sceneRenderTexture = std::make_unique<RenderTexture>();

	sceneRenderTexture->Initialize(
		dxCommon.get(),
		srvManager.get(),
		rtvManager.get(),
		WinApp::kClientWidth,
		WinApp::kClientHeight
	);
	
	// bloom用CBVの生成
	std::unique_ptr<BloomConstantBuffer> bloomCB = std::make_unique<BloomConstantBuffer>();
	bloomCB->Initialize(dxCommon.get());

	// ポストエフェクトの初期化
	std::unique_ptr<PostEffect> postEffect = std::make_unique<PostEffect>();
	postEffect->Initialize(dxCommon.get(), bloomCB.get());
	
	std::unique_ptr<RenderTexture> bloomRT_A;
	std::unique_ptr<RenderTexture> bloomRT_B;
	bloomRT_A = std::make_unique<RenderTexture>();
	bloomRT_B = std::make_unique<RenderTexture>();

	bloomRT_A->Initialize(
		dxCommon.get(),
		srvManager.get(),
		rtvManager.get(),
		WinApp::kClientWidth,
		WinApp::kClientHeight
	);

	bloomRT_B->Initialize(
		dxCommon.get(),
		srvManager.get(),
		rtvManager.get(),
		WinApp::kClientWidth,
		WinApp::kClientHeight
	);
	
	// ブルームパラメータ
	BloomParam bloomParam{};
	bloomParam.threshold = 0.0f;
	bloomParam.intensity = 1.2f;
	bloomCB->Update(bloomParam);

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
	ModelManager::GetInstance()->LoadModel("cubeDamage.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("player.obj");
	ModelManager::GetInstance()->LoadModel("playerBullet.obj");
	ModelManager::GetInstance()->LoadModel("enemy.obj");
	ModelManager::GetInstance()->LoadModel("enemyBullet.obj");
	ModelManager::GetInstance()->LoadModel("playerParticle.obj");
	ModelManager::GetInstance()->LoadModel("enemyParticle.obj");

	Object3dCommon::GetInstance()->Initialize(dxCommon.get());
	
	SpriteCommon::GetInstance()->Initialize(dxCommon.get());

	std::unique_ptr<Sprite> sceneRTSprite = std::make_unique<Sprite>();
	sceneRTSprite->Initialize(
		SpriteCommon::GetInstance(),
		sceneRenderTexture->GetSrvIndex(),
		srvManager.get()
	);

	sceneRTSprite->SetPosition({ 0.0f, 0.0f });
	sceneRTSprite->SetSize({
		(float)WinApp::kClientWidth,
		(float)WinApp::kClientHeight
		});

	// キーの初期化
	Input::GetInstance()->Initialize(WinApp::GetInstance()->GetWindowClass(), WinApp::GetInstance()->GetHwnd());
	
	std::unique_ptr<SceneManager> sceneManager;
	sceneManager = std::make_unique<SceneManager>();
	sceneManager->Initialize();

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
			
			ImGui::Begin("Bloom");
			ImGui::DragFloat("Threshold", &bloomParam.threshold, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Insensity", &bloomParam.intensity, 0.01f);
			ImGui::End();

			bloomCB->Update(bloomParam);

			sceneManager->Update();

			sceneRTSprite->Update();

			// ImGuiの内部コマンドを生成する
			ImGui::Render();
			dxCommon->PreDraw();
			srvManager->PreDraw();

			// Scene → SceneRT
			dxCommon->SetRenderTarget(sceneRenderTexture->GetRTVHandle());
			dxCommon->ClearRenderTarget(sceneRenderTexture->GetRTVHandle());
			dxCommon->ClearDepthBuffer();

			sceneManager->Draw();

			// ① BrightPass : SceneRT → bloomRT_A
			dxCommon->SetRenderTarget(bloomRT_A->GetRTVHandle());
			dxCommon->ClearRenderTarget(bloomRT_A->GetRTVHandle());

			postEffect->Draw(
				sceneRenderTexture->GetSrvManager()->GetGPUDescriptorHandle(
					sceneRenderTexture->GetSrvIndex()),
				kAdd_Bloom_Extract
			);

			// ② BlurH : bloomRT_A → bloomRT_B
			dxCommon->SetRenderTarget(bloomRT_B->GetRTVHandle());
			dxCommon->ClearRenderTarget(bloomRT_B->GetRTVHandle());

			postEffect->Draw(
				bloomRT_A->GetSrvManager()->GetGPUDescriptorHandle(
					bloomRT_A->GetSrvIndex()),
				kAdd_Bloom_BlurH
			);

			// ③ BlurV : bloomRT_B → bloomRT_A
			dxCommon->SetRenderTarget(bloomRT_A->GetRTVHandle());
			dxCommon->ClearRenderTarget(bloomRT_A->GetRTVHandle());

			postEffect->Draw(
				bloomRT_B->GetSrvManager()->GetGPUDescriptorHandle(
					bloomRT_B->GetSrvIndex()),
				kAdd_Bloom_BlurV
			);

			// ④ BackBuffer に戻す（今は Blur の結果をそのまま表示）
			dxCommon->SetBackBuffer();
			
			postEffect->DrawComposite(srvManager->GetGPUDescriptorHandle(sceneRenderTexture->GetSrvIndex()), srvManager->GetGPUDescriptorHandle(bloomRT_A->GetSrvIndex()));
			
			SpriteCommon::GetInstance()->PreDraw(kNone);
			//sceneRTSprite->Draw();   // ← SceneRT の SRV を貼る
			sceneManager->DrawSprite();

			// 実際のcommandListのImGuiの描画コマンドを組む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetList().Get());

			dxCommon->PostDraw();

			sceneManager->ChangeScene();
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