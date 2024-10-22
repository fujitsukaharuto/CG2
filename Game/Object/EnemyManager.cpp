#include "Object/EnemyManager.h"

//local
#include "ModelManager.h"
#include "Collision/CollisionManager.h"

#include "Random.h"

EnemyManager::EnemyManager(){
	noteEnemies_.clear();
	chainEnemies_.clear();
	obstacles_.clear();
}

EnemyManager::~EnemyManager(){}

void EnemyManager::Initialize(){
	//ファイルを読む
	LoadPopFile();
}

void EnemyManager::Update(){
	UpdatePopData();


	/////////////////////////////////////////////////////////////////////////////////////////
	//                  単体の敵の更新と解放
	/////////////////////////////////////////////////////////////////////////////////////////
	for (auto it = noteEnemies_.begin(); it != noteEnemies_.end();){
		(*it)->Update();

		//削除フラグがtrueなら消す
		if ((*it)->GetIsRemoved()){
			CollisionManager::GetInstance()->RemoveCollider((*it).get());
			it = noteEnemies_.erase(it);  // 安全に削除
		}

		//音符に変わっていない状態で削除ラインまで行くと消える
		else if (!(*it)->GetIsChangedNote() && (*it)->GetTranslate().x <= Field::fieldEndPosX + (*it)->GetScale().x * 0.5f){
			CollisionManager::GetInstance()->RemoveCollider((*it).get());
			it = noteEnemies_.erase(it);  // 安全に削除
			PopObstacle();
		} else{
			++it;  // 削除されなかった場合イテレータを進める
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//                  連結している敵の更新と解放
	/////////////////////////////////////////////////////////////////////////////////////////
	for (auto it = chainEnemies_.begin(); it != chainEnemies_.end();){
		(*it)->Update();

		// ChainEnemy 内の全ての NoteEnemy が削除された場合には ChainEnemy 自体も削除する
		if ((*it)->IsAllRemove()){
			it = chainEnemies_.erase(it);  // ChainEnemy 自体を削除
		} else{
			++it;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//                  障害物の更新と解放
	/////////////////////////////////////////////////////////////////////////////////////////
	for (auto it = obstacles_.begin(); it != obstacles_.end(); ){
		(*it)->Update();

		// 削除フラグが true なら消す
		if (!(*it)->GetIsAlive()){
			CollisionManager::GetInstance()->RemoveCollider((*it).get());
			it = obstacles_.erase(it);  // 削除後は次の要素を指す
		} else{
			++it;  // 削除しなければ次の要素に進む
		}
	}

}

void EnemyManager::Draw(){
	for (auto& enemy : noteEnemies_){
		enemy->Draw();
	}

	for (auto& enemy : chainEnemies_){
		enemy->Draw();
	}

	for (auto& obstacle : obstacles_){
		obstacle->Draw();
	}
}

void EnemyManager::RemoveEnemey(Character* enemy){
	//リストから削除
	noteEnemies_.remove_if([enemy] (const std::unique_ptr<NoteEnemy>& noteEnemyPtr){
		return noteEnemyPtr.get() == enemy;
						   });
}

void EnemyManager::LoadPopFile(){
	//ファイルを開く
	std::ifstream file;
	file.open("./resource/csv/enemyPop.csv");
	assert(file.is_open());

	//ファイルの内容を文字列ストリームにコピー
	enemyPopCommands_ << file.rdbuf();

	//ファイルを閉じる
	file.close();
}

void EnemyManager::UpdatePopData(){
	// 待機中なら待機処理を行う
	if (isWaiting){
		waitTimer--;
		if (waitTimer <= 0){
			isWaiting = false;  // 待機完了
		}
		return;  // 待機中はこれ以上の処理を行わない
	}

	// コマンドを1行ずつ処理
	std::string line;
	if (getline(enemyPopCommands_, line)){
		// 1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);

		std::string word;
		// ,区切りで行の先頭文字列を取得
		getline(line_stream, word, ',');

		// "//"から始まる行はコメント
		if (word.find("//") == 0){
			// コメント行を飛ばす
			return;  // 次のフレームで次の行を処理
		}

	#pragma region POPコマンド
		// POPコマンド
		if (word.find("POP") == 0){
			// POPに続く数字を取得
			getline(line_stream, word, ',');

			int fieldIndex = std::atoi(word.c_str());

			// 数字が1～5の範囲内であれば、その座標を使って敵を生成
			if (fieldIndex >= 1 && fieldIndex <= 5){


				//生成する敵の種類を判別
				getline(line_stream, word, ',');
				int popType = std::atoi(word.c_str());

				//連鎖してる敵を生成
				if (popType == static_cast< int >(EnemyType::CHAIN_ENEMY)){
					CreateChainEnemy(fieldIndex);
				} else{ //連鎖している敵以外の番号の場合連鎖していない敵を生成
					CreateUnChainEnemy(fieldIndex);
				}


				return;  // 次のフレームで次の行を処理
			}
		}
	#pragma endregion

	#pragma region WAITコマンド
		// WAITコマンド
		else if (word.find("WAIT") == 0){
			getline(line_stream, word, ',');

			// 待ち時間を取得
			int32_t waitTime = atoi(word.c_str());

			// 待機状態にする
			isWaiting = true;
			waitTimer = waitTime;

			return;  // 待機状態に移行し、次のフレームで処理再開
		}
	#pragma endregion
	} else{
		// コマンドが終端に到達した場合、ストリームをリセット
		ResetPopData();
	}
}

// ストリームをリセットして最初からコマンドを再度読み込む関数
void EnemyManager::ResetPopData(){
	// ストリームの位置を先頭に戻す
	enemyPopCommands_.clear();  // エラー状態をクリア
	enemyPopCommands_.seekg(0, std::ios::beg);  // ストリームを先頭に移動
}

void EnemyManager::CreateChainEnemy(int fieldIndex){
	// チェーンエネミーを生成
	std::unique_ptr<ChainEnemy> chainEnemy = std::make_unique<ChainEnemy>();

	// モデルを作成し、ChainEnemyを初期化
	std::array<Object3d*, 2> models = CreateEnemyModels(2);

	chainEnemy->Initialize(models, GetSpawnPosition(fieldIndex));

	// フィールドインデックスを設定
	chainEnemy->SetFieldIndex(fieldIndex - 1);

	chainEnemy->SetEnemyManager(this);

	// chainEnemies_ に追加
	chainEnemies_.emplace_back(std::move(chainEnemy));
}

void EnemyManager::CreateUnChainEnemy(int fieldIndex){
	// 敵を生成しリストに追加
	NoteEnemy* noteEnemy = new NoteEnemy();
	Object3d* noteEnemyModel = CreateSingleEnemyModel();

	noteEnemy->Initialize(noteEnemyModel, GetSpawnPosition(fieldIndex));

	noteEnemy->SetFieldIndex(fieldIndex - 1);

	noteEnemies_.emplace_back(std::move(noteEnemy));
}

void EnemyManager::PopObstacle(){
	int fieldIndex = Random::GetInt(1, 5);//1~5の範囲
	Obstacle* obsctacle = new Obstacle();
	Object3d* obsctacleModel = CreateObstacleModel();

	obsctacle->Initialize(obsctacleModel, GetObstacleSpawnPosition(fieldIndex));

	obstacles_.emplace_back(std::move(obsctacle));
}

Vector3 EnemyManager::GetSpawnPosition(int fieldIndex){
	float spawnPosY = pField_->GetPos(fieldIndex - 1).y;
	float spawnPosX = Field::cameraScrollX_ + 45.0f;
	return Vector3 {spawnPosX, spawnPosY, 0.0f};
}

Vector3 EnemyManager::GetObstacleSpawnPosition(int fieldIndex){
	float spawnPosY = pField_->GetPos(fieldIndex - 1).y;
	float swapnPosX = 45.0f;
	float offset = 10.0f;
	float fieldEnd = Field::fieldEndPosX;
	float spawnPosX = Random::GetFloat(fieldEnd + offset, swapnPosX + Field::cameraScrollX_);
	return Vector3 {spawnPosX, spawnPosY, 0.0f};
}

/////////////////////////////////////////////////////////////////////////////////////////
//                          モデルの生成
/////////////////////////////////////////////////////////////////////////////////////////

std::array<Object3d*, 2> EnemyManager::CreateEnemyModels(int count){
	std::array<Object3d*, 2> models {};
	for (int i = 0; i < count; i++){
		models[i] = new Object3d;
		models[i]->Create("enemy.obj");
	}
	return models;
}

Object3d* EnemyManager::CreateSingleEnemyModel(){
	Object3d* model = new Object3d;
	model->Create("enemy.obj");
	return model;
}

Object3d* EnemyManager::CreateObstacleModel(){
	Object3d* model = new Object3d;
	model->Create("obstacle.obj");
	return model;
}

