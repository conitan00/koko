#pragma once

#pragma once

class ACTION_PvsAI {

public:
	void Out();
	void init(); // 初期化

private:

	static const int range = 4; // Ai学習における視野
	int episode = 1;
	int numDeath = 0;
	int cnt = 0; // キャラの方向描画のためのカウント
	static const int numPla = 1; // 敵キャラ数
	static const int numAI = 1; // AIキャラ数
	static const int numCha = numPla + numAI; // 総キャラ数
	static const int num_bom = 10; // 最大ボムストック数
	bool end = false; // エピソード終了フラグ
	int endCnt = 100; // エピソード終了演出時間
	const int Mov = CELL / 20; // 移動距離
	int bonKeys[2] = { KEY_INPUT_Q, KEY_INPUT_I }; // 各プレイヤーの爆弾設置キー
	int Keys[2][4] = { { KEY_INPUT_S, KEY_INPUT_A, KEY_INPUT_D, KEY_INPUT_W  },
					   { KEY_INPUT_J, KEY_INPUT_H, KEY_INPUT_K, KEY_INPUT_U } }; // 各プレイヤーの移動キー
	int Dirs[4][2] = { {0, 1}, {-1, 0}, {1, 0}, {0, -1} }; // 移動方向（下左右上）
	int initPosCha[4][2] = { {CELL / 2, CELL / 2}, {POS_MAX_X - CELL / 2, POS_MAX_Y - CELL / 2}, {POS_MAX_X - CELL / 2, CELL / 2}, {CELL / 2, POS_MAX_Y - CELL / 2} }; // キャラクターの初期座標（4キャラ分）
	std::vector<std::unique_ptr<DQNAgent>> Cha; // キャラクターの動的配列

	void learn(); // 機械学習
	void Death(int i); // キャラ死亡時処理
	void End(); // エピソード終了処理

};

ACTION_PvsAI Act_PvsAI;


