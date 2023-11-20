#pragma once
#include <string>

// キーの入力状態
struct KeyState 
{
	bool nowKey;
	bool prevKey;
};

// 入力キーの定義
struct KeyDefine
{
	std::string name;
	int inputKey;
};

// シングルトンクラス
class InputManager
{
private:
	InputManager()
	{
	}

	static InputManager* instance;

public:
	static void CreateInstance()
	{
		instance = new InputManager();
	}

	static void DeleteInstance()
	{
		delete instance;
	}

	static InputManager* GetInstance() 
	{
		return instance;
	}

private:
	std::map<std::string, KeyState> m_KeyState;
	std::vector<KeyDefine> m_KeyMap;

public:
	~InputManager() = default;

	void Initialize();
	void Update();

	bool GetKeyState(std::string keyName);
	bool GetKeyPressed(std::string keyName);
	bool GetKeyReleased(std::string keyName);
};