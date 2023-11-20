#include "InputManager.h"

InputManager* InputManager::instance = nullptr;

void InputManager::Initialize()
{
	// �L�[�}�b�v�̒�`
	m_KeyMap = {
		{"Up"   , 'W'},
		{"Down" , 'S'},
		{"Left" , 'A'},
		{"Right", 'D'},
		{"Enter" , VK_SPACE},
		{"Mouse", VK_LBUTTON},
		{"Esc", VK_ESCAPE},
	};

	for (KeyDefine keyMap : m_KeyMap) {
		m_KeyState[keyMap.name] = { 0, 0 };
	}
}

void InputManager::Update()
{
	// �L�[���͂̍X�V
	for (int i = 0; i < m_KeyMap.size(); i++) {
		m_KeyState[m_KeyMap[i].name].prevKey = m_KeyState[m_KeyMap[i].name].nowKey;

		m_KeyState[m_KeyMap[i].name].nowKey = (bool)GetAsyncKeyState(m_KeyMap[i].inputKey);
	}
}

// ���݂̃L�[���͏�Ԃ��擾
bool InputManager::GetKeyState(std::string keyName)
{
	return m_KeyState[keyName].nowKey;
}

// �L�[�������ꂽ�u�Ԃ��擾
bool InputManager::GetKeyPressed(std::string keyName)
{
	return (m_KeyState[keyName].prevKey == false) && (m_KeyState[keyName].nowKey == true);
}

// �L�[�������ꂽ�u�Ԃ��擾
bool InputManager::GetKeyReleased(std::string keyName)
{
	return (m_KeyState[keyName].prevKey == true) && (m_KeyState[keyName].nowKey == false);
}