void ACTION_PvsAI::Out() {

	if (Key[KEY_INPUT_0] == 1) {
		Cha[1]->qnet.save = true;
	}

	cnt++;

	// 全キャラに対して移動・爆弾の設置を実行
	for (int i = 0; i < ACTION_PvsAI::numCha; i++) {
		if (Cha[i]->death) continue;

		// キャラクターが何セル目に位置するか記録
		Cha[i]->X_cell = Cha[i]->X / CELL;
		Cha[i]->Y_cell = Cha[i]->Y / CELL;

		
		// プレイヤーの移動＆衝突判定
		if (Cha[i]->AorE == "Pla") {
			for (int dir = 0; dir < 4; dir++) {
				if (Key[Keys[i][dir]] != 0) {
					Cha[i]->Dir = dir;
					Cha[i]->X += Cha[i]->Mov * Dirs[dir][0];
					Cha[i]->Y += Cha[i]->Mov * Dirs[dir][1];
					if (0 <= Cha[i]->X - CHA_X2 && Cha[i]->X + CHA_X2 <= POS_MAX_X &&
						0 <= Cha[i]->Y - CHA_Y2_UP && Cha[i]->Y + CHA_Y2_DO <= POS_MAX_Y) {
					}
					else {
						Cha[i]->X -= Cha[i]->Mov * Dirs[dir][0];
						Cha[i]->Y -= Cha[i]->Mov * Dirs[dir][1];
						break;
					}

					if (Sta.Blo.Type_Exp[Cha[i]->X - CHA_X2][Cha[i]->Y + CHA_Y2_DO] == '1' ||
						Sta.Blo.Type_Exp[Cha[i]->X + CHA_X2][Cha[i]->Y + CHA_Y2_DO] == '1' ||
						Sta.Blo.Type_Exp[Cha[i]->X - CHA_X2][Cha[i]->Y - CHA_Y2_UP] == '1' ||
						Sta.Blo.Type_Exp[Cha[i]->X + CHA_X2][Cha[i]->Y - CHA_Y2_UP] == '1') {
						Cha[i]->X -= Cha[i]->Mov * Dirs[dir][0];
						Cha[i]->Y -= Cha[i]->Mov * Dirs[dir][1];
					}

					break;
				}
			}
		}

		// プレイヤーの爆弾設置
		if (Cha[i]->AorE == "Pla") {
			if (Key[bonKeys[i]] == 1) {
				for (int j = 0; j < Cha[i]->numBom; j++) {
					if (Cha[i]->boms[j].exist == false) {
						Cha[i]->boms[j] = { true, 9.0, CELL * Cha[i]->X_cell + CELL / 2, CELL * Cha[i]->Y_cell + CELL / 2, Cha[i]->X_cell, Cha[i]->Y_cell };
						break;
					}
				}
			}
		}

		// AIの移動, 爆弾設置
		else if (Cha[i]->AorE == "AI") {
			if (Cha[i]->totalMove == 0) {


				learn();

			}
			Cha[i]->X += Dirs[Cha[i]->Dir][0] * Mov;
			Cha[i]->Y += Dirs[Cha[i]->Dir][1] * Mov;
			Cha[i]->totalMove += Mov;
			if (Cha[i]->totalMove >= 2 * CELL) Cha[i]->totalMove = 0;
		}

	}

	Sta.Out(); // ステージ描画

	for (int i = 0; i < ACTION_PvsAI::numCha; i++) {
		if (Cha[i]->death) continue;
		for (int b = 0; b < num_bom; b++) {
			if (Cha[i]->boms[b].exist) {
				if (3.0 < Cha[i]->boms[b].time && Cha[i]->boms[b].time <= 9.0) {
					if (Cha[i]->boms[b].time >= 8.0) DrawRotaGraph(Cha[i]->boms[b].X, Cha[i]->boms[b].Y, 1.0, 0, Pic.Bom[1], TRUE);
					else if (Cha[i]->boms[b].time >= 6.8) DrawRotaGraph(Cha[i]->boms[b].X, Cha[i]->boms[b].Y, 1.0, 0, Pic.Bom[2], TRUE);
					else if (Cha[i]->boms[b].time >= 5.5) DrawRotaGraph(Cha[i]->boms[b].X, Cha[i]->boms[b].Y, 1.0, 0, Pic.Bom[1], TRUE);
					else if (Cha[i]->boms[b].time >= 4.2) DrawRotaGraph(Cha[i]->boms[b].X, Cha[i]->boms[b].Y, 1.0, 0, Pic.Bom[3], TRUE);
					else if (Cha[i]->boms[b].time >= 3.0) DrawRotaGraph(Cha[i]->boms[b].X, Cha[i]->boms[b].Y, 1.0, 0, Pic.Bom[4], TRUE);
				}
				else {
					for (int fireLength = 0; fireLength <= Cha[i]->fireLength; fireLength++)
					{
						for (int fireDir = 0; fireDir < 4; fireDir++)
						{
							if (Sta.Blo.Type[Cha[i]->boms[b].X_cell + Dirs[fireDir][0]][Cha[i]->boms[b].Y_cell + Dirs[fireDir][1]] == '0')
							{
								if (Cha[i]->boms[b].time >= 2.5) DrawRotaGraph(Cha[i]->boms[b].X + fireLength * Dirs[fireDir][0] * CELL, Cha[i]->boms[b].Y + fireLength * Dirs[fireDir][1] * CELL, 1.0, 0, Pic.Bom[30], TRUE);
								else if (Cha[i]->boms[b].time >= 2.0) DrawRotaGraph(Cha[i]->boms[b].X + fireLength * Dirs[fireDir][0] * CELL, Cha[i]->boms[b].Y + fireLength * Dirs[fireDir][1] * CELL, 1.0, 0, Pic.Bom[31], TRUE);
								else if (Cha[i]->boms[b].time >= 1.5) DrawRotaGraph(Cha[i]->boms[b].X + fireLength * Dirs[fireDir][0] * CELL, Cha[i]->boms[b].Y + fireLength * Dirs[fireDir][1] * CELL, 1.0, 0, Pic.Bom[32], TRUE);
								else if (Cha[i]->boms[b].time >= 1.0) DrawRotaGraph(Cha[i]->boms[b].X + fireLength * Dirs[fireDir][0] * CELL, Cha[i]->boms[b].Y + fireLength * Dirs[fireDir][1] * CELL, 1.0, 0, Pic.Bom[33], TRUE);
								else if (Cha[i]->boms[b].time >= 0.5) DrawRotaGraph(Cha[i]->boms[b].X + fireLength * Dirs[fireDir][0] * CELL, Cha[i]->boms[b].Y + fireLength * Dirs[fireDir][1] * CELL, 1.0, 0, Pic.Bom[34], TRUE);
								else if (Cha[i]->boms[b].time >= 0.0) DrawRotaGraph(Cha[i]->boms[b].X + fireLength * Dirs[fireDir][0] * CELL, Cha[i]->boms[b].Y + fireLength * Dirs[fireDir][1] * CELL, 1.0, 0, Pic.Bom[35], TRUE);

								for (int k = 0; k < ACTION_PvsAI::numCha; k++) {
									if (Cha[k]->death) continue;
									if (Cha[i]->boms[b].X_cell + fireLength * Dirs[fireDir][0] == Cha[k]->X_cell && Cha[i]->boms[b].Y_cell + fireLength * Dirs[fireDir][1] == Cha[k]->Y_cell)
									{
										if (end == false) {
											if (!Cha[k]->death) {
												Cha[k]->death = true;
												numDeath++;
												if (k == 0) {
													// 行動不可の選択肢に関するマスク
													vector<bool> mask;
													if (Cha[1]->Y_cell == STAGE_MAX_Y - 1) { mask.push_back(true); }
													else { mask.push_back(false); }
													if (Cha[1]->X_cell == 0) { mask.push_back(true); }
													else { mask.push_back(false); }
													if (Cha[1]->X_cell == STAGE_MAX_X - 1) { mask.push_back(true); }
													else { mask.push_back(false); }
													if (Cha[1]->Y_cell == 0) { mask.push_back(true); }
													else { mask.push_back(false); }
													if (Cha[1]->Y_cell == STAGE_MAX_Y - 1) { mask.push_back(true); }
													else { mask.push_back(false); }
													if (Cha[1]->X_cell == 0) { mask.push_back(true); }
													else { mask.push_back(false); }
													if (Cha[1]->X_cell == STAGE_MAX_X - 1) { mask.push_back(true); }
													else { mask.push_back(false); }
													if (Cha[1]->Y_cell == 0) { mask.push_back(true); }
													else { mask.push_back(false); }
													// --------------------------------
													Memory memory = { Cha[1]->state_pre, Cha[1]->action_pre, -1, Cha[1]->state_pre, true, Cha[1]->mask_pre, mask };
													//Cha[1]->update(memory);
												}
											}
											if (numDeath == numCha - 1) {
												end = true;
											}

											break;
										}
										else {
											break;
										}
									}
								}
							}
						}
					}
				}
				Cha[i]->boms[b].time -= 0.1;
				if (Cha[i]->boms[b].time < 0) {
					Cha[i]->boms[b].exist = false;
				}
			}
		}
	}

	for (int i = 0; i < ACTION_PvsAI::numCha; i++) {
		int p = (cnt / 12) % 3;
		if (!Cha[i]->death) {
			DrawRotaGraph(Cha[i]->X, Cha[i]->Y, 1.0, 0, Pic.Cat[Cha[i]->picID][Cha[i]->Dir * 3 + p], TRUE);
		}
	}

	for (int i = 0; i < ACTION_PvsAI::numCha; i++) {
		if (Cha[i]->death) Death(i);
	}

	if (end) End();

}


