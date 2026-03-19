/**
 * TaskSchedulerSystem.h
 * タスクスケジューラーシステム（JSON対応版・ホットリロード対応）
 *
 * ■ 設計方針
 *   - タイマーやループシーケンスのパラメータをJSONで定義可能にする
 *   - コールバックは「名前(文字列)」で登録し、JSONから参照する
 *   - nlohmann/json を使用（C++14互換）
 *   - ファイル読み込み / 文字列読み込み の両方に対応
 *   - デバッグビルド時はファイルのホットリロードに対応（APP_ENABLE_PARAM_HOT_RELOAD準拠）
 *
 * ■ JSONフォーマット例
 * {
 *     "timers": [
 *         {
 *             "callback": "SpawnEnemy",
 *             "delay": 2.0,
 *             "delayNextFrame": false
 *         }
 *     ],
 *     "loopSequences": [
 *         {
 *             "totalDuration": 5.0,
 *             "events": [
 *                 {
 *                     "callback": "FireBullet",
 *                     "delay": 0.0,
 *                     "delayNextFrame": false
 *                 }
 *             ]
 *         }
 *     ]
 * }
 */
#pragma once
#include <cassert>
#include <algorithm>
#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

 /** ホットリロード有効か（ParameterManagerと同じマクロを共有） */
#ifdef K2_DEBUG
#ifndef APP_ENABLE_PARAM_HOT_RELOAD
#define APP_ENABLE_PARAM_HOT_RELOAD
#endif
#endif

#ifdef APP_ENABLE_PARAM_HOT_RELOAD
#include <sys/stat.h>
#endif


class TaskSchedulerSystem : public Noncopyable
{
	using Callback = std::function<void()>;
	using Json = nlohmann::json;

private:
	// ----------------------------------------------------------------
	// 内部構造体
	// ----------------------------------------------------------------
	struct Event
	{
		float triggerTime;      // 実行される目標時刻
		Callback callback;
		float interval;         // ループ用の間隔保持
		bool delayNextFrame;    // 実行条件を満たした後、さらに1フレーム待つか
	};

	struct LoopSequence
	{
		float totalDuration;
		float phaseTime;
		std::vector<Event> events;
		int nextEventIndex;
	};

	// ----------------------------------------------------------------
	// ホットリロード用：読み込み元ファイル情報
	// ----------------------------------------------------------------
#ifdef APP_ENABLE_PARAM_HOT_RELOAD
	struct LoadedFileInfo
	{
		std::string filePath;       // 読み込んだファイルのパス
		time_t lastWriteTime;       // 最終更新時刻
	};
	std::vector<LoadedFileInfo> loadedFiles_;
#endif

	// ----------------------------------------------------------------
	// コールバックレジストリ
	// ----------------------------------------------------------------
	std::unordered_map<std::string, Callback> callbackRegistry_;

	// ----------------------------------------------------------------
	// ランタイムデータ
	// ----------------------------------------------------------------
	float currentTime_ = 0.0f;
	std::vector<Event> events_;
	bool needsSort_ = false;

	std::vector<Event> currentFrameEvents_;
	std::vector<Event> pendingNextFrameEvents_;

	std::vector<LoopSequence> loopSequences_;

public:
	TaskSchedulerSystem() {}
	~TaskSchedulerSystem() {}

	// ================================================================
	//  コールバック登録
	// ================================================================

	/**
	 * コールバックを名前付きで登録する（JSON定義から参照するため）
	 * @param name JSON内で使う識別名（例: "SpawnEnemy"）
	 * @param callback 実行される処理
	 */
	void RegisterCallback(const std::string& name, Callback callback)
	{
		callbackRegistry_[name] = std::move(callback);
	}

	/**
	 * 登録済みコールバックを削除する
	 */
	void UnregisterCallback(const std::string& name)
	{
		callbackRegistry_.erase(name);
	}

	/**
	 * 全コールバックをクリアする
	 */
	void ClearCallbacks()
	{
		callbackRegistry_.clear();
	}

	// ================================================================
	//  JSON からのスケジュール読み込み
	// ================================================================

