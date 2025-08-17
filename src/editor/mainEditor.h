#ifndef VRAKTAL_EDITOR_INITEDITOR_H
#define VRAKTAL_EDITOR_INITEDITOR_H
#pragma once

namespace core::rhi
{
	class RenderContext;
}

namespace editor
{
	class MainEditor
	{
		core::rhi::RenderContext* m_rCtx;

	public:
		MainEditor(core::rhi::RenderContext& rCtx);
		void RunImgui();
		void ClearImgui();


	};
}

#endif //VRAKTAL_EDITOR_INITEDITOR_H