// 機械学習
void ACTION_PvsAI::learn() {

	//Vector3D state(2, Vector2D(STAGE_MAX_Y, Vector1D(STAGE_MAX_X, 0.0))); // 状態を初期化（自分の位置、敵の位置、爆弾の位置）

	//for (int i = 0; i < numCha; i++) {
	//	Cha[i]->X_cell = Cha[i]->X / CELL;
	//	Cha[i]->Y_cell = Cha[i]->Y / CELL;
	//}

	//for (const auto& cha : Cha) {
	//	if (cha->AorE == "AI") {
	//		state[0][cha->Y_cell][cha->X_cell] = 1.0;
	//	}
	//	/*else if (cha->AorE == "Ene") {
	//		state[1][cha->Y_cell][cha->X_cell] = 1.0;
	//	}*/

	//	for (const auto& bom : cha->boms) {
	//		if (bom.exist) {
	//			state[1][bom.Y_cell][bom.X_cell] = 1.0;
	//		}
	//	}
	//}
	Vector3D state(2, Vector2D(2 * range + 1, Vector1D(2 * range + 1, 0.0))); // 状態を初期化（爆弾の位置、敵の位置）

	for (int i = 0; i < numCha; i++) {
		Cha[i]->X_cell = Cha[i]->X / CELL;
		Cha[i]->Y_cell = Cha[i]->Y / CELL;
	}

	// ボム、キャラの位置を保存
	for (const auto& cha : Cha) {

		if (cha != Cha[1]) {
			if (Cha[1]->X_cell - range <= cha->X_cell && cha->X_cell <= Cha[1]->X_cell + range &&
				Cha[1]->Y_cell - range <= cha->Y_cell && cha->Y_cell <= Cha[1]->Y_cell + range) {
				state[0][range + (cha->Y_cell - Cha[1]->Y_cell)][range + (cha->X_cell - Cha[1]->X_cell)] = 1.0;
			}
		}

		for (const auto& bom : cha->boms) {
			if (bom.exist) {
				if (Cha[1]->X_cell - range <= bom.X_cell && bom.X_cell <= Cha[1]->X_cell + range &&
					Cha[1]->Y_cell - range <= bom.Y_cell && bom.Y_cell <= Cha[1]->Y_cell + range) {
					state[1][range + (bom.Y_cell - Cha[1]->Y_cell)][range + (bom.X_cell - Cha[1]->X_cell)] = 1.0;
				}
			}
		}

	}


	// 行動不可の選択肢に関するマスク
	vector<bool> mask;
	if (Cha[1]->Y_cell == STAGE_MAX_Y - 1) { mask.push_back(true); }
	else { mask.push_back(false); }
	if (Cha[1]->X_cell == 0) { mask.push_back(true); }
	else { mask.push_back(false); }
	if (Cha[1]->X_cell == STAGE_MAX_X - 1) { mask.push_back(true); }
	else { mask.push_back(false); }
	if (Cha[1]->Y_cell == 0) { mask.push_back(true); }
	else { mask.push_back(false); }
	if (Cha[1]->Y_cell == STAGE_MAX_Y - 1) { mask.push_back(true); }
	else { mask.push_back(false); }
	if (Cha[1]->X_cell == 0) { mask.push_back(true); }
	else { mask.push_back(false); }
	if (Cha[1]->X_cell == STAGE_MAX_X - 1) { mask.push_back(true); }
	else { mask.push_back(false); }
	if (Cha[1]->Y_cell == 0) { mask.push_back(true); }
	else { mask.push_back(false); }
	// --------------------------------

	int tempDir = Cha[1]->get_action(state, mask);
	// 爆弾を設置
	if (tempDir >= 4) {
		for (int j = 0; j < num_bom; j++) {
			if (Cha[1]->boms[j].exist == false) {
				Cha[1]->boms[j] = { true, 9.0, CELL * Cha[1]->X_cell + CELL / 2, CELL * Cha[1]->Y_cell + CELL / 2, Cha[1]->X_cell, Cha[1]->Y_cell };
				break; // 1回だけ爆弾を置いてループを抜ける
			}
		}
	}
	tempDir = tempDir % 4;
	if (0 <= Cha[1]->X_cell + Dirs[tempDir][0] && Cha[1]->X_cell + Dirs[tempDir][0] < STAGE_MAX_X && 0 <= Cha[1]->Y_cell + Dirs[tempDir][1] && Cha[1]->Y_cell + Dirs[tempDir][1] < STAGE_MAX_Y)
	{
		Cha[1]->Dir = tempDir;
	}
	else
	{
		while (true) {
			random_device rd;
			mt19937 gen(rd());
			uniform_int_distribution<int> dist(0, 3);
			int tempDir = dist(gen);
			if (0 < Cha[1]->X + Dirs[tempDir][0] * CELL && Cha[1]->X + Dirs[tempDir][0] * CELL < POS_MAX_X &&
				0 < Cha[1]->Y + Dirs[tempDir][1] * CELL && Cha[1]->Y + Dirs[tempDir][1] * CELL < POS_MAX_Y) {
				Cha[1]->Dir = tempDir;
				break;
			}
		}
	}

	double reward = 0.01;

	if (!Cha[1]->state_pre.empty()) {
		Memory memory = { Cha[1]->state_pre, Cha[1]->action_pre, reward, state, false, Cha[1]->mask_pre, mask };
		//Cha[1]->update(memory);
	}

	Cha[1]->state_pre = state; // 状態を保存
	Cha[1]->action_pre = Cha[1]->Dir;
	Cha[1]->mask_pre = mask;

}