	/**
	 * JSONファイルからスケジュールを読み込む
	 * ホットリロード有効時は自動的にファイル監視対象に追加される
	 * @param filePath JSONファイルのパス
	 * @return 読み込みに成功したか
	 */
	bool LoadFromFile(const std::string& filePath)
	{
		std::ifstream ifs(filePath);
		if (!ifs.is_open()) {
			return false;
		}

		Json j;
		try {
			ifs >> j;
		}
		catch (const Json::parse_error&) {
			return false;
		}

		LoadFromJson(j);

#ifdef APP_ENABLE_PARAM_HOT_RELOAD
		// 既に同じパスが登録済みなら更新時刻だけ更新、なければ新規追加
		bool found = false;
		for (auto& info : loadedFiles_) {
			if (info.filePath == filePath) {
				info.lastWriteTime = GetFileLastWriteTime(filePath.c_str());
				found = true;
				break;
			}
		}
		if (!found) {
			LoadedFileInfo info;
			info.filePath = filePath;
			info.lastWriteTime = GetFileLastWriteTime(filePath.c_str());
			loadedFiles_.push_back(std::move(info));
		}
#endif

		return true;
	}

	/**
	 * JSON文字列からスケジュールを読み込む
	 * ※ ホットリロード対象にはならない（ファイルパスが無いため）
	 * @param jsonString JSON文字列
	 * @return 読み込みに成功したか
	 */
	bool LoadFromString(const std::string& jsonString)
	{
		Json j;
		try {
			j = Json::parse(jsonString);
		}
		catch (const Json::parse_error&) {
			return false;
		}

		LoadFromJson(j);
		return true;
	}

	/**
	 * パース済みのnlohmann::jsonオブジェクトからスケジュールを読み込む
	 * @param j JSONオブジェクト
	 */
	void LoadFromJson(const Json& j)
	{
		// --- timers ---
		if (j.contains("timers") && j["timers"].is_array()) {
			for (const auto& timerObj : j["timers"]) {
				LoadTimer(timerObj);
			}
		}

		// --- loopSequences ---
		if (j.contains("loopSequences") && j["loopSequences"].is_array()) {
			for (const auto& seqObj : j["loopSequences"]) {
				LoadLoopSequence(seqObj);
			}
		}
	}

	// ================================================================
	//  従来のコードベース API（既存コードとの互換性を維持）
	// ================================================================

	/**
	 * タイマーをスケジュールする（コード直接指定）
	 */
	void AddTimer(float delay, Callback callback, bool delayNextFrame = false)
	{
		float targetTime = currentTime_ + delay;
		events_.push_back({ targetTime, std::move(callback), delay, delayNextFrame });
		needsSort_ = true;
	}

	/**
	 * ループシーケンスを作成する（コード直接指定）
	 */
	int CreateLoopSequence(float totalDuration)
	{
		LoopSequence seq;
		seq.totalDuration = totalDuration;
		seq.phaseTime = 0.0f;
		seq.nextEventIndex = 0;
		loopSequences_.push_back(std::move(seq));
		return static_cast<int>(loopSequences_.size()) - 1;
	}

	/**
	 * ループシーケンスにタイマーを登録する（コード直接指定）
	 */
	void AddLoopTimer(int sequenceId, float delay, Callback callback, bool delayNextFrame = false)
	{
		assert(sequenceId >= 0 && sequenceId < static_cast<int>(loopSequences_.size()));
		auto& seq = loopSequences_[sequenceId];
		assert(delay < seq.totalDuration && "delayはtotalDurationより小さくしてください");

		seq.events.push_back({ delay, std::move(callback), 0.0f, delayNextFrame });

		std::sort(seq.events.begin(), seq.events.end(), [](const Event& a, const Event& b)
			{
				return a.triggerTime < b.triggerTime;
			});
	}

	// ================================================================
	//  ユーティリティ
	// ================================================================

	/**
	 * 全スケジュール（タイマー＆ループシーケンス）をクリアする
	 * ※ コールバックレジストリはクリアしない
	 * ※ ホットリロード用ファイル情報はクリアしない
	 */
	void ClearSchedule()
	{
		events_.clear();
		currentFrameEvents_.clear();
		pendingNextFrameEvents_.clear();
		loopSequences_.clear();
		needsSort_ = false;
	}

