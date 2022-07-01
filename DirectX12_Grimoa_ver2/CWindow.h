#pragma once
//*****************************************************************************
//!	@file	CWindow.h
//!	@brief	
//!	@note	�E�C���h�E�֘A�N���X
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
//!	@brief	�E�C���h�E�֘A�N���X(�V���O���g��)
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