// 初期化
void ACTION_PvsAI::init() {

	// キャラ数分のエージェントオブジェクトを作成
	Cha.clear();
	for (int i = 0; i < numCha; ++i) {
		Cha.push_back(std::make_unique<DQNAgent>());
	}

	// 各キャラのメンバ変数初期化
	for (int i = 0; i < numCha; i++)
	{
		if (i < numAI) Cha[i]->AorE = "Pla";
		else Cha[i]->AorE = "AI";
		Cha[i]->X = initPosCha[i][0];
		Cha[i]->Y = initPosCha[i][1];
		Cha[i]->Dir = 0;
		Cha[i]->numBom = 1;
		Cha[i]->Mov = 2;
		Cha[i]->fireLength = 1;
		Cha[i]->picID = 0;
		Cha[i]->death = false;
		Cha[i]->totalMove = 0;

		for (int b = 0; b < num_bom; b++)
		{
			Cha[i]->boms[b].exist = false;
		}
	}

	// AIキャラのデザインを設定
	Cha[1]->picID = 10;

	Cha[1]->epsilon = 0.0;


}

// 各キャラの
void ACTION_PvsAI::Death(int i) {

	Cha[i]->death_cnt--;
	if (Cha[i]->death_cnt >= 0) {
		DrawFormatStringToHandle(Cha[i]->X, Cha[i]->Y, Col.Blu, Fon.c[30], "Lose");
	}

}