	/**
	 * 現在時刻をリセットする
	 */
	void ResetTime()
	{
		currentTime_ = 0.0f;
	}

	/**
	 * 全リセット（スケジュール＋時刻＋ホットリロード情報）
	 */
	void Reset()
	{
		ClearSchedule();
		ResetTime();
#ifdef APP_ENABLE_PARAM_HOT_RELOAD
		loadedFiles_.clear();
#endif
	}

	// ================================================================
	//  毎フレーム更新
	// ================================================================

	void Update(float deltaTime)
	{
#ifdef APP_ENABLE_PARAM_HOT_RELOAD
		// ホットリロードチェック（ファイル変更検出時は自動リロード）
		CheckHotReload();
#endif

		currentTime_ += deltaTime;

		if (needsSort_) {
			SortEvents();
			needsSort_ = false;
		}

		// 実行時刻が来たイベントを処理
		for (auto it = events_.begin(); it != events_.end(); ) {
			if (currentTime_ >= it->triggerTime) {
				if (it->delayNextFrame) {
					pendingNextFrameEvents_.push_back(std::move(*it));
				}
				else {
					if (it->callback) {
						it->callback();
					}
				}
				it = events_.erase(it);
			}
			else {
				break;
			}
		}

		// ループシーケンスの更新
		UpdateLoopSequence(deltaTime);

		// 「次フレーム実行」タスクの処理
		std::vector<Event> eventsToExecute;
		eventsToExecute.swap(currentFrameEvents_);

		for (const auto& task : eventsToExecute) {
			if (task.callback) task.callback();
		}

		currentFrameEvents_.swap(pendingNextFrameEvents_);
	}

private:
	// ----------------------------------------------------------------
	// ホットリロード
	// ----------------------------------------------------------------

#ifdef APP_ENABLE_PARAM_HOT_RELOAD
	/**
	 * ファイル更新日時を取得する
	 * ParameterManagerと同じ方式（stat関数）
	 */
	static time_t GetFileLastWriteTime(const char* path)
	{
		struct stat result;
		if (stat(path, &result) == 0) {
			return result.st_mtime;
		}
		return 0;
	}

	/**
	 * ファイルが更新されているかチェックする
	 */
	static bool CheckFileModified(const LoadedFileInfo& info)
	{
		return GetFileLastWriteTime(info.filePath.c_str()) > info.lastWriteTime;
	}

	/**
	 * ホットリロードチェック＆実行
	 *
	 * 読み込み済みファイルの更新を検出したら：
	 *   1. 現在のスケジュールを全クリア（ClearSchedule）
	 *   2. 時刻をリセット
	 *   3. 全ファイルを再読み込み（LoadFromJson）
	 *   4. 更新時刻を記録
	 *
	 * ※ 複数ファイルをロードしている場合、1つでも変更があれば
	 *   全ファイルを再読み込みする（整合性を保つため）
	 */
	void CheckHotReload()
	{
		bool anyModified = false;
		for (const auto& info : loadedFiles_) {
			if (CheckFileModified(info)) {
				anyModified = true;
				break;
			}
		}

		if (!anyModified) {
			return;
		}

		// スケジュールをクリア（ファイル情報リストは保持する）
		ClearSchedule();
		currentTime_ = 0.0f;

		// 全ファイルを再読み込み
		for (auto& info : loadedFiles_) {
			std::ifstream ifs(info.filePath);
			if (!ifs.is_open()) {
				continue;
			}

			Json j;
			try {
				ifs >> j;
			}
			catch (const Json::parse_error&) {
				continue;
			}

			LoadFromJson(j);

			// 更新時刻を記録
			info.lastWriteTime = GetFileLastWriteTime(info.filePath.c_str());
		}
	}
#endif // APP_ENABLE_PARAM_HOT_RELOAD

	// ----------------------------------------------------------------
	// JSON読み込みヘルパー
	// ----------------------------------------------------------------

	/**
	 * レジストリからコールバックを検索する
	 * 見つからない場合は空の Callback を返す
	 */
	Callback FindCallback(const std::string& name) const
	{
		auto it = callbackRegistry_.find(name);
		if (it != callbackRegistry_.end()) {
			return it->second;
		}
		return nullptr;
	}

