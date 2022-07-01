#pragma once
//*****************************************************************************
//!	@file	CWindow.h
//!	@brief	
//!	@note	ウインドウ関連クラス
//!	@author	
//*****************************************************************************
#pragma once

//-----------------------------------------------------------------------------
//	Include header files.
//-----------------------------------------------------------------------------
#include	<Windows.h>

class CWindowCallback;

//=============================================================================
//!	@class	CWindow
//!	@brief	ウインドウ関連クラス(シングルトン)
//=============================================================================
class CWindow
{
	HWND				m_Handle;
	MSG					m_Message;
	CWindowCallback*	m_cpCallback;

public:
	const char*			m_className;

	static CWindow* Instance();

	bool ExecMessage();

	void SetWindow(HINSTANCE	h_Instance,
		long			h_Width,
		long			h_Height);

	long GetMessage() const;
	HWND GetHandle() const;
	CWindowCallback* GetCallback() const;

private:
	CWindow();
	virtual ~CWindow();
	CWindow(const CWindow&);
	CWindow& operator = (const CWindow&) {}
};