// エピソード終了時の処理
void ACTION_PvsAI::End() {

	// 画面切り替えまでの時間を計測
	endCnt--;

	// 1位のキャラに対しての演出描画
	for (int i = 0; i < numCha; i++) {
		if (!Cha[i]->death) DrawFormatStringToHandle(Cha[i]->X, Cha[i]->Y, Col.Red, Fon.c[30], "Win");
	}

	// 画面切り替え、初期化
	if (endCnt <= 0) {

		// 1位のキャラの勝利カウントを加算
		for (int i = 0; i < numCha; i++) {
			if (!Cha[i]->death) {
				Cha[i]->win_num++;
				Cha[i]->win_num_tmp++;
			}
		}

		// 各キャラのメンバ変数初期化
		for (int i = 0; i < numCha; i++)
		{
			if (i < numPla) Cha[i]->AorE = "Pla";
			else Cha[i]->AorE = "AI";
			Cha[i]->X = initPosCha[i][0];
			Cha[i]->Y = initPosCha[i][1];
			Cha[i]->Dir = 0;
			Cha[i]->numBom = 1;
			Cha[i]->Mov = 2;
			Cha[i]->fireLength = 1;
			Cha[i]->picID = 0;
			Cha[i]->death = false;
			Cha[i]->totalMove = 0;

			for (int b = 0; b < num_bom; b++)
			{
				Cha[i]->boms[b].exist = false;
			}
		}

		// AIキャラのデザインを設定
		Cha[1]->picID = 10;

		end = false;
		numDeath = 0;
		episode++;

		// 各キャラの勝利総数をコンソールに出力
		for (const auto& cha : Cha) {
			cout << cha->win_num << ", ";
		}
		cout << endl;

		// 最新100エピソードでの各キャラの勝率をコンソールに出力
		if (episode % 100 == 0) {

			cout << "\nwinrate: ";
			for (int i = 0; i < numCha; i++) {
				cout << Cha[i]->win_num_tmp << "% ";
			}
			for (int i = 0; i < numCha; i++) {
				Cha[i]->win_num_tmp = 0;
			}
			cout << endl << endl;
		}

		// 10エピソードごとにε(を減少させる
		if (episode % 10 == 0 && Cha[1]->epsilon >= Cha[1]->e_min) {
			Cha[1]->epsilon -= Cha[1]->e_deg_rate;
			cout << "epsilon: " << Cha[1]->epsilon << endl;
		}

		// 20エピソードごとにターゲットネットワークを同期
		if (episode % 20 == 0) {
			Cha[1]->sync_qnet();
		}

	}

}