	/**
	 * JSONオブジェクトからタイマーを1件読み込んで events_ に追加する
	 *
	 * 必須キー:
	 *   "callback" (string) — RegisterCallbackで登録済みの名前
	 *   "delay"    (number) — 実行までの秒数
	 *
	 * オプション:
	 *   "delayNextFrame" (bool, default: false)
	 */
	void LoadTimer(const Json& timerObj)
	{
		if (!timerObj.contains("callback") || !timerObj.contains("delay")) {
			return;
		}

		std::string cbName = timerObj["callback"].get<std::string>();
		float delay = timerObj["delay"].get<float>();
		bool dnf = false;

		if (timerObj.contains("delayNextFrame")) {
			dnf = timerObj["delayNextFrame"].get<bool>();
		}

		Callback cb = FindCallback(cbName);
		float targetTime = currentTime_ + delay;
		events_.push_back({ targetTime, std::move(cb), delay, dnf });
		needsSort_ = true;
	}

	/**
	 * JSONオブジェクトからループシーケンスを1件読み込む
	 *
	 * 必須キー:
	 *   "totalDuration" (number)
	 *   "events"        (array)
	 *
	 * events配列の各要素:
	 *   "callback" (string) — 必須
	 *   "delay"    (number) — 必須。シーケンス先頭からの秒数
	 *   "delayNextFrame" (bool, default: false)
	 */
	void LoadLoopSequence(const Json& seqObj)
	{
		if (!seqObj.contains("totalDuration") || !seqObj.contains("events")) {
			return;
		}

		float totalDuration = seqObj["totalDuration"].get<float>();
		int seqId = CreateLoopSequence(totalDuration);

		if (seqObj["events"].is_array()) {
			for (const auto& evObj : seqObj["events"]) {
				if (!evObj.contains("callback") || !evObj.contains("delay")) {
					continue;
				}

				std::string cbName = evObj["callback"].get<std::string>();
				float delay = evObj["delay"].get<float>();
				bool dnf = false;

				if (evObj.contains("delayNextFrame")) {
					dnf = evObj["delayNextFrame"].get<bool>();
				}

				Callback cb = FindCallback(cbName);
				AddLoopTimer(seqId, delay, std::move(cb), dnf);
			}
		}
	}

	// ----------------------------------------------------------------
	// ソート・ループ更新（既存ロジック）
	// ----------------------------------------------------------------

	void SortEvents()
	{
		std::sort(events_.begin(), events_.end(), [](const Event& a, const Event& b)
			{
				return a.triggerTime < b.triggerTime;
			});
	}

	void UpdateLoopSequence(float deltaTime)
	{
		for (auto& seq : loopSequences_) {
			if (seq.events.empty()) {
				seq.phaseTime += deltaTime;
				while (seq.phaseTime >= seq.totalDuration) {
					seq.phaseTime -= seq.totalDuration;
				}
				continue;
			}

			float remaining = deltaTime;

			while (remaining > 0.0f) {
				float prevPhase = seq.phaseTime;
				float advanced = (std::min)(remaining, seq.totalDuration - prevPhase);
				seq.phaseTime += advanced;
				remaining -= advanced;

				FireSequenceEvents(seq, prevPhase, seq.phaseTime);

				if (seq.phaseTime >= seq.totalDuration) {
					seq.phaseTime -= seq.totalDuration;
					seq.nextEventIndex = 0;
				}
			}
		}
	}

	void FireSequenceEvents(LoopSequence& seq, float fromPhase, float toPhase)
	{
		for (int i = seq.nextEventIndex; i < static_cast<int>(seq.events.size()); ++i) {
			const Event& ev = seq.events[i];
			if (ev.triggerTime >= fromPhase && ev.triggerTime < toPhase) {
				if (ev.delayNextFrame) {
					pendingNextFrameEvents_.push_back(ev);
				}
				else {
					if (ev.callback) ev.callback();
				}
				seq.nextEventIndex = i + 1;
			}
			else if (ev.triggerTime >= toPhase) {
				break;
			}
		}
	}
